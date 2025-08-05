/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#include "encoding.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define TEST_STRING "hello world!你好世界켐ㅇㄹ貴方元気？☺"

void test_conversion(const char *str) {

    size_t len;
    char *result = xcb_utf8_to_compound_text(str, strlen(str), &len);

    char *utf8_result = xcb_compound_text_to_utf8(result, len, NULL);

    assert(strcmp(utf8_result, str) == 0);

    free(result);
    free(utf8_result);
}

int main() {
    xcb_compound_text_init();
    test_conversion(TEST_STRING);
    test_conversion("\xe2\x80\x93");

    return 0;
}
