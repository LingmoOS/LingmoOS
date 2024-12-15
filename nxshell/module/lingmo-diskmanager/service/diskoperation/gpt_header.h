// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef GPT_HEADER_H
#define GPT_HEADER_H

#include <stdint.h>
/* GPT header */
typedef struct gpt_header {
        uint64_t            signature; /* header identification */
        uint32_t            revision; /* header version */
        uint32_t            size; /* in bytes */
        uint32_t            crc32; /* header CRC checksum */
        uint32_t            reserved1; /* must be 0 */
        uint64_t            my_lba; /* LBA of block that contains this struct (LBA 1) */
        uint64_t            alternative_lba; /* backup GPT header */
        uint64_t            first_usable_lba; /* first usable logical block for partitions */
        uint64_t            last_usable_lba; /* last usable logical block for partitions */
} __attribute__ ((packed)) gpt_header_t;




#endif // GPT_HEADER_H
