PRU_SDK_DIR=../pru_sdk

C_FILES := host_main.c $(PRU_SDK_DIR)/example/common/mio.c
DTS_FILES := pru_enable-00A0.dts

CC := gcc
LD := gcc
STRIP := strip
PASM := pasm
DTC := dtc

C_FLAGS := -DSTART_ADDR=`cat START_ADDR`
C_FLAGS += -Wall -O2 -mtune=cortex-a8 -march=armv7-a
C_FLAGS += -I$(PRU_SDK_DIR)/include -I$(PRU_SDK_DIR)/example/

L_FLAGS += -L$(PRU_SDK_DIR)/lib
L_LIBS += -lprussdrv

BIN_FILES := $(P_FILES:.p=.bin)
O_FILES := $(C_FILES:.c=.o)
DTBO_FILES := $(DTS_FILES:.dts=.dtbo)

all:	main $(BIN_FILES) $(DTBO_FILES)

main:	$(O_FILES)
	$(LD) -static -o $@ $(O_FILES) $(L_FLAGS) $(L_LIBS)
	$(STRIP) $@

%.bin : %.p
	$(PASM) -V2 -I$(PRU_SDK_DIR)/include -b $<

%.o : %.c
	$(CC) $(C_FLAGS) -c -o $@ $<

%.dtbo : %.dts
	$(DTC) -@ -O dtb -o $@ $<

.PHONY	: clean all
clean:
	rm -f $(O_FILES) $(BIN_FILES) $(DTBO_FILES) main

cleanall: clean
	rm -f data.bin text.bin START_ADDR pru_main.asm pru_hal.asm

