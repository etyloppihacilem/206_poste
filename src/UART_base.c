/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 21 May. 2025 at 20h52
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "UART_base.h"
#include "LPC17xx.h"
#include "parsing_IR.h"
#include "pave_DIP.h"
#include "utils.h"
#include <stdint.h>

#define MSG_LENGTH 32

char            msg_base[MSG_LENGTH + 1] = { 0 };

static uint8_t c_base = 0;


uint8_t msg_base_received = 0;

static void parsing_base_interrogation() {
    // Vérifier que le message commence par '@T'
    if (msg_base[0] != '@' || msg_base[1] != 'T')
        return; // Message invalide, on l'ignore

    // Extraire le numéro de poste (XX)
    uint8_t poste_cible = parse_nb(msg_base[3], msg_base[2]); // Conversion des caractères en nombre

    // Vérifier si le poste cible correspond au poste actuel
    if (poste_cible == num_poste)
        msg_base_received = 1; // Indiquer que le message est destiné à ce poste
}

void UART0_IRQHandler(void) {
    if (LPC_UART0->IIR & 0x04) { // RDA: Receive Data Available
        if (c_base == MSG_LENGTH)
            c_base = 0; // Réinitialiser si le message est trop long

        char c = LPC_UART0->RBR & 0xFF; // Lire le caractère reçu
        if (c == '\0')
            return; // Ignorer les caractères nuls

        msg_base[c_base++] = c;    // Ajouter le caractère au message
        msg_base[c_base]   = '\0'; // Terminer la chaîne

        // Vérifier si le message est complet (se termine par "\r\n")
        if (c_base >= 2 && msg_base[c_base - 1] == '\n' && msg_base[c_base - 2] == '\r') {
            parsing_base_interrogation(); // Traiter le message
            c_base = 0;                   // Réinitialiser pour le prochain message
        }
    }
}

static uint32_t get_UART0_pclk(void) {
    uint32_t             sel        = (LPC_SC->PCLKSEL0 >> 8) & 0x03;
    static const uint8_t div_lut[4] = { 4, 1, 2, 8 };
    return SystemCoreClock / div_lut[sel];
}

/*
 * Il faut une précision de l'horloge de UART0 pour pouvoir monter à 115200 bauds.
 * Pour 9600, pas besoin d'un calcul si compliqué.
 * */
void set_UART0_baud(uint32_t baud) {
    uint32_t pclk     = get_UART0_pclk();
    uint32_t best_dll = 0, best_mul = 1, best_divadd = 0;
    uint32_t best_err = 0xFFFFFFFF;

    for (uint32_t mul = 1; mul <= 15; ++mul) {
        for (uint32_t divadd = 0; divadd < mul; ++divadd) {
            uint32_t dll = (pclk + 8 * baud * mul) / (16 * baud * mul) - divadd; // arrondi
            if (dll == 0 || dll > 0xFFFF)
                continue;

            uint32_t real = pclk / (16 * (dll + ((float) divadd / mul)));
            uint32_t err  = (real > baud) ? real - baud : baud - real;

            if (err < best_err) {
                best_err    = err;
                best_dll    = dll;
                best_mul    = mul;
                best_divadd = divadd;
            }
        }
    }

    LPC_UART0->LCR |= 0x80; // DLAB = 1
    LPC_UART0->DLM  = best_dll >> 8;
    LPC_UART0->DLL  = best_dll & 0xFF;
    LPC_UART0->FDR  = (best_mul << 4) | best_divadd;
    LPC_UART0->LCR &= ~0x80; // DLAB = 0
}

void init_com_poste(uint32_t baudrate) {
    LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));
    LPC_PINCON->PINSEL0 |= (1 << 4) | (1 << 6); // P0.2 = TXD0, P0.3 = RXD0
    LPC_SC->PCONP       |= (1 << 3);            // UART0

    // Désactiver interruptions pendant init
    LPC_UART0->IER = 0;

    // 8 bits, 1 stop bit, pas de parité
    set_UART0_baud(baudrate);

    LPC_UART0->LCR = 0x03; // 8 bits transmit
    LPC_UART0->FCR = 0x07; // Activer FIFO, reset RX/TX

    // Attente que tout soit prêt
    while (!(LPC_UART0->LSR & 0x20))
        ;
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 2); // priorité haute parce que ça doit aller vite
    LPC_UART0->IER = 1;              // RBR interrupt enable
}

void disable_base_rx() {
    LPC_UART0->IER &= ~1;
}

void enable_base_rx() {
    LPC_UART0->IER |= 1;
}


/*
 * call this when done with processing message.
 * */
void base_msg_done() {
    msg_base_received = 0;
}

int UART0_putchar(int c) { // peut être bufferiser ça si les prints prennent trop de temps.
    while (!(LPC_UART0->LSR & 0x20))
        ; // attente THRE
    LPC_UART0->THR = c;
    return c;
}

void debug_write(const char *str) {
    while (*str != '\0')
        UART0_putchar(*(str++));
}

void debug_put_int(int32_t n) {
    if (n < 0) {
        UART0_putchar('-');
        n *= -1;
    }
    if (n >= 10)
        debug_put_int(n / 10);
    UART0_putchar((n % 10) + '0');
}

void debug_put_uint(uint32_t n) {
    if (n >= 10)
        debug_put_uint(n / 10);
    UART0_putchar((n % 10) + '0');
}

void debug_put_hex(uint32_t n) {
    if (n >= 16)
        debug_put_uint(n / 10);
    char c = n % 16;
    if (c < 10)
        UART0_putchar(c + '0');
    else
        UART0_putchar(c - 10 + 'A');
}

void send_msg_IR_to_base(message_IR *msg) {
    if (!msg)
        return;

    char buffer[16];
    buffer[0] = 'R';

    // ID_rob en hexadécimal (0–F)
    uint8_t id_hex = msg->ID_rob & 0xF;
    buffer[1]      = (id_hex < 10) ? ('0' + id_hex) : ('A' + id_hex - 10);

    // Calcul de la vitesse
    uint8_t vitesse = ((msg->vitesse) * 5 + 20) / 10;
    buffer[2]       = (vitesse < 10) ? ('0' + vitesse) : ('A' + vitesse - 10);

    char status_char = 0;
    switch (msg->status) {
        case ir_dispo:
            status_char = dispo;
            break;
        case ir_enlevecolis:
            status_char = enlevecolis;
            break;
        case ir_colispris:
            status_char = colispris;
            break;
        case ir_livraison:
            status_char = livraison;
        default:;
    }

    buffer[3] = status_char;

    buffer[4] = '\r';
    buffer[5] = '\n';
    buffer[6] = '\0';

    debug_write(buffer);
}

void send_msg_pave_to_base(message_pave *msg) {
    if (!msg)
        return;

    char buffer[7];
    buffer[0] = msg->livreur;
    buffer[1] = 'P';

    // Convertir l'ID_post (dixaine + unité de type char) en numéro
    uint8_t poste = parse_nb(msg->ID_post_dix, msg->ID_post_uni);

    if (poste > 99) // invalide
        return;

    // Poste en 2 chiffres décimaux
    buffer[2] = '0' + (poste / 10);
    buffer[3] = '0' + (poste % 10);

    buffer[4] = '\r';
    buffer[5] = '\n';
    buffer[6] = '\0';

    debug_write(buffer);
}


