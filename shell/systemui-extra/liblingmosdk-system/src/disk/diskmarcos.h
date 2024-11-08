/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef KDK_SYSTEM_DISK_MACROS_H__
#define KDK_SYSTEM_DISK_MACROS_H__

#ifdef	__linux__
enum {	/* ioctl() numbers */
	HDIO_DRIVE_CMD		= 0x031f,
	HDIO_DRIVE_RESET	= 0x031c,
	HDIO_DRIVE_TASK		= 0x031e,
	HDIO_DRIVE_TASKFILE	= 0x031d,
	HDIO_GETGEO		= 0x0301,
	HDIO_GETGEO_BIG		= 0x0330,
	HDIO_GET_32BIT		= 0x0309,
	HDIO_GET_ACOUSTIC	= 0x030f,
	HDIO_GET_BUSSTATE	= 0x031a,
	HDIO_GET_DMA		= 0x030b,
	HDIO_GET_IDENTITY	= 0x030d,
	HDIO_GET_KEEPSETTINGS	= 0x0308,
	HDIO_GET_MULTCOUNT	= 0x0304,
	HDIO_GET_NOWERR		= 0x030a,
	HDIO_GET_QDMA		= 0x0305,
	HDIO_GET_UNMASKINTR	= 0x0302,
	HDIO_OBSOLETE_IDENTITY	= 0x0307,
	HDIO_SCAN_HWIF		= 0x0328,
	HDIO_SET_32BIT		= 0x0324,
	HDIO_SET_ACOUSTIC	= 0x032c,
	HDIO_SET_BUSSTATE	= 0x032d,
	HDIO_SET_DMA		= 0x0326,
	HDIO_SET_KEEPSETTINGS	= 0x0323,
	HDIO_SET_MULTCOUNT	= 0x0321,
	HDIO_SET_NOWERR		= 0x0325,
	HDIO_SET_PIO_MODE	= 0x0327,
	HDIO_SET_QDMA		= 0x032e,
	HDIO_SET_UNMASKINTR	= 0x0322,
	HDIO_SET_WCACHE		= 0x032b,
	HDIO_TRISTATE_HWIF	= 0x031b,
	HDIO_UNREGISTER_HWIF	= 0x032a,
	CDROM__SPEED		= 0x5322,
};

#ifndef	DEFAULT_SECTOR_SIZE
#define	DEFAULT_SECTOR_SIZE	512
#endif

/* very basic ioctls, should be available everywhere */
# ifndef BLKROSET
#  define BLKROSET   _IO(0x12,93)	/* 将设备设置为只读 (0 = read-write) */
#  define BLKROGET   _IO(0x12,94)	/* 获取设备的只读属性状态 (0 = read_write) */
#  define BLKRRPART  _IO(0x12,95)	/* 重新获取分区表 */
#  define BLKGETSIZE _IO(0x12,96)	/* return device size /512 (long *arg) */
#  define BLKFLSBUF  _IO(0x12,97)	/* 刷新缓冲区 */
#  define BLKRASET   _IO(0x12,98)	/* set read ahead for block device */
#  define BLKRAGET   _IO(0x12,99)	/* get current read ahead setting */
#  define BLKFRASET  _IO(0x12,100)	/* set filesystem (mm/filemap.c) read-ahead */
#  define BLKFRAGET  _IO(0x12,101)	/* get filesystem (mm/filemap.c) read-ahead */
#  define BLKSECTSET _IO(0x12,102)	/* set max sectors per request (ll_rw_blk.c) */
#  define BLKSECTGET _IO(0x12,103)	/* get max sectors per request (ll_rw_blk.c) */
#  define BLKSSZGET  _IO(0x12,104)	/* get block device sector size */

/* ioctls introduced in 2.2.16, removed in 2.5.58 */
#  define BLKELVGET  _IOR(0x12,106,size_t) /* elevator get */
#  define BLKELVSET  _IOW(0x12,107,size_t) /* elevator set */

#  define BLKBSZGET  _IOR(0x12,112,size_t)
#  define BLKBSZSET  _IOW(0x12,113,size_t)
# endif /* !BLKROSET */

# ifndef BLKGETSIZE64
#  define BLKGETSIZE64 _IOR(0x12,114,size_t) /* return device size in bytes (u64 *arg) */
# endif

/* block device topology ioctls, introduced in 2.6.32 (commit ac481c20) */
# ifndef BLKIOMIN
#  define BLKIOMIN   _IO(0x12,120)
#  define BLKIOOPT   _IO(0x12,121)
#  define BLKALIGNOFF _IO(0x12,122)
#  define BLKPBSZGET _IO(0x12,123)
# endif

/* discard zeroes support, introduced in 2.6.33 (commit 98262f27) */
# ifndef BLKDISCARDZEROES
#  define BLKDISCARDZEROES _IO(0x12,124)
# endif

/* filesystem freeze, introduced in 2.6.29 (commit fcccf502) */
# ifndef FIFREEZE
#  define FIFREEZE   _IOWR('X', 119, int)    /* Freeze */
#  define FITHAW     _IOWR('X', 120, int)    /* Thaw */
# endif

/* uniform CD-ROM information */
# ifndef CDROM_GET_CAPABILITY
#  define CDROM_GET_CAPABILITY 0x5331
# endif

#endif	// __linux__

#endif
