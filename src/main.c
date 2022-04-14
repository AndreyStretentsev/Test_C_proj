#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>
#include "main.h"

#include "flash.h"
#include "storage.h"
#include "gif.h"
#include "gifdec.h"
#include "text.h"

#define BB_GIF_FILE_NAME    "bb.gif"
#define OUT_BB_GIF_FILE_NAME "out_bb.gif"
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
        int w = storage_file_write(&file, (uint32_t *)fbuf, r);
        LOGI("read %d, w_res %d", r, w);
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
    while (gif_get_frame(&animation) == 1) {
        gif_decode_n_render(&animation, buffer);
        console_display_image(display, buffer);
        Sleep(animation.gce.delay * 10);
    }
    gif_rewind(&animation);
    console_delete_display(display);
    free(buffer);
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

extern font_info_t zXSpectrum7_8ptFontInfo;

bool text_test() {
    uint8_t cc[DISP_LEDS_NUM] = {0xFF, 0x0, 0x0};
    uint8_t bc[DISP_LEDS_NUM] = {0, 0, 0};
    char h_str[] = "Horizontal scroll\0";
    char v_str[] = "Vertical\0";
    uint8_t *disp_buf = malloc(DISP_COLS_NUM * DISP_ROWS_NUM * DISP_LEDS_NUM);
    memset(disp_buf, 0, DISP_COLS_NUM * DISP_ROWS_NUM * DISP_LEDS_NUM);
    void *display = console_create_display(DISP_COLS_NUM, DISP_ROWS_NUM);
    int x_max = text_strlen_px(&zXSpectrum7_8ptFontInfo, h_str);
    LOGI("strlen in px = %d", x_max);
    for (int x = DISP_COLS_NUM; x > 0; x--) {
        if (text_draw_string(
            disp_buf, 
            &zXSpectrum7_8ptFontInfo, 
            h_str, 
            x, 1, DISP_COLS_NUM, DISP_ROWS_NUM, 
            0, 0, 
            cc, bc
        ) != 0) {
            console_delete_display(display);
            LOGE("fail");
            free(disp_buf);
            return false;
        }
        console_display_image(display, disp_buf);
        memset(disp_buf, 0, DISP_COLS_NUM * DISP_ROWS_NUM * DISP_LEDS_NUM);
        Sleep(100);
    }
    for (int x = 0; x < x_max; x++) {
        if (text_draw_string(
            disp_buf, 
            &zXSpectrum7_8ptFontInfo, 
            h_str, 
            0, 1, DISP_COLS_NUM, DISP_ROWS_NUM, 
            x, 0, 
            cc, bc
        ) != 0) {
            console_delete_display(display);
            LOGE("fail");
            free(disp_buf);
            return false;
        }
        console_display_image(display, disp_buf);
        memset(disp_buf, 0, DISP_COLS_NUM * DISP_ROWS_NUM * DISP_LEDS_NUM);
        Sleep(100);
    }
    for (int y = DISP_ROWS_NUM; y > 0; y--) {
        if (text_draw_string(
            disp_buf, 
            &zXSpectrum7_8ptFontInfo, 
            v_str, 
            0, y, DISP_COLS_NUM, DISP_ROWS_NUM, 
            0, 0, 
            cc, bc
        ) != 0) {
            console_delete_display(display);
            LOGE("fail");
            free(disp_buf);
            return false;
        }
        console_display_image(display, disp_buf);
        memset(disp_buf, 0, DISP_COLS_NUM * DISP_ROWS_NUM * DISP_LEDS_NUM);
        Sleep(100);
    }
    for (int y = 0; y < zXSpectrum7_8ptFontInfo.character_height; y++) {
        if (text_draw_string(
            disp_buf, 
            &zXSpectrum7_8ptFontInfo, 
            v_str, 
            0, 0, DISP_COLS_NUM, DISP_ROWS_NUM, 
            0, y, 
            cc, bc
        ) != 0) {
            console_delete_display(display);
            LOGE("fail");
            free(disp_buf);
            return false;
        }
        console_display_image(display, disp_buf);
        memset(disp_buf, 0, DISP_COLS_NUM * DISP_ROWS_NUM * DISP_LEDS_NUM);
        Sleep(100);
    }
    console_delete_display(display);
    free(disp_buf);
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

    uint16_t file4_id = storage_generate_id();
    if (!file_copy_to_storage_test(OUT_BB_GIF_FILE_NAME, file4_id, 4080))
        goto test_fail;

    if (!file_delete_test(file2_id))
        goto test_fail;

    if (!file_copy_to_storage_test(OCEAN_GIF_FILE_NAME, file2_id, 4080))
        goto test_fail;

    if (!file_copy_from_storage_test(NEW_GIF_FILE_NAME, file2_id, 4080))
        goto test_fail;

    if (!gif_test(file4_id))
        goto test_fail;

    // if (!text_test())
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
#if 0 // internal pelette file bmp
char bmp_header[] = {
    0x42, 0x4d, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
    0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xD0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};

#include "internal_palette.h"

uint8_t int_pal[] = GIF_INTERNAL_PALETTE;
uint8_t ending[] = {0, 0};

FILE *f = fopen("internal_palette.bmp", "wb");
fwrite(bmp_header, sizeof(uint8_t), sizeof(bmp_header), f);
for (int y = 35; y >= 0; y--) {
    for (int x = 0; x < 6; x++) {
        for (int c = 1; c <= DISP_LEDS_NUM; c++) {
            LOGI("%d, %d, %d", y, x, c);
            LOGI("%d", (y * 6 + x + 1) * DISP_LEDS_NUM - c);
            fwrite(&int_pal[(y * 6 + x + 1) * DISP_LEDS_NUM - c], 1, 1, f);
        }
    }
    fwrite(ending, 1, 2, f);
}
fclose(f);

#endif

int main() {
    flash_init();
    storage_init();
    if (!run_tests())
        exit(1);
    // gifdec_teset("bb.gif");
    
    exit(0);
}
