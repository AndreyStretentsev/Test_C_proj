#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "main.h"

#include "storage.h"

#define BB_GIF_FILE_NAME    "bb.gif"
#define PUTIN_GIF_FILE_NAME "put.gif"
#define OCEAN_GIF_FILE_NAME "ocean.gif"
#define NEW_GIF_FILE_NAME   "copy.gif"

bool file_copy_to_storage_test(const char * filename, uint16_t id, uint32_t chunk_size) {
    file_t file;
    FILE *fp = NULL;
    f_error_t ret = FILE_OK;
    uint8_t __attribute__((aligned(4))) fbuf[chunk_size];

    printf("Open %s\n", filename);
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Can't open a file\n");
        return false;
    }
    struct stat stbuf;
    if (stat(filename, &stbuf) == -1) {
        printf("Can't access to the file\n");
        fclose(fp);
        return false;
    }
    uint32_t file_size = stbuf.st_size;
    printf("File size = %ld\n", file_size);
    printf("Creating the new file %ld in storage\n", id);
    ret = storage_file_create(&file, id, (uint32_t)stbuf.st_size);
    if (ret != FILE_OK) {
        printf("File not created. Reason = %d\n", ret);
        fclose(fp);
        return false;
    }
    while (file_size) {
        size_t r = fread(fbuf, sizeof(uint8_t), chunk_size, fp);
        printf("read %d, w_res %d\n", r, storage_file_write(&file, (uint32_t *)fbuf, r));
        file_size -= r;
    }
    fclose(fp);
    return true;
}

bool file_copy_from_storage_test(const char * filename, uint16_t id, uint32_t chunk_size) {
    file_t file;
    FILE *fp = NULL;
    f_error_t ret = FILE_OK;
    uint8_t __attribute__((aligned(4))) fbuf[chunk_size];
    printf("Creating %s\n", filename);
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("Can't create a file\n");
        return false;
    }
    ret = storage_get_file_by_id(&file, id);
    if (ret != FILE_OK) {
        printf("File not found. Reason = %d\n", ret);
        fclose(fp);
        return false;
    }
    printf("Copying file %ld contents\n", id);
    do {
        ret = storage_file_read(&file, (uint32_t *)fbuf, chunk_size);
        fwrite(fbuf, sizeof(uint8_t), chunk_size, fp);
    } while (ret != FILE_EOF);
    fclose(fp);
    return true;
}

bool file_delete_test(uint16_t id) {
    file_t file;
    f_error_t ret = FILE_OK;
    printf("Deleting file %ld in storage\n", id);
    ret = storage_get_file_by_id(&file, id);
    if (ret != FILE_OK) {
        printf("File not found. Reason = %d\n", ret);
        return false;
    }
    ret = storage_file_delete(&file);
    if (ret != FILE_OK) {
        printf("File not deleted. Reason = %d\n", ret);
        return false;
    }
    return true;
}

bool run_tests() {
    uint16_t file1_id = 0;
    while (file1_id == 0)
        file1_id = rand();
    if (!file_copy_to_storage_test(BB_GIF_FILE_NAME, file1_id, 512))
        goto test_fail;

    uint16_t file2_id = 0;
    while (file2_id == 0)
        file2_id = rand();
    if (!file_copy_to_storage_test(BB_GIF_FILE_NAME, file2_id, 512))
        goto test_fail;

    uint16_t file3_id = 0;
    while (file3_id == 0)
        file3_id = rand();
    if (!file_copy_to_storage_test(BB_GIF_FILE_NAME, file3_id, 512))
        goto test_fail;

    if (!file_delete_test(file2_id))
        goto test_fail;

    if (!file_copy_to_storage_test(OCEAN_GIF_FILE_NAME, file2_id, 512))
        goto test_fail;

    if (!file_copy_from_storage_test(NEW_GIF_FILE_NAME, file2_id, 512))
        goto test_fail;

    printf("All test results are successfull!\n");
    return true;
test_fail:
    printf("Tests failed!\n");
    return false;
}

int main() {
    flash_init();
    storage_init();
    if (!run_tests())
        exit(1);
    exit(0);
}
