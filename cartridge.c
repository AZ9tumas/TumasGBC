#include "cartridge.h"

bool initCartridge(Cartridge* c, uint8_t* data, size_t size){
    if (data == NULL || size < 0x4000) {
        printf(":(\n");
    }

    c->file = data;

    /* Set the logo */
    memcpy(c->logoChecksum, &data[0x104], 0x30);
    /* Set the title */
    memcpy(c->title, &data[0x134], 11);
    /* Set the manufacturer code */
    memcpy(c->manufacturerCode, &data[0x13F], 4);
    /* New Licencee Code */
    c->lCode = data[0x145];
    /* Set the cartridge type */
    c->cType = data[0x147];
    /* Set the rom size */
    c->romSize = data[0x148];
    /* Set the ram size */
    c->extRamSize = data[0x149];
    /* Set destination code */
    c->dCode = data[0x14A];
    /* Set old lincencee code */
    c->oldLCode = data[0x14B];
    /* Set rom version */
    c->romVersion = data[0x14C];
    /* Set header checksum */
    c->headerChecksum = data[0x14D];
    /* Set global checksum */
    c->globalChecksum = (data[0x14E] << 8) | data[0x14F];
    
    c->inserted = false;
    return true;
}

static char* toStrLicenceeCode(LICENCEE_CODE code) {
    switch (code) {
        case LC_NONE: return "None";
        case LC_NINTENDO_R_AND_D1: return "Nintendo R&D1";
        case LC_CAPCOM: return "Capcom";
        case LC_ELECTRONIC_ARTS: return "Electronic Arts";
        case LC_HUDSON_SOFT: return "Hudson Soft";
        case LC_B_AI: return "b-ai";
        case LC_KSS: return "kss";
        case LC_POW: return "pow";
        case LC_PCM_COMPLETE: return "PCM Complete";
        case LC_SAN_X: return "san-x";
        case LC_KEMCO_JAPAN: return "Kemco Japan";
        case LC_SETA: return "seta";
        case LC_VIACOM: return "Viacom";
        case LC_NINTENDO: return "Nintendo";
        case LC_BANDAI: return "Bandai";
        case LC_OCEAN_ACCLAIM: return "Ocean/Acclaim";
        case LC_KONAMI: return "Konami";
        default: return "Unknown";                  /* Or not added yet */
    }
}

static char* toStrCartridgeType(CARTRIDGE_TYPE code) {
    switch (code) {
        case CARTRIDGE_MBC1: return "MBC1";
        case CARTRIDGE_MBC1_RAM: return "MBC1 + RAM";
        case CARTRIDGE_MBC1_RAM_BATTERY: return "MBC1 + RAM + Battery";
        case CARTRIDGE_MBC2: return "MBC2";
        case CARTRIDGE_MBC2_BATTERY: return "MBC2 + Battery";
        case CARTRIDGE_NONE: return "None";
        case CARTRIDGE_ROM_RAM: return "ROM + RAM";
        case CARTRIDGE_ROM_RAM_BATTERY: return "ROM + RAM + BATTERY";
        case CARTRIDGE_MMM01: return "MMM01";
        case CARTRIDGE_MMM01_RAM: return "MMM01 + RAM";
        case CARTRIDGE_MMM01_RAM_BATTERY: return "MMM01 + RAM + BATTERY";
        case CARTRIDGE_MBC3_TIMER_BATTERY: return "MBC3 + TIMER + BATTERY";
        case CARTRIDGE_MBC3_TIMER_RAM_BATTERY: return "MBC3 + TIMER + RAM + BATTERY";
        case CARTRIDGE_MBC3: return "MBC3";
        case CARTRIDGE_MBC3_RAM: return "MBC3 + RAM";
        case CARTRIDGE_MBC3_RAM_BATTERY: return "MBC3 + RAM + BATTERY";
        case CARTRIDGE_MBC5: return "MBC5";
        case CARTRIDGE_MBC5_RAM: return "MBC5 + RAM";
        case CARTRIDGE_MBC5_RAM_BATTERY: return "MBC5 + RAM + BATTERY";
        case CARTRIDGE_MBC5_RUMBLE: return "MBC5 + RUMBLE";
        case CARTRIDGE_MBC5_RUMBLE_RAM: return "MBC5 + RUMBLE + RAM";
        case CARTRIDGE_MBC5_RUMBLE_RAM_BATTERY: return "MBC5 + RUMBLE + RAM + BATTERY";
        case CARTRIDGE_MBC6: return "MBC6";
        case CARTRIDGE_MBC7_SENSOR_RUMBLE_RAM_BATTERY: return "MBC7 + SENSOR + RUMBLE + RAM + BATTERY";
        case CARTRIDGE_POCKET_CAMERA: return "+POCKET CAMERA";
        case CARTRIDGE_BANDAI_TAMA: return "+BANDAI TAMA";
        case CARTRIDGE_HUC3: return "HuC3";
        case CARTRIDGE_HUC1_RAM_BATTERY: return "HuC1 + RAM + BATTERY";
        default: return "Unknown";
    }
}

static char* toStrRomSize(ROM_SIZE code) {
    switch (code) {
        case ROM_32KB: return "32 KB";
        case ROM_64KB: return "64 KB";
        case ROM_128KB: return "128 KB";
        case ROM_256KB: return "256 KB";
        case ROM_512KB: return "512 KB";
        case ROM_1MB: return "1 MB";
        case ROM_2MB: return "2 MB";
        case ROM_4MB: return "4 MB";
        case ROM_8MB: return "8 MB";
        default: return "Unknown";
    }
}

static char* toStrRamSize(RAM_SIZE code) {
    switch (code) {
        case EXT_RAM_0: return "No External Ram";
        case EXT_RAM_2KB: return "2 KB EXT RAM";
        case EXT_RAM_8KB: return "8 KB EXT RAM";
        case EXT_RAM_32KB: return "32 KB EXT RAM";
        case EXT_RAM_64KB: return "84 KB EXT RAM";
        case EXT_RAM_128KB: return "128 KB EXT RAM";
        default: return "Unknown";
    }
}

void printCartridge(Cartridge* c) {
    printf("== CARTRIDGE DETAILS ==\n");
    // ez
    printf("Title : %11s\n", c->title);
    printf("Licencee Code : %s\n", toStrLicenceeCode(c->lCode));
    printf("Cartridge Type : %s\n", toStrCartridgeType(c->cType));
    printf("ROM Size : %s\n", toStrRomSize(c->romSize));
    printf("External RAM Size : %s\n", toStrRamSize(c->extRamSize));
    printf("Rom Mask Version : %d\n", c->romVersion);
    printf("Legacy Licencee Code : %x\n", c->oldLCode);
    printf("Destination Code : %s\n", c->dCode == DEST_CODE_JP ? "Japan" : "International");

    printf("== MADY BY AZ9TUMAS ==\n");
}

void freeCartridge(Cartridge* c){
    free(c->file);
}