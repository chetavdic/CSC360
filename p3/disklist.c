#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "diskfunctions.h"

struct __attribute__((__packed__)) dir_entry_timedate_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};


struct __attribute__((__packed__)) dir_entry_t {
    uint8_t status;                
    uint32_t starting_block;      
    uint32_t block_count;         
    uint32_t size;               
    struct dir_entry_timedate_t create_time; 
    struct dir_entry_timedate_t modify_time; 
    uint8_t filename[31];          
    uint8_t unused[6];             
};
//read big endian 16 bit
uint16_t read_uint16_be(const uint8_t *ptr) {
    return (ptr[0] << 8) | ptr[1];
}
//read big edian 32 bit
uint32_t read_uint32_be(const uint8_t *ptr) {
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}


void formatDateTime(struct dir_entry_timedate_t *timedate, char *formatted_date, char *formatted_time) {

    snprintf(formatted_date, 11, "%04d/%02d/%02d",timedate->year, timedate->month, timedate->day);
    snprintf(formatted_time, 9, "%02d:%02d:%02d",timedate->hour, timedate->minute, timedate->second);
}

void listDirectory(FILE *disk, uint32_t start_block, uint32_t block_count) {

    uint8_t *dir_buffer = (uint8_t *)malloc(BLOCK_SIZE);

   
    for (int i = 0; i < block_count; i++) {
        readBlock(disk, start_block + i, dir_buffer); // Read block into buffer

        for (int offset = 0; offset < BLOCK_SIZE; offset += 64) {
            uint8_t *entry_ptr = dir_buffer + offset;

            // Check status
            uint8_t status = entry_ptr[0];
            if (!(status & 0x01)) {
                continue; 
            }

            // Check type
            char type = '-';
            if (status & 0x02) {
                type = 'F'; 
            } else if (status & 0x04) {
                type = 'D'; 
            }

            uint32_t size = read_uint32_be(entry_ptr + 9);
            uint16_t year = read_uint16_be(entry_ptr + 13);
            uint8_t month = entry_ptr[15];
            uint8_t day = entry_ptr[16];
            uint8_t hour = entry_ptr[17];
            uint8_t minute = entry_ptr[18];
            uint8_t second = entry_ptr[19];

            
            char filename[31];
            memcpy(filename, entry_ptr + 27, 31);
            filename[30] = '\0';

            
            struct dir_entry_timedate_t create_time = {year, month, day, hour, minute, second};
            char formatted_date[11], formatted_time[9];
            formatDateTime(&create_time, formatted_date, formatted_time);


            printf("%c %10u %30s %s %s\n",type, size, filename, formatted_date, formatted_time);
        }
    }

    free(dir_buffer);
}

int findSubdirectory(FILE *disk, uint32_t start_block, uint32_t block_count, const char *subdir_name, uint32_t *subdir_start_block, uint32_t *subdir_block_count) {
    uint8_t *dir_buffer = (uint8_t *)malloc(BLOCK_SIZE);

    // Iterate through each block in the directory
    for (int i = 0; i < block_count; i++) {
        readBlock(disk, start_block + i, dir_buffer); // Read block into buffer

        for (int offset = 0; offset < BLOCK_SIZE; offset += 64) {
            uint8_t *entry_ptr = dir_buffer + offset;

            // Check status
            uint8_t status = entry_ptr[0];
            if (!(status & 0x01)) {
                continue; 
            }

            // Check type for dir
            if (!(status & 0x04)) {
                continue; 
            }

            char filename[31];
            memcpy(filename, entry_ptr + 27, 31);
            filename[30] = '\0';

            if (strcmp(filename, subdir_name) == 0) {
                *subdir_start_block = read_uint32_be(entry_ptr + 1);
                *subdir_block_count = read_uint32_be(entry_ptr + 5);
                free(dir_buffer);
                return 1; 
            }
        }
    }

    free(dir_buffer);
    return 0; 
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <disk_image> <directory_path>\n", argv[0]);
        return 1;
    }

    FILE *disk = fopen(argv[1], "rb+"); 
    if (disk == NULL) {
        perror("Error opening file");
        return 1;
    }

    const char *path = argv[2]; 

    struct superblock_t superblock;
    getSuperBlock(disk, &superblock); 

    uint32_t start_block = superblock.root_dir_start_block;
    uint32_t block_count = superblock.root_dir_block_count;

    // If path is "/", root, else find subdir
    if (strcmp(path, "/") == 0) {
        listDirectory(disk, start_block, block_count);
    } else {

        char *subdir_name = strdup(path + 1); 
        if (findSubdirectory(disk, start_block, block_count, subdir_name, &start_block, &block_count)) {
            listDirectory(disk, start_block, block_count);
        } else {
            fprintf(stderr, "Error: Subdirectory not found.\n");
        }
        free(subdir_name);
    }

    fclose(disk);
    return 0;
}
