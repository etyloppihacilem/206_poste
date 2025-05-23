/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 21 May. 2025 at 20h52
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "UART_base.h"
#include "LPC17xx.h"
#include "utils.h"
#include <stdint.h>

#define MSG_LENGTH 32
#define INBOX_SIZE 4

t_msg_from_base inbox_base[INBOX_SIZE]   = { 0 };
char             msg_base[MSG_LENGTH + 1] = { 0 };

static uint8_t c_base      = 0;
static uint8_t w_base      = 0;
static uint8_t r_base      = 0;

uint8_t poste_num = 0;
uint8_t msg_base_received = 0;

static void parsing_base_interrogation() {
    // Vérifier que le message commence par '@T'
    if (msg_base[0] != '@' || msg_base[1] != 'T') {
        return; // Message invalide, on l'ignore
    }

    // Extraire le numéro de poste (XX)
    uint8_t poste_cible = parse_nb(msg_base[3], msg_base[2]); // Conversion des caractères en nombre

    // Vérifier si le poste cible correspond au poste actuel
    if (poste_cible == poste_num) {
        msg_base_received = 1; // Indiquer que le message est destiné à ce poste
    }
}

void UART1_IRQHandler(void) {
    if (LPC_UART1->IIR & 0x04) { // RDA: Receive Data Available
        if (c_base == MSG_LENGTH) {
            c_base = 0; // Réinitialiser si le message est trop long
        }

        char c = LPC_UART1->RBR & 0xFF; // Lire le caractère reçu
        if (c == '\0') {
            return; // Ignorer les caractères nuls
        }

        msg_base[c_base++] = c; // Ajouter le caractère au message
        msg_base[c_base]   = '\0'; // Terminer la chaîne

        // Vérifier si le message est complet (se termine par "\r\n")
        if (c_base >= 2 && msg_base[c_base - 1] == '\n' && msg_base[c_base - 2] == '\r') {
            parsing_base_interrogation(); // Traiter le message
            c_base = 0; // Réinitialiser pour le prochain message
        }
    }
}

static uint32_t get_uart1_pclk(void) {
    uint32_t             sel        = (LPC_SC->PCLKSEL0 >> 8) & 0x03;
    static const uint8_t div_lut[4] = { 4, 1, 2, 8 };
    return SystemCoreClock / div_lut[sel];
}

/*
 * Il faut une précision de l'horloge de UART1 pour pouvoir monter à 115200 bauds.
 * Pour 9600, pas besoin d'un calcul si compliqué.
 * */
void set_uart1_baud(uint32_t baud) {
    uint32_t pclk     = get_uart1_pclk();
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

    LPC_UART1->LCR |= 0x80; // DLAB = 1
    LPC_UART1->DLM  = best_dll >> 8;
    LPC_UART1->DLL  = best_dll & 0xFF;
    LPC_UART1->FDR  = (best_mul << 4) | best_divadd;
    LPC_UART1->LCR &= ~0x80; // DLAB = 0
}

void init_com_poste(uint32_t baudrate) {
    LPC_SC->PCONP |= (1 << 4); // on active l'horloge et le power de UART 1

    LPC_PINCON->PINSEL0 &= ~(3 << 30);
    LPC_PINCON->PINSEL0 |= 1 << 30; // P0.15 = TXD1
    LPC_PINCON->PINSEL1 &= ~3;
    LPC_PINCON->PINSEL1 |= 1; // P0.16 = RXD1

    // Désactiver interruptions pendant init
    LPC_UART1->IER = 0;

    // 8 bits, 1 stop bit, pas de parité
    set_uart1_baud(baudrate);

    LPC_UART1->LCR = 0x03; // 8 bits transmit
    LPC_UART1->FCR = 0x07; // Activer FIFO, reset RX/TX

    // Attente que tout soit prêt
    while (!(LPC_UART1->LSR & 0x20))
        ;
    NVIC_EnableIRQ(UART1_IRQn);
    NVIC_SetPriority(UART1_IRQn, 2); // priorité haute parce que ça doit aller vite
    LPC_UART1->IER = 1;              // RBR interrupt enable
}

void disable_base_rx() {
    LPC_UART1->IER &= ~1;
}

void enable_base_rx() {
    LPC_UART1->IER |= 1;
}

/*
 * returns message to treat, or NULL
 * */
t_msg_from_base *get_base_msg() {
    t_msg_from_base *msg;
    disable_base_rx();
    if (r_base == w_base)
        msg = 0;
    else
        msg = inbox_base + r_base; // on retourne un pointeur sur le message
    enable_base_rx();
    return msg;
}

/*
 * call this when done with processing message.
 * */
void base_msg_done() {
    disable_base_rx();
    if (r_base == INBOX_SIZE - 1)
        r_base = 0;
    else
        r_base++;
    enable_base_rx();
}

static int uart1_putchar(int c) { // peut être bufferiser ça si les prints prennent trop de temps.
    while (!(LPC_UART1->LSR & 0x20))
        ; // attente THRE
    LPC_UART1->THR = c;
    return c;
}

void send_msg_to_base() {
    if (msg_base_received) {
        msg_base_received = 0; // Réinitialiser après traitement

        //msg du poste vers base
        uart1_putchar('R');

        
        uart1_putchar('\r');
        uart1_putchar('\n');
        base_msg_done()
    }
}