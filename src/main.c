#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "main.h"

#include "storage.h"

#define GIF_FILE_NAME   "D:\\Projects\\VS\\cproj\\ezgif.com-gif-maker.gif"
#define NEW_GIF_FILE_NAME   "D:\\Projects\\VS\\cproj\\gumno.gif"
#define FILE_CHUNK_SIZE 512

uint8_t __attribute__((aligned(4))) fbuf[FILE_CHUNK_SIZE];

int main() {
    flash_init();
    storage_init();
    file_t file;
    FILE *fp = NULL;

    fp = fopen(GIF_FILE_NAME, "rb");
    if (fp == NULL) {
        printf("Can't open file\n");
        exit(-1);
    }
    struct stat stbuf;
    if (stat(GIF_FILE_NAME, &stbuf) == -1) {
        printf("Can't access to file\n");
        exit(-1);
    }
    uint32_t file_size = stbuf.st_size;
    printf("File size = %ld\n", file_size);
    printf("result = %d\n", storage_file_create(&file, 1, (uint32_t)stbuf.st_size));
    while (file_size) {
        size_t r = fread(fbuf, sizeof(uint8_t), FILE_CHUNK_SIZE, fp);
        printf("readed %d, w_res %d\n", r, storage_file_write(&file, (uint32_t *)fbuf, r));
        file_size -= r;
    }
    exit(0);
}
