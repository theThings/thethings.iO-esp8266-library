CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

ESPTOOL=/home/ferran/Workspace/thethings.io/esp8266/esp-open-sdk/esptool/esptool.py

led-0x00000.bin: led
	$(ESPTOOL) elf2image $^

led: led.o 

led.o: led.c

flash: led-0x00000.bin
	$(ESPTOOL) write_flash 0 led-0x00000.bin 0x40000 led-0x40000.bin

clean:
	rm *.o *.bin led
