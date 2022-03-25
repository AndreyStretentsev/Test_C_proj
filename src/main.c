#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "main.h"

#include "storage.h"

#define GIF_FILE_NAME   "D:\\Projects\\Test_c_proj\\ezgif.com-gif-maker.gif"
#define NEW_GIF_FILE_NAME   "D:\\Projects\\Test_c_proj\\gumno.gif"
#define FILE_CHUNK_SIZE 512

uint8_t __attribute__((aligned(4))) fbuf[FILE_CHUNK_SIZE];

int main() {
    flash_init();
    storage_init();
    file_t file;
    FILE *fp = NULL;

    printf("Open the file\n");
    fp = fopen(GIF_FILE_NAME, "rb");
    if (fp == NULL) {
        printf("Can't open a file\n");
        exit(-1);
    }
    struct stat stbuf;
    if (stat(GIF_FILE_NAME, &stbuf) == -1) {
        printf("Can't access to the file\n");
        fclose(fp);
        exit(-1);
    }
    uint32_t file_size = stbuf.st_size;
    printf("File size = %ld\n", file_size);
    printf("Creating the new file in storage\n");
    printf("result = %d\n", storage_file_create(&file, 1, (uint32_t)stbuf.st_size));
    while (file_size) {
        size_t r = fread(fbuf, sizeof(uint8_t), FILE_CHUNK_SIZE, fp);
        printf("read %d, w_res %d\n", r, storage_file_write(&file, (uint32_t *)fbuf, r));
        file_size -= r;
    }
    fclose(fp);
    fp = NULL;
    printf("Creating the new file in project directory\n");
    fp = fopen(NEW_GIF_FILE_NAME, "wb");
    if (fp == NULL) {
        printf("Can't create a file\n");
        exit(-1);
    }
    storage_file_set_cursor(&file, 0);
    f_error_t ret = FILE_OK;
    printf("Copying file contents\n");
    do {
        ret = storage_file_read(&file, (uint32_t *)fbuf, FILE_CHUNK_SIZE);
        fwrite(fbuf, sizeof(uint8_t), FILE_CHUNK_SIZE, fp);
    } while (ret != FILE_EOF);
    fclose(fp);
    printf("Done\n");
    exit(0);
}
