/*
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


#include "klog_dump.h"
#include "libkyconf.h"
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/etc", strlen("/etc"));
}

int klog_rotate_init(int cid, const char *name, const char *rootpath)
{
	const char *ruler = kdk_conf_get_value(cid, "DUMP", "rules");
	if (strcasecmp(ruler, "none") == 0)
		return 0;
	const char *oversize = kdk_conf_get_value(cid, "DUMP", "size");
	int compress = atoi(kdk_conf_get_value(cid, "DUMP", "compress"));

	char tmp[1025];
	snprintf(tmp, 1024, "/etc/lingmosdk/lingmosdk-base/logrotate.d/%s", name);
	char canonical_filename[PATH_MAX] = "\0";
	memset(canonical_filename,0,PATH_MAX);
	realpath(tmp, canonical_filename);
	if(!verify_file(canonical_filename))
	{
		return -1;
	}
	FILE *fp = fopen(canonical_filename, "wt+");
	if (!fp)
		return -1;
	
	snprintf(tmp, 1024, "%s/%s* {\n", rootpath, name);
	fputs(tmp, fp);
	if (strcasecmp(ruler, "daily") == 0)
		fputs("\tdaily\n", fp);
	else if (strcasecmp(ruler, "weekly") == 0)
		fputs("\tweekly\n", fp);
	else
		fputs("\tmonthly\n", fp);
	
	fputs("\trotate 7\n", fp);
	fputs("\tnotifempty\n", fp);
	fputs("\tnocopytruncate\n", fp);
	if (compress)
		fputs("\tcompress\n", fp);
	else
		fputs("\tnocompress\n", fp);
	
	if (strcasecmp(ruler, "size") == 0)
	{
		int size = atoi(oversize);
		snprintf(tmp, 1024, "\tsize %d", size);
		char *p = oversize;
		while (*p && isdigit(p))
			p ++;
		if (strncasecmp(p, "M", 1) == 0)
			strcat(tmp, "M");
		else if (strncasecmp(p, "K", 1) == 0)
			strcat(tmp, "k");
		else if (strncasecmp(p, "G", 1) == 0)
			strcat(tmp, "G");
		strcat(tmp, "\n");
		fputs(tmp, fp);
	}
	fputs("}", fp);
	fclose(fp);
	return 0;
}
