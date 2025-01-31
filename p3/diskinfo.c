#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "diskfunctions.h"

int main (int argc, char* argv[]){

    FILE* disk = fopen(argv[1], "rb+"); 
    if (disk == NULL) {
        perror("Error opening file");
        return 1;
    }
    char* buffer = (char*)malloc(BLOCK_SIZE);
    readBlock(disk, 0, buffer);

    struct superblock_t superblock;
    getSuperBlock(disk,&superblock);


    //iterate through FAT
    uint32_t free_blocks = 0;
    uint32_t reserved_blocks = 0;
    uint32_t allocated_blocks = 0;
    char* fat_buffer = (char*)malloc(BLOCK_SIZE);

    for (int i = 0; i < superblock.fat_block_count; i++) { //for every block in FAT
        readBlock(disk, superblock.fat_start_block + i, fat_buffer); //read block into buffer

        // Iterate trough each fat entry
        for (int j = 0; j < BLOCK_SIZE; j += 4) {
            uint32_t fat_entry = concatToHex(fat_buffer[j], fat_buffer[j + 1], fat_buffer[j + 2], fat_buffer[j + 3]);
            if (fat_entry == 0x00000000) {
                free_blocks++;
            } else if (fat_entry == 0x00000001) {
                reserved_blocks++;
            } else {
                allocated_blocks++;
            }
        }
    }

    free(fat_buffer);

    printf("Super block information: \n");
    printf("Block size: %u\n",superblock.block_size);
    printf("Block count: %u\n",superblock.block_count);
    printf("FAT starts: %u\n",superblock.fat_start_block);
    printf("FAT blocks: %u\n",superblock.fat_block_count);
    printf("Root directory start: %u\n",superblock.root_dir_start_block);
    printf("Root directory blocks: %u\n\n",superblock.root_dir_block_count);
    
    printf("FAT information:\n");
    printf("Free Blocks: %u\n", free_blocks);
    printf("Reserved Blocks: %u\n", reserved_blocks);
    printf("Allocated Blocks: %u\n", allocated_blocks);

    free(buffer);
    fclose(disk);

    return 0;
}