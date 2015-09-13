all: text.bin

# The default target is building text.bin and data.bin.
# When the "copy" target (make copy) is used, START_ADDR
# is created and the binary files including START_ADDR
# are copied to the Beaglebone Black by ./copy_files_over.sh

PRU_SDK_DIR=../pru_sdk
PRU_CGT_DIR=../pru_2.0.0B2

SCP_SRC=data.bin text.bin pru_main.asm pru_hal.asm START_ADDR
SCP_DST=bbb:~/dev/pru_c_toolchain_demo/pru_sdk/example/pruss_c_adc

pru_hal.obj: pru_hal.c
	${PRU_CGT_DIR}/bin/clpru --silicon_version=2 --hardware_mac=on -i${PRU_CGT_DIR}/include -i${PRU_CGT_DIR}/lib -c -s pru_hal.c

pru_main.obj: pru_main.c
	${PRU_CGT_DIR}/bin/clpru --silicon_version=2 --hardware_mac=on -i${PRU_CGT_DIR}/include -i${PRU_CGT_DIR}/lib -O3 -c -s pru_main.c

pru_main.elf: pru_hal.obj pru_main.obj
	${PRU_CGT_DIR}/bin/clpru --silicon_version=2 --hardware_mac=on -i${PRU_CGT_DIR}/include -i${PRU_CGT_DIR}/lib -z pru_main.obj pru_hal.obj -llibc.a -m pru_main.map -o pru_main.elf ${PRU_CGT_DIR}/example/AM3359_PRU.cmd

text.bin: pru_main.elf
	${PRU_CGT_DIR}/bin/hexpru ${PRU_CGT_DIR}/bin.cmd ./pru_main.elf

START_ADDR: pru_main.elf
	echo 0x`${PRU_CGT_DIR}/bin/dispru pru_main.elf | grep _c_int00 | cut -f1 -d\  ` > START_ADDR

copy: START_ADDR text.bin
	scp ${SCP_SRC} ${SCP_DST}

clean:
	rm -f pru_hal.obj pru_hal.asm pru_main.asm pru_main.obj pru_main.map pru_main.elf data.bin text.bin START_ADDR

