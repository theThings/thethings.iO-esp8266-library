CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -I../../../libraries/c/ -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

ESPTOOL=esptool.py

button-0x00000.bin: button
	$(ESPTOOL) elf2image $^

button: button.o ../../../libraries/c/thethingsio.o

button.o: button.c user_config.h

../../../libraries/c/thethingsio.o: ../../../libraries/c/thethingsio.c

flash: button-0x00000.bin
	$(ESPTOOL) write_flash 0 button-0x00000.bin 0x40000 button-0x40000.bin

clean:
	rm *.o *.bin button
