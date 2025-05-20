# ######################################################################################################################
#
#              """          Makefile
#       -\-    _|__
#        |\___/  . \        Created on 15 May. 2025 at 17:14
#        \     /(((/        by hmelica
#         \___/)))/         hmelica@student.42.fr
#
# ######################################################################################################################


####################
## Project config ##
####################

SRC_DIR		= src
PROJECT		= poste
SRC_FILES	= $(shell find $(SRC_DIR) -type f -name "*.c")

##############
## Make var ##
##############

SYS_SRCS = LPC1769/startup_LPC17xx.c LPC1769/system_LPC17xx.c
SYS_SRCS_DIR = LPC1769
SYS_OBJECTS =
INCLUDE_PATHS = -I. -I./LPC1769 -I./include
LIBRARY_PATHS =
LIBRARIES =
LINKER_SCRIPT = ./LPC1769/LPC1769.ld
BUILD_DIR = build
OBJECTS = build/system_LPC17xx.o build/startup_LPC17xx.o
OBJECTS += $(patsubst ${SRC_DIR}%.c,${BUILD_DIR}%.o,${SRC_FILES})
OBJECTS += $(patsubst ${SYS_SRCS_DIR}%.c,${BUILD_DIR}%.o,${SYS_SRCS})

###############################################################################
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy

CCLOCAL = gcc

CPU = -mcpu=cortex-m3 -mthumb
CC_FLAGS = $(CPU) -c -fno-common -fmessage-length=0 -Wall -fno-exceptions -ffunction-sections -fdata-sections -g
CC_SYMBOLS = -DTARGET_LPC1769 -DTOOLCHAIN_GCC_ARM -DNDEBUG -D__CORTEX_M3

LD_FLAGS = -mcpu=cortex-m3 -mthumb -Wl,--gc-sections,-Map=$(PROJECT).map,--cref --specs=nano.specs
LD_SYS_LIBS = -lc -lgcc -lnosys

NXPSUM = ./tools/nxpsum

all: $(PROJECT).bin

clean:
	rm -rf $(PROJECT).bin $(PROJECT).elf $(BUILD_DIR) $(PROJECT).map ${LXPSUM}

re: clean
	${MAKE}

flash: all
	openocd -f interface/cmsis-dap.cfg -f target/lpc17xx.cfg -c "adapter speed 4000" -c "program $(PROJECT).bin verify reset exit"

clangd: # configure clangd for tests
	bash ./tools/clangd_generator.sh

.PHONY: all clean re flash clangd

###############################################################################
${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

${BUILD_DIR}/%.o: ${SRC_DIR}/%.c | ${BUILD_DIR}
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

${BUILD_DIR}/%.o: ${SYS_SRCS_DIR}/%.c | ${BUILD_DIR}
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	mkdir -p $(BUILD_DIR)
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 $(INCLUDE_PATHS) -o $@ $<

# This is needed for NXP Cortex M devices
${NXPSUM}:
	$(CCLOCAL) tools/nxpsum.c -std=c99 -o ${NXPSUM}

$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(LIBRARIES) $(LD_SYS_LIBS)

$(PROJECT).bin: $(PROJECT).elf ${NXPSUM}
	$(OBJCOPY) -O binary $< $@
	# Compute nxp checksum on .bin file here
	${NXPSUM} $@
