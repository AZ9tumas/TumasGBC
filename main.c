#include "cpu.h"

int main(int argc, char* argv[]){
    //printf("Welcome to Tumas GBC ...\n");

    Emulator emu;

    initEmulator(&emu);

    if (argc > 1) {
        //printf("Found Input files\n");

        char* filePath = argv[1];
        FILE* file = fopen(filePath, "r");
        
        if (file == NULL) {
            printf("Cannot open file\n");
            exit(10);
        }

        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate space...
        uint8_t* memory = (uint8_t*)malloc(size);
        size_t hell = fread(memory, size, 1, file);
        fclose(file);

        Cartridge cart;
        initCartridge(&cart, memory, size);
        //printCartridge(&cart);
        // Start Emulator here if we found a file
        startEmulator(&cart, &emu);
    } else {
        printf("No input files\n");
    }

    
    return 0;
}
