#ifndef DISKFUNCTIONS_H
#define DISKFUNCTIONS_H

#include <stdint.h>
#include <stdio.h>

extern const int BLOCK_SIZE;

struct __attribute__((__packed__)) superblock_t {
    uint16_t block_size;
    uint32_t block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

int getSuperBlock(FILE* disk, struct superblock_t* superblock);
void readBlock(FILE* disk, int blockNum, char* buffer);
uint32_t concatToHex(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

#endif // DISKFUNCTIONS_H
