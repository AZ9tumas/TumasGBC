gbc: main.o cpu.o cartridge.o debug.o
	gcc main.o cpu.o cartridge.o debug.o -o gbc
	rm *.o

main.o: main.c
	gcc -c main.c
cpu.o: cpu.h cpu.c
	gcc -c cpu.c
cartridge.o: cartridge.h cartridge.c
	gcc -c cartridge.c
debug.o: debug.h debug.c
	gcc -c debug.c
