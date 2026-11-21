// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdlib>
#include <cstring>
#include <unistd.h>

int main(int argc, char** argv)
{
    int fd = -1;
    for (int i = 1; i + 1 < argc; ++i) {
        if (strcmp(argv[i], "-displayfd") == 0)
            fd = atoi(argv[i + 1]);
    }
    const char* mode = getenv("LINGMO_XWAYLAND_FAULT");
    if (fd < 0 || !mode)
        return 1;
    if (strcmp(mode, "partial") == 0) {
        write(fd, "12", 2);
        close(fd);
    } else if (strcmp(mode, "malformed") == 0) {
        write(fd, "bad\n", 4);
    }
    for (;;)
        pause();
}
