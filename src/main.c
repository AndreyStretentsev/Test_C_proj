#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>
#include "main.h"

#include "storage.h"
#include "gif.h"
#include "gifdec.h"

#define BB_GIF_FILE_NAME    "bb.gif"
#define PUTIN_GIF_FILE_NAME "put.gif"
#define OCEAN_GIF_FILE_NAME "ocean.gif"
#define NEW_GIF_FILE_NAME   "copy.gif"

bool file_copy_to_storage_test(const char * filename, uint16_t id, uint32_t chunk_size) {
    file_t file;
    FILE *fp = NULL;
    f_err_t ret = FILE_OK;
    uint8_t __attribute__((aligned(4))) fbuf[chunk_size];

    LOGI("Open %s", filename);
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        LOGE("Can't open a file");
        return false;
    }
    struct stat stbuf;
    if (stat(filename, &stbuf) == -1) {
        LOGE("Can't access to the file");
        fclose(fp);
        return false;
    }
    uint32_t file_size = stbuf.st_size;
    LOGI("File size = %ld", file_size);
    LOGI("Creating the new file %ld in storage", id);
    ret = storage_file_create(&file, id, (uint32_t)stbuf.st_size);
    if (ret != FILE_OK) {
        LOGE("File not created. Reason = %d", ret);
        fclose(fp);
        return false;
    }
    while (file_size) {
        size_t r = fread(fbuf, sizeof(uint8_t), chunk_size, fp);
        LOGI("read %d, w_res %d", r, storage_file_write(&file, (uint32_t *)fbuf, r));
        file_size -= r;
    }
    fclose(fp);
    return true;
}

bool file_copy_from_storage_test(const char * filename, uint16_t id, uint32_t chunk_size) {
    file_t file;
    FILE *fp = NULL;
    f_err_t ret = FILE_OK;
    uint8_t fbuf[chunk_size];
    LOGI("Creating %s", filename);
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        LOGE("Can't create a file");
        return false;
    }
    ret = storage_get_file_by_id(&file, id);
    if (ret != FILE_OK) {
        LOGE("File not found. Reason = %d", ret);
        fclose(fp);
        return false;
    }
    LOGI("Copying file %ld contents", id);
    int r;
    do {
        r = storage_file_read(&file, fbuf, chunk_size);
        LOGI("read %d", r);
        fwrite(fbuf, sizeof(uint8_t), r, fp);
    } while (r != 0);
    fclose(fp);
    return true;
}

bool gif_test(uint16_t id) {
    file_t file;
    f_err_t ret_f = FILE_OK;
    gif_err_t ret_g = G_OK;
    ret_f = storage_get_file_by_id(&file, id);
    if (ret_f != FILE_OK) {
        LOGE("File not found. Reason = %d", ret_f);
        return false;
    }
    if (!is_gif_file(&file)) {
        LOGE("Not a GIF file");
        return false;
    }
    gif_t animation = {0};
    ret_g = gif_open(&file, &animation);
    if (ret_g != G_OK) {
        LOGE("gif_open returned %d", ret_g);
        return false;
    }
    char *buffer = malloc(animation.width * animation.height * 3);
    void *display = console_create_display(animation.width, animation.height);
    for (unsigned looped = 1;; looped++) {
        while (gif_get_frame(&animation) == 1) {
            gif_decode_n_render(&animation, buffer);
            console_display_image(display, buffer);
            Sleep(animation.gce.delay * 10);
        }
        if (looped == animation.loop_count)
            break;
        gif_rewind(&animation);
    }
    return true;
}

bool file_delete_test(uint16_t id) {
    file_t file;
    f_err_t ret = FILE_OK;
    LOGI("Deleting file %ld in storage", id);
    ret = storage_get_file_by_id(&file, id);
    if (ret != FILE_OK) {
        LOGE("File not found. Reason = %d", ret);
        return false;
    }
    ret = storage_file_delete(&file);
    if (ret != FILE_OK) {
        LOGE("File not deleted. Reason = %d", ret);
        return false;
    }
    return true;
}

bool run_tests() {
    uint16_t file1_id = storage_generate_id();
    if (!file_copy_to_storage_test(BB_GIF_FILE_NAME, file1_id, 4080))
        goto test_fail;

    uint16_t file2_id = storage_generate_id();
    if (!file_copy_to_storage_test(BB_GIF_FILE_NAME, file2_id, 4080))
        goto test_fail;

    uint16_t file3_id = storage_generate_id();
    if (!file_copy_to_storage_test(BB_GIF_FILE_NAME, file3_id, 4080))
        goto test_fail;

    if (!file_delete_test(file2_id))
        goto test_fail;

    if (!file_copy_to_storage_test(OCEAN_GIF_FILE_NAME, file2_id, 4080))
        goto test_fail;

    if (!file_copy_from_storage_test(NEW_GIF_FILE_NAME, file2_id, 4080))
        goto test_fail;

    // if (!gif_test(file1_id))
    //     goto test_fail;

    LOGI("All test results are successfull!");
    return true;
test_fail:
    LOGE("Tests failed!");
    return false;
}

void gifdec_teset(const char *filename) {
    gd_GIF *gif = gd_open_gif(filename);
    char *buffer = malloc(gif->width * gif->height * 3);
    void *display = console_create_display(gif->width, gif->height);
    for (unsigned looped = 1;; looped++) {
        while (gd_get_frame(gif)) {
            gd_render_frame(gif, buffer);
            console_display_image(display, buffer);
            Sleep(gif->gce.delay * 10);
        }
        if (looped == gif->loop_count)
            break;
        gd_rewind(gif);
    }
    free(buffer);
    gd_close_gif(gif);
}

int main() {
    flash_init();
    storage_init();
    if (!run_tests())
        exit(1);
    LOGD("");
    // gifdec_teset("bb.gif");
    exit(0);
}
