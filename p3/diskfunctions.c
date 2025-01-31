#include "diskfunctions.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

const int BLOCK_SIZE = 512;


uint32_t concatToHex(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d;
}


void readBlock(FILE* disk, int blockNum, char* buffer) {
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}


int getSuperBlock(FILE* disk, struct superblock_t* superblock) {
    char* buffer = (char*)malloc(BLOCK_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate memory for buffer");
        return 1;
    }


    readBlock(disk, 0, buffer);

    uint8_t high = buffer[8];
    uint8_t low = buffer[9];
    superblock->block_size = (high << 8) | low;

    superblock->block_count = concatToHex(buffer[10], buffer[11], buffer[12], buffer[13]);
    superblock->fat_start_block = concatToHex(buffer[14], buffer[15], buffer[16], buffer[17]);
    superblock->fat_block_count = concatToHex(buffer[18], buffer[19], buffer[20], buffer[21]);
    superblock->root_dir_start_block = concatToHex(buffer[22], buffer[23], buffer[24], buffer[25]);
    superblock->root_dir_block_count = concatToHex(buffer[26], buffer[27], buffer[28], buffer[29]);

    free(buffer);
    return 0;
}
