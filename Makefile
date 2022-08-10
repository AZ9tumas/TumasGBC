CFLAGS = -O2 `sdl2-config --cflags`
LFLAGS = -O2 `sdl2-config --libs`

gbc: main.o cpu.o cartridge.o debug.o display.o emulator.o
	gcc main.o cpu.o cartridge.o debug.o display.o emulator.o $(LFLAGS) -o gbc
	rm *.o
main.o: cpu.h main.c
	gcc -c main.c $(CFLAGS)
cpu.o: cpu.h cpu.c
	gcc -c cpu.c $(CFLAGS)
cartridge.o: cartridge.h cartridge.c
	gcc -c cartridge.c $(CFLAGS)
debug.o: debug.h debug.c
	gcc -c debug.c $(CFLAGS)
display.o: display.h display.c
	gcc -c display.c $(CFLAGS)
emulator.o: emulator.h emulator.c
	gcc -c emulator.c $(CFLAGS)
