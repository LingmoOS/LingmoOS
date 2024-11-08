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


/**
 * @file kyutils.c
 * @brief 
 * @author liuyang <liuyang@kylinos.cn>
 * @version 1.0
 * @date 2021-09-01
 * 
 * @copyright Copyright: 2021,KylinSoft Co.,Ltd.
 * 
 */

#include "kyutils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef __uint8_t		uint8_t;
typedef __uint16_t		uint16_t;
typedef __uint32_t		uint32_t;
typedef __uint64_t		uint64_t;

typedef __intmax_t		intmax_t;
typedef __uintmax_t		uintmax_t;

#define PRIu64			__PRI64_PREFIX "u"

# if __WORDSIZE == 64
#  define __PRI64_PREFIX	"l"
#  define __PRIPTR_PREFIX	"l"
# else
#  define __PRI64_PREFIX	"ll"
#  define __PRIPTR_PREFIX
# endif

enum
{
	SIZE_SUFFIX_1LETTER  = 0,			// 进制单位(保留一个字母，如M）
	SIZE_SUFFIX_3LETTER  = (1 << 0),	// 进制单位(保留三个字母，如MiB)
	SIZE_SUFFIX_SPACE    = (1 << 1),    // 数据与单位间保留空格
	SIZE_DECIMAL_2DIGITS = (1 << 2)		// 保留两位小数
};


int kdkVolumeBaseCharacterConvert(const char* origin_data, KDKVolumeBaseType result_base, char* result_data);
int kdkVolumeBaseNumericalConvert(double origin_numerical, KDKVolumeBaseType origin_base, KDKVolumeBaseType result_base, double* result_numerical);


static int do_scale_by_power (uintmax_t *x, int base, int power)
{
	while (power--) {
		if (UINTMAX_MAX / base < *x)
			return -ERANGE;
		*x *= base;
	}
	return 0;
}

static int get_exp2(uint64_t n, KDKVolumeBaseType base)
{
	int shft;
	int num;

	if((base == KDK_KILO) || (base == KDK_MEGA) ||
			(base == KDK_GIGA) || (base == KDK_TERA)|| 
			(base == KDK_PETA) || (base == KDK_EXA)){
				base -= 6;
			}

	for (shft = 10; shft <= base * 10; shft += 10) {
		if (n < (1ULL << shft))
			break;
	}
	
	return shft - 10;
}

int parse_size(const char *str, uintmax_t *res, int *power)
{
	const char *p;
	char *end;
	uintmax_t x, frac = 0;
	int base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	static const char *suf  = "KMGTPEZY";
	static const char *suf2 = "kmgtpezy";
	const char *sp;

	*res = 0;

	if (!str || !*str) {
		//rc = -EINVAL;
		rc = -KDK_INVAILD_ARGUMENT;
        goto err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	while (isspace((unsigned char) *p))
		p++;
	if (*p == '-') {
		//rc = -EINVAL;
		rc = -KDK_INVAILD_ARGUMENT;
        goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0); //强制类型转换

	if (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -KDK_INVAILD_ARGUMENT;
        goto err;
	}
	if (!end || !*end)
		goto done;			/* without suffix */
	p = end;
	
check_suffix:
	if (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	else if ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1024;			/* XB, 10^N */
		// 消除 ‘iB’和‘B’的区别
	else if (*(p + 1)) {
		struct lconv const *l = localeconv();
		const char *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		if (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const char *fstr = p + dpsz;
			for (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			if (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				if (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					//rc = errno ? -errno : -EINVAL;
					rc = errno ? -errno : -KDK_INVAILD_ARGUMENT;
                    goto err;
				}
			} else
				end = (char *) p;

			if (frac && (!end  || !*end)) {
				rc = -KDK_INVAILD_ARGUMENT;
				goto err;		/* without suffix, but with frac */
			}
			p = end;
			goto check_suffix;
		}
		rc = -KDK_INVAILD_ARGUMENT;
		goto err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	if (sp)
		pwr = (sp - suf) + 1;
	else {
		sp = strchr(suf2, *p);
		if (sp)
			pwr = (sp - suf2) + 1;
		else {
			rc = -EINVAL;
			goto err;
		}
	}
	rc = do_scale_by_power(&x, base, pwr);
	if (power)
		*power = pwr;
	if (frac && pwr) {
		int i;
		uintmax_t frac_div = 10, frac_poz = 1, frac_base = 1;

		/* mega, giga, ... */
		do_scale_by_power(&frac_base, base, pwr);

		/* maximal divisor for last digit (e.g. for 0.05 is
		 * frac_div=100, for 0.054 is frac_div=1000, etc.)
		 */
		while (frac_div < frac)
			frac_div *= 10;

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		for (i = 0; i < frac_zeros; i++)
			frac_div *= 10;

		/*
		 * Go backwardly from last digit and add to result what the
		 * digit represents in the frac_base. For example 0.25G
		 *
		 *  5 means 1GiB / (100/5)
		 *  2 means 1GiB / (10/2)
		 */

		do {
			unsigned int seg = frac % 10;		 /* last digit of the frac */
			uintmax_t seg_div = frac_div / frac_poz; /* what represents the segment 1000, 100, .. */

			frac /= 10;	/* remove last digit from frac */
			frac_poz *= 10;
			if (seg)
				x += frac_base / (seg_div / seg);
		} while (frac);
	}
done:
	*res = x;
err:
	if (rc < 0)
		errno = -rc;
	return rc;
}

char *size_to_human_string(int options, uint64_t bytes, int base)
{
	char buf[32];
	int dec, exp;
	uint64_t frac;
	const char *letters = "BKMGTPE";
	char suffix[sizeof(" KiB")], *psuf = suffix;
	char c;

	if (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';

	exp = get_exp2(bytes, base);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;
	double result = (double)bytes / (1ULL << exp);
	
	*psuf++ = c;

	if ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}
	else if (((base == KDK_KILOBYTE) || (base == KDK_MEGABYTE) ||
			(base == KDK_MEGABYTE) || (base == KDK_GIGABYTE)|| 
			(base == KDK_TERABYTE) || (base == KDK_PETABYTE) ||
			(base == KDK_EXABYTE)) && (c != 'B')) {
				*psuf++ = 'B';
	}

	*psuf = '\0';

	char res[24] = {0};

	if (frac) {
		if (options & SIZE_DECIMAL_2DIGITS) {
			// frac = (frac / (1ULL << (exp - 10)) + 5) / 10;
			// if (frac % 10 == 0)
			// 	frac /= 10;	/* convert N.90 to N.9 */
			sprintf(res, "%.2f", result);
			double num = atof(res);
			double ret = num - dec;
			frac = ret * 100;
			if (frac % 10 == 0)
				sprintf(res, "%.1f", result);
		} else {
			// frac = (frac / (1ULL << (exp - 10)) + 50) / 100;
			// if (frac == 10)
			// 	dec++, frac = 0;
			sprintf(res, "%.1f", result);
		}
	}

	if (frac) {
		struct lconv const *l = localeconv();
		char *dp = l ? l->decimal_point : NULL;

		if (!dp || !*dp)
			dp = ".";
		snprintf(buf, sizeof(buf), "%s%s", res, suffix);
	} else
		snprintf(buf, sizeof(buf), "%d%s", dec, suffix);

	return strdup(buf);
}

int kdkVolumeBaseCharacterConvert(const char* origin_data, KDKVolumeBaseType result_base, char* result_data)
{

    int ret = 0; 
    uintmax_t  basic_data;
    int power;
    // 入参检查
	if (origin_data == NULL)
    {

		syslog(LOG_ERR, "[LINGMOSDK:utils:%s] in parameter error， please check usage！",__FUNCTION__);
        return KDK_INVAILD_ARGUMENT;
    }
	// 出参检查
	if (result_data == NULL)
	{	
		syslog(LOG_ERR, "[LINGMOSDK:utils:%s] out parameter error, please check memory application situation!",__FUNCTION__);
		return KDK_INVAILD_ARGUMENT;
	}

    ret = parse_size(origin_data, &basic_data, &power);
    if (ret != 0)
    {
        syslog(LOG_ERR, "[LINGMOSDK:utils:%s] parse origin data failed, please contact developer, origin data = %s, errcode = %d\n", __FUNCTION__, origin_data ,ret);
        return KDK_INVAILD_DATA_FORMAT;
    }
	// printf("%lu\n", basic_data);
    char *temp;
    temp = size_to_human_string(SIZE_SUFFIX_1LETTER | SIZE_DECIMAL_2DIGITS, basic_data, result_base);
    strcpy(result_data, temp); 
	if (temp != NULL)
		free(temp);
	return ret;
}



int kdkVolumeBaseNumericalConvert(double origin_numerical, KDKVolumeBaseType origin_base, KDKVolumeBaseType result_base, double* result_numerical)
{

	int ret = 0;	
	
	//将数据转化构建成字符串
	char unit[4] = {0};
	switch (origin_base)
	{
	case KDK_KILOBYTE:
		/* code */
		strcpy(unit, "KB");
		break;
	case KDK_MEGABYTE:
		strcpy(unit, "MB");
		break;
	case KDK_GIGABYTE:
		strcpy(unit, "GB");
		break;
 	case KDK_TERABYTE:
	 	strcpy(unit, "TB");
		break;
    case KDK_PETABYTE:
		strcpy(unit, "PB");
		break;
	case KDK_EXABYTE:
		strcpy(unit, "EB");
		break;
	case KDK_KILO:
		strcpy(unit, "K");
		break;
	case KDK_MEGA:
		strcpy(unit, "M");
		break;
	case KDK_GIGA:
		strcpy(unit, "G");
		break;
	case KDK_TERA:
		strcpy(unit, "T");
		break;
	case KDK_PETA:
		strcpy(unit, "P");
		break;
	case KDK_EXA:
		strcpy(unit, "E");
		break;
	default:
		strcpy(unit, "B");
		break;
	}
	char origin_string[200] = {0};
	sprintf(origin_string, "%.2f%s",origin_numerical, unit);
	
	// 后续重复解析过程
	uintmax_t  basic_data;
    int power;
    ret = parse_size(origin_string, &basic_data, &power);
    if (ret != 0)
    {
        syslog(LOG_ERR, "[LINGMOSDK:utils:%s] parse origin data failed, please contact developer, origin data = %s, errcode = %d\n", __FUNCTION__, origin_string ,ret);
        return KDK_INVAILD_DATA_FORMAT;
    }

    char *temp;
    temp = size_to_human_string(SIZE_DECIMAL_2DIGITS, basic_data, result_base);
   
	*result_numerical = atof(temp);

	if(temp != NULL)
		free(temp);
	return ret;
}
