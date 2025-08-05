/*
 * libdpkg - Debian packaging suite library routines
 * meminfo.c - system memory information functions
 *
 * Copyright © 2021 Sebastian Andrzej Siewior <sebastian@breakpoint.cc>
 * Copyright © 2021-2022 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <sys/stat.h>

#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <dpkg/dpkg.h>
#include <dpkg/fdio.h>
#include <dpkg/meminfo.h>

/*
 * An estimate of how much memory is available. Swap will not be used, the
 * page cache may be purged, not everything will be reclaimed that might be
 * reclaimed, watermarks are considered.
 */

struct meminfo_field {
	const char *name;
	ssize_t len;
	int tag;
	uint64_t value;
};
#define MEMINFO_FIELD(name, tag) name, sizeof(name) - 1, tag, 0

static struct meminfo_field *
meminfo_find_field(struct meminfo_field *fields, const size_t nfields,
               const char *fieldname, const ssize_t fieldlen)
{
	size_t f;

	for (f = 0; f < nfields; f++) {
		if (fieldlen != fields[f].len)
			continue;
		if (strncmp(fieldname, fields[f].name, fields[f].len) != 0)
			continue;

		return &fields[f];
	}

	return NULL;
}

static uint64_t
meminfo_sum_fields(const struct meminfo_field *fields, const size_t nfields)
{
	uint64_t sum = 0;
	size_t f;

	for (f = 0; f < nfields; f++)
		sum += fields[f].value;

	return sum;
}

enum meminfo_error_code
meminfo_get_available_from_file(const char *filename, uint64_t *val)
{
	char buf[4096];
	char *str;
	ssize_t bytes;
	int fd;
	struct meminfo_field fields[] = {
		{ MEMINFO_FIELD("MemFree", DPKG_BIT(0)) },
		{ MEMINFO_FIELD("Buffers", DPKG_BIT(1)) },
		{ MEMINFO_FIELD("Cached", DPKG_BIT(2)) },
	};
	const int want_tags = DPKG_BIT(array_count(fields)) - 1;
	int seen_tags = 0;

	*val = 0;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return MEMINFO_NO_FILE;

	bytes = fd_read(fd, buf, sizeof(buf));
	close(fd);

	if (bytes <= 0)
		return MEMINFO_NO_DATA;

	buf[bytes] = '\0';

	str = buf;
	while (1) {
		struct meminfo_field *field;
		char *end;

		end = strchr(str, ':');
		if (end == 0)
			break;

		field = meminfo_find_field(fields, array_count(fields),
		                           str, end - str);
		if (field) {
			intmax_t num;

			str = end + 1;
			errno = 0;
			num = strtoimax(str, &end, 10);
			if (num <= 0)
				return MEMINFO_INT_NEG;
			if ((num == INTMAX_MAX) && errno == ERANGE)
				return MEMINFO_INT_MAX;
			/* It should end with ' kB\n'. */
			if (end[0] != ' ' || end[1] != 'k' || end[2] != 'B')
				return MEMINFO_NO_UNIT;

			/* This should not overflow, but just in case. */
			if (num < (INTMAX_MAX / 1024))
				num *= 1024;

			field->value = num;
			seen_tags |= field->tag;
		}

		if (seen_tags == want_tags)
			break;

		end = strchr(end + 1, '\n');
		if (end == 0)
			break;
		str = end + 1;
	}

	if (seen_tags != want_tags)
		return MEMINFO_NO_INFO;

	*val = meminfo_sum_fields(fields, array_count(fields));
	return MEMINFO_OK;
}

enum meminfo_error_code
meminfo_get_available(uint64_t *val)
{
#ifdef __linux__
	return meminfo_get_available_from_file("/proc/meminfo", val);
#else
	return MEMINFO_NO_FILE;
#endif
}
