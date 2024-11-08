/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#include<stdio.h>
#include "lingmo-print.h"
#include <stdarg.h>
#include<stdlib.h>

int vprint_by_level(int print_level, FILE * __restrict stream,
						char * __restrict format, va_list args)
{
	int ret = 0;
	int level_int = 0;

    level_int = PRINT_LEVEL_DEFAULT;


	if (print_level <= level_int)
	{

        switch (print_level)
        {
           case PRINT_LEVEL_EMERG:
           case PRINT_LEVEL_ALERT:
           case PRINT_LEVEL_CRIT:
           case PRINT_LEVEL_ERROR:
                fprintf(stderr, "%s", PRINT_COLOR_RED);
                break;
           case PRINT_LEVEL_WARN:
                fprintf(stdout, "%s", PRINT_COLOR_PURPLE);
                break;
           case PRINT_LEVEL_NOTICE:
                fprintf(stdout, "%s", PRINT_COLOR_BLUE);
                break;
           case PRINT_LEVEL_INFO:
                fprintf(stdout, "%s", PRINT_COLOR_CYAN);
                break;
           case PRINT_LEVEL_DEBUG:
                fprintf(stdout, "%s", PRINT_COLOR_YELLOW);
                break;
           default:
               fprintf(stdout, "%s", PRINT_COLOR_WHITE);
        }


		switch (print_level)
		{
        case PRINT_LEVEL_EMERG:
			fprintf(stderr, "[EMERG] ");
			break;
        case PRINT_LEVEL_ALERT:
			fprintf(stderr, "[ALERT] ");
			break;
        case PRINT_LEVEL_CRIT:
            fprintf(stderr, "[CRIT] ");
			break;
        case PRINT_LEVEL_ERROR:
			fprintf(stderr, "[ERROR] ");
			break;
        case PRINT_LEVEL_WARN:
            fprintf(stdout, "[WARN] ");
			break;
        case PRINT_LEVEL_NOTICE:
            fprintf(stdout, "[NOTE] ");
			break;
        case PRINT_LEVEL_INFO:
            fprintf(stdout, "[INFO] ");
			break;
        case PRINT_LEVEL_DEBUG:
			fprintf(stdout, "[DEBUG] ");
			break;
		default:
			fprintf(stdout, "[UKNOW] ");
		}

		ret = vfprintf(stream, format, args);

        if (print_level <= PRINT_LEVEL_ERROR && print_level >= 0)
            fprintf(stderr, "%s", PRINT_COLOR_END);
        else
            fprintf(stdout, "%s", PRINT_COLOR_END);

	}

	return ret;
}

int print_by_level(int print_level, FILE * stream, char * format, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, format);
    ret = vprint_by_level(print_level, stream, format, args);
	va_end(args);

	return ret;
}

int print_debug(char * format, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, format);
    ret = vprint_by_level(PRINT_LEVEL_DEBUG, stdout, format, args);
	va_end(args);

	return ret;
}

int print_info(char * format, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, format);
    ret = vprint_by_level(PRINT_LEVEL_INFO, stdout, format, args);
	va_end(args);

	return ret;
}

int print_notice(char * format, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, format);
    ret = vprint_by_level(PRINT_LEVEL_NOTICE, stdout, format, args);
	va_end(args);

	return ret;
}

int print_warning(char * format, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, format);
    ret = vprint_by_level(PRINT_LEVEL_WARN, stdout, format, args);
	va_end(args);

	return ret;
}

int print_error(char * format, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, format);
    ret = vprint_by_level(PRINT_LEVEL_ERROR, stderr, format, args);
	va_end(args);

	return ret;
}
