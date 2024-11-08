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

#include "libkycpu.h"
#include <cstring-extension.h>
#include <errno.h>
#include <libkylog.h>
#include <lingmosdk/lingmosdk-base/sdkmarcos.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include "sys/sysinfo.h"
#include <locale.h>
#include <libintl.h>
#include "unistd.h"
#ifdef __linux__
#include <sys/utsname.h>
#endif
#define MIDSIZE 128

#define LOCALEDIR "/usr/share/locale/"
#define GETTEXT_PACKAGE "lingmosdk-hardware"

#ifndef max
# define max(x, y) __extension__ ({		\
	__typeof__(x) _max1 = (x);		\
	__typeof__(y) _max2 = (y);		\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
#endif

#ifndef min
# define min(x, y) __extension__ ({		\
	__typeof__(x) _min1 = (x);		\
	__typeof__(y) _min2 = (y);		\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif

struct _cpuInfo{
    size_t ncpus;               // CPU数量
    size_t corenums;            // CPU核心数量
    size_t wordorder;              // 字节序
    size_t cacheLevel;             // 缓存级别数
    size_t cpu_process;          // cpu线程数
    char *arch;                 // 架构
    char *vendor;               // 生产商
    char *model;                // 型号
    char *cur_freq_MHz;         // 主频
    char *min_freq_MHz;         // 最小主频
    char *max_freq_MHz;         // 最大主频
    char *flags;                // CPU 特性
    char *virt;                 // 虚拟化支持
};

struct _cpuInfo *cpuinf;

struct id_part {
    const int id;
    const char* name;
};

static const struct id_part arm_part[] = {
    { 0x810, "ARM810" },
    { 0x920, "ARM920" },
    { 0x922, "ARM922" },
    { 0x926, "ARM926" },
    { 0x940, "ARM940" },
    { 0x946, "ARM946" },
    { 0x966, "ARM966" },
    { 0xa20, "ARM1020" },
    { 0xa22, "ARM1022" },
    { 0xa26, "ARM1026" },
    { 0xb02, "ARM11 MPCore" },
    { 0xb36, "ARM1136" },
    { 0xb56, "ARM1156" },
    { 0xb76, "ARM1176" },
    { 0xc05, "Cortex-A5" },
    { 0xc07, "Cortex-A7" },
    { 0xc08, "Cortex-A8" },
    { 0xc09, "Cortex-A9" },
    { 0xc0d, "Cortex-A17" },	/* Originally A12 */
    { 0xc0f, "Cortex-A15" },
    { 0xc0e, "Cortex-A17" },
    { 0xc14, "Cortex-R4" },
    { 0xc15, "Cortex-R5" },
    { 0xc17, "Cortex-R7" },
    { 0xc18, "Cortex-R8" },
    { 0xc20, "Cortex-M0" },
    { 0xc21, "Cortex-M1" },
    { 0xc23, "Cortex-M3" },
    { 0xc24, "Cortex-M4" },
    { 0xc27, "Cortex-M7" },
    { 0xc60, "Cortex-M0+" },
    { 0xd01, "Cortex-A32" },
    { 0xd03, "Cortex-A53" },
    { 0xd04, "Cortex-A35" },
    { 0xd05, "Cortex-A55" },
    { 0xd06, "Cortex-A65" },
    { 0xd07, "Cortex-A57" },
    { 0xd08, "Cortex-A72" },
    { 0xd09, "Cortex-A73" },
    { 0xd0a, "Cortex-A75" },
    { 0xd0b, "Cortex-A76" },
    { 0xd0c, "Neoverse-N1" },
    { 0xd0d, "Cortex-A77" },
    { 0xd0e, "Cortex-A76AE" },
    { 0xd13, "Cortex-R52" },
    { 0xd20, "Cortex-M23" },
    { 0xd21, "Cortex-M33" },
    { 0xd40, "Neoverse-V1" },
    { 0xd41, "Cortex-A78" },
    { 0xd42, "Cortex-A78AE" },
    { 0xd44, "Cortex-X1" },
    { 0xd46, "Cortex-510" },
    { 0xd47, "Cortex-710" },
    { 0xd48, "Cortex-X2" },
    { 0xd49, "Neoverse-N2" },
    { 0xd4a, "Neoverse-E1" },
    { 0xd4b, "Cortex-A78C" },
    { -1, "unknown" },
};

static const struct id_part brcm_part[] = {
    { 0x0f, "Brahma B15" },
    { 0x100, "Brahma B53" },
    { 0x516, "ThunderX2" },
    { -1, "unknown" },
};

static const struct id_part dec_part[] = {
    { 0xa10, "SA110" },
    { 0xa11, "SA1100" },
    { -1, "unknown" },
};

static const struct id_part cavium_part[] = {
    { 0x0a0, "ThunderX" },
    { 0x0a1, "ThunderX 88XX" },
    { 0x0a2, "ThunderX 81XX" },
    { 0x0a3, "ThunderX 83XX" },
    { 0x0af, "ThunderX2 99xx" },
    { -1, "unknown" },
};

static const struct id_part apm_part[] = {
    { 0x000, "X-Gene" },
    { -1, "unknown" },
};

static const struct id_part qcom_part[] = {
    { 0x00f, "Scorpion" },
    { 0x02d, "Scorpion" },
    { 0x04d, "Krait" },
    { 0x06f, "Krait" },
    { 0x201, "Kryo" },
    { 0x205, "Kryo" },
    { 0x211, "Kryo" },
    { 0x800, "Falkor V1/Kryo" },
    { 0x801, "Kryo V2" },
    { 0x803, "Kryo 3XX Silver" },
    { 0x804, "Kryo 4XX Gold" },
    { 0x805, "Kryo 4XX Silver" },
    { 0xc00, "Falkor" },
    { 0xc01, "Saphira" },
    { -1, "unknown" },
};

static const struct id_part samsung_part[] = {
    { 0x001, "exynos-m1" },
    { -1, "unknown" },
};

static const struct id_part nvidia_part[] = {
    { 0x000, "Denver" },
    { 0x003, "Denver 2" },
    { 0x004, "Carmel" },
    { -1, "unknown" },
};

static const struct id_part marvell_part[] = {
    { 0x131, "Feroceon 88FR131" },
    { 0x581, "PJ4/PJ4b" },
    { 0x584, "PJ4B-MP" },
    { -1, "unknown" },
};

static const struct id_part apple_part[] = {
    { 0x022, "Icestorm" },
    { 0x023, "Firestorm" },
    { -1, "unknown" },
};

static const struct id_part faraday_part[] = {
    { 0x526, "FA526" },
    { 0x626, "FA626" },
    { -1, "unknown" },
};

static const struct id_part intel_part[] = {
    { 0x200, "i80200" },
    { 0x210, "PXA250A" },
    { 0x212, "PXA210A" },
    { 0x242, "i80321-400" },
    { 0x243, "i80321-600" },
    { 0x290, "PXA250B/PXA26x" },
    { 0x292, "PXA210B" },
    { 0x2c2, "i80321-400-B0" },
    { 0x2c3, "i80321-600-B0" },
    { 0x2d0, "PXA250C/PXA255/PXA26x" },
    { 0x2d2, "PXA210C" },
    { 0x411, "PXA27x" },
    { 0x41c, "IPX425-533" },
    { 0x41d, "IPX425-400" },
    { 0x41f, "IPX425-266" },
    { 0x682, "PXA32x" },
    { 0x683, "PXA930/PXA935" },
    { 0x688, "PXA30x" },
    { 0x689, "PXA31x" },
    { 0xb11, "SA1110" },
    { 0xc12, "IPX1200" },
    { -1, "unknown" },
};

static const struct id_part fujitsu_part[] = {
    { 0x001, "A64FX" },
    { -1, "unknown" },
};

static const struct id_part hisi_part[] = {
    { 0xd01, "Kunpeng-920" },	/* aka tsv110 */
    { -1, "unknown" },
};

static const struct id_part ft_part[] = {
    { 0x660, "FTC660" },
    { 0x661, "FTC661" },
    { 0x662, "FTC662" },
    { 0x663, "FTC663" },
    { -1, "unknown" },
};

static const struct id_part unknown_part[] = {
    { -1, "unknown" },
};

struct hw_impl {
   const int    id;
   const struct id_part     *parts;
   const char   *name;
};

static const struct hw_impl hw_implementer[] = {
    { 0x41, arm_part,     "ARM" },
    { 0x42, brcm_part,    "Broadcom" },
    { 0x43, cavium_part,  "Cavium" },
    { 0x44, dec_part,     "DEC" },
    { 0x46, fujitsu_part, "FUJITSU" },
    { 0x48, hisi_part,    "HiSilicon" },
    { 0x49, unknown_part, "Infineon" },
    { 0x4d, unknown_part, "Motorola/Freescale" },
    { 0x4e, nvidia_part,  "NVIDIA" },
    { 0x50, apm_part,     "APM" },
    { 0x51, qcom_part,    "Qualcomm" },
    { 0x53, samsung_part, "Samsung" },
    { 0x56, marvell_part, "Marvell" },
    { 0x61, apple_part,   "Apple" },
    { 0x66, faraday_part, "Faraday" },
    { 0x69, intel_part,   "Intel" },
    { 0x70, ft_part,      "Phytium" },
    { 0xc0, unknown_part, "Ampere" },
    { -1,   unknown_part, "unknown" },
};

static void _free_cpuinfo()
{
    if (cpuinf)
    {
        SAFE_FREE(cpuinf->arch);
        SAFE_FREE(cpuinf->vendor);
        SAFE_FREE(cpuinf->model);
        SAFE_FREE(cpuinf->cur_freq_MHz);
        SAFE_FREE(cpuinf->max_freq_MHz);
        SAFE_FREE(cpuinf->min_freq_MHz);

        SAFE_FREE(cpuinf);
    }
}

static int lookup(char *line, char *pattern, char **value)
{
	char *p, *v;
	int len = strlen(pattern);
    // klog_debug("%s\n",pattern);
	/* don't re-fill already found tags, first one wins */
    if (strcmp(pattern,"processor")){
        if (!*line || *value)
		    return 0;
    }

	/* pattern */
	if (strncmp(line, pattern, len))
		return 0;

	/* white spaces */
	for (p = line + len; isspace(*p); p++);

	/* separator */
	if (*p != ':')
		return 0;

	/* white spaces */
	for (++p; isspace(*p); p++);

	/* value */
	if (!*p)
		return 0;
	v = p;

	/* end of value */
	len = strlen(line) - 1;
	for (p = line + len; isspace(*(p-1)); p--);
	*p = '\0';

	*value = strdup(v);
	return 1;
}

static void _get_cpu_info()
{
    // 我知道这里有竞态问题，但是不想引入pthread，所以算了
    if (!cpuinf)
    {
        cpuinf = (struct _cpuInfo *)calloc(1, sizeof(struct _cpuInfo));
        if (!cpuinf)
        {
            klog_err("内存分配失败：%s\n", strerror(errno));
            return ;
        }
    }
#ifdef __linux__
    struct utsname utsbuf;
    if (uname(&utsbuf) == -1)
    {
        klog_err("uname 失败：%s\n", strerror(errno));
        SAFE_FREE(cpuinf);
        return ;
    }

    cpuinf->arch = strdup(utsbuf.machine);

    FILE *fp = fopen("/proc/cpuinfo", "rt");
    if (!fp)
    {
        klog_err("/proc/cpuinfo 读取失败：%s\n", strerror(errno));
        SAFE_FREE(cpuinf);
        return ;
    }

#define CPUINF_BUFSIZE  8192
    char buffer[CPUINF_BUFSIZE];
    char *corenums;
    char *cpu_process;
    int cur_freq = 0;
    while (fgets(buffer, CPUINF_BUFSIZE, fp))
    {
        // klog_debug("%s",buffer);
        // if (strlen(strskipspace(buffer)) == 0)  // only '\n' left
        //     break;
        if (lookup(buffer, "vendor", &cpuinf->vendor));
        else if (lookup(buffer, "vendor_id", &cpuinf->vendor));
        else if (lookup(buffer, "model name", &cpuinf->model));
        else if (lookup(buffer, "flags", &cpuinf->flags));      // x86
        else if (lookup(buffer, "features", &cpuinf->flags));   // s390
        else if (lookup(buffer, "Loongson Features", &cpuinf->flags));
        else if (lookup(buffer, "Features", &cpuinf->flags));   // aarch64
        else if (lookup(buffer, "type", &cpuinf->flags));       // sparc64
        else if (lookup(buffer, "cpu MHz", &cpuinf->cur_freq_MHz));
        else if (lookup(buffer, "CPU MHz", &cpuinf->cur_freq_MHz));
        else if (lookup(buffer, "processor", &cpu_process))
        {
            cpuinf->cpu_process = atoi(cpu_process) + 1;
            free(cpu_process);
        }
        else if (lookup(buffer, "cpu cores", &corenums))
        {
            cpuinf->corenums = atoi(corenums);
            free(corenums);
        }
    }
    fclose(fp);

    if(cpuinf->model)
    {
        if(strstr(cpuinf->model, "Loongson"))
        {
            cpuinf->vendor = strdup("loongson");
        }
    }
    

    if (cpuinf->flags)
    {
        if (strstr(cpuinf->flags, " svm "))
            cpuinf->virt = strdup("svm");
        else if (strstr(cpuinf->flags, " vmx "))
            cpuinf->virt = strdup("vmx");
    }
    
    if(cpuinf->vendor == NULL)
    {
        char ret[MIDSIZE];
        int num = 0;
        int part, j;
	    const struct id_part *parts = NULL;
        fp = fopen("/sys/devices/system/cpu/cpu0/regs/identification/midr_el1", "rt");
        if (!fp)
        {
            klog_err("midr_el1 读取失败：%s\n", strerror(errno));
            SAFE_FREE(cpuinf);
            return ;
        }
        fgets(buffer, CPUINF_BUFSIZE, fp);
        char *substr = "x";
        char *date = strstr(buffer, substr);
        strcpy(buffer, date + 9);
        sscanf(buffer,"%2x",&num) ;

        for (j = 0; hw_implementer[j].id != -1; j++) {
            if (hw_implementer[j].id == num) {
                parts = hw_implementer[j].parts;
                cpuinf->vendor = strdup(hw_implementer[j].name);
                break;
            }
	    }
        fclose(fp);
    }

    if(cpuinf->model == NULL)
    {
        fp = fopen("/proc/cpuinfo", "rt");
        if (!fp)
        {
            klog_err("/proc/cpuinfo 读取失败：%s\n", strerror(errno));
            SAFE_FREE(cpuinf);
            return ;
        }
        while(fgets(buffer, CPUINF_BUFSIZE, fp))
        {
            if (lookup(buffer, "Hardware", &cpuinf->model));//huawei 9A0
        }
        fclose(fp);
    }

    if(cpuinf->corenums == 0)
    {
        cpuinf->corenums = sysconf(_SC_NPROCESSORS_ONLN);
    }
    
    if(cpuinf->cur_freq_MHz == NULL)
    {
        char ret[MIDSIZE] = {0};
        int part = 0;
        char path[128] = {0};
        int i = 0;
        char buffer[64] = {0};
        for(i = 0; i < cpuinf->cpu_process; i++)
        {
            memset(path, 0, 128);
            sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_cur_freq", i);
            fp = fopen(path, "rt");
            if (!fp)
            {
                klog_err("cpuinfo_max_freq 读取失败：%s\n", strerror(errno));
                return ;
            }
            fgets(ret, MIDSIZE, fp);
            part = atoi(ret);
            part = part / 1000;
            cur_freq += part;
            fclose(fp);
        }
        sprintf(buffer, "%ld", cur_freq / cpuinf->cpu_process);
        cpuinf->cur_freq_MHz = strdup(buffer);
    }

#endif
    return;
}

#define GET_VALUE(name, default_val)    do{\
                                        if (! cpuinf)\
                                            _get_cpu_info();\
                                        return cpuinf ? cpuinf->name : default_val;\
                                        }while(0)

const char* kdk_cpu_get_arch()
{
    GET_VALUE(arch, NULL);
}

const char* kdk_cpu_get_vendor()
{
    GET_VALUE(vendor, NULL);
}

const char* kdk_cpu_get_model()
{
    GET_VALUE(model, NULL);
}

const char* kdk_cpu_get_freq_MHz()
{
    GET_VALUE(cur_freq_MHz, NULL);
}

unsigned int kdk_cpu_get_corenums()
{
    GET_VALUE(corenums, 0);
}

const char* kdk_cpu_get_virt()
{
    GET_VALUE(virt, NULL);
}

unsigned int kdk_cpu_get_process()
{
    GET_VALUE(cpu_process, 0);
}

float kdk_cpu_get_max_freq_MHz()
{
    char ret[MIDSIZE] = {0};
    int num = 0;
    int part = 0;
    char path[128] = {0};
    FILE *fp = NULL;
#ifdef __linux__
    int process = kdk_cpu_get_process();
    int i = 0;
    for(i = 0; i < process; i++)
    {
        memset(path, 0, 128);
        sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
        fp = fopen(path, "rt");
        if (!fp)
        {
            klog_err("cpuinfo_max_freq 读取失败：%s\n", strerror(errno));
            return -1;
        }
        fgets(ret, MIDSIZE, fp);
        part = atoi(ret);
        part = part / 1000;
        num = max(num, part);
        fclose(fp);
    }  
#endif
    return num;  
}

float kdk_cpu_get_min_freq_MHz()
{
    char ret[MIDSIZE] = {0};
    int num = kdk_cpu_get_max_freq_MHz();
    int part = 0;
    char path[128] = {0};
    FILE *fp = NULL;
#ifdef __linux__
    int process = kdk_cpu_get_process();
    int i = 0;
    for(i = 0; i < process; i++)
    {
        memset(path, 0, 128);
        sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", i);
        fp = fopen(path, "rt");
        if (!fp)
        {
            klog_err("cpuinfo_min_freq 读取失败：%s\n", strerror(errno));
            return -1;
        }
        fgets(ret, MIDSIZE, fp);
        part = atoi(ret);
        part = part / 1000;
        num = min(num, part);
        fclose(fp);
    }  
#endif
    return num; 
}

char* kdk_cpu_get_running_time()
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    FILE *fp = NULL;
    int time_sec = 0;
    char *runtime = (char *)malloc(sizeof(char) * 128);
    if(!runtime)
        return NULL;
    memset(runtime, 0, 128);

    if ((fp = fopen("/proc/uptime", "r"))) {

        char buf[128] = {0};
        long up_sec, up_usec;
        if (fgets(buf, sizeof(buf), fp)) {
            int nm = sscanf(buf, "%ld.%ld", &up_sec, &up_usec);
            if (nm == 2) {
                time_sec = up_sec;
                long days = time_sec / 86400;
                long hours = (time_sec - days * 86400) / 3600;
                long mins = (time_sec - days * 86400 - hours * 3600) / 60;
                char *lang = getenv("LANG");
                if(strstr(lang, "en_US"))
                {
                    sprintf(runtime, "%lddays%ldhours%ldminutes", days, hours, mins);
                }
                else{
                    sprintf(runtime, gettext("%ldDD%ldHH%ldMM"), days, hours, mins);
                }
            }
        }
        fclose(fp);
    }
    return runtime;
}

unsigned int kdk_cpu_get_sockets()
{
    int sockets = 0;
    char buffer[64] = {0};
    memset(buffer, 0, 64);
    char buf[2048] = {0};
    memset(buf, 0, 2048);
    int i = 0;
    FILE *pipeLine = popen("lscpu", "r");
    if (!pipeLine)
    {
        return -1;
    }
    if (__glibc_likely(pipeLine != NULL))
    {
        while (fgets(buf, sizeof(buf), pipeLine))
        {
            if(strstr(buf, "座："))
            {
                sscanf(buf, "%*s %s", buffer);
		        strstripspace(buffer);
                sockets = atoi(buffer);
            }
            else if(strstr(buf, "socket(s)"))
            {
                sscanf(buf, "%*s %s", buffer);
		        strstripspace(buffer);
                sockets = atoi(buffer);
            }
        }
    }
    pclose(pipeLine);
    return sockets;
}

unsigned int kdk_cpu_get_L1d_cache()
{
#ifdef __linux__
    char res_buffer[64] = {0};
    char buf[1024] = {0};
    FILE *fp = NULL;
    int cache_nums = 0;

    fp = popen("lscpu", "r");
    if(!fp)
    {
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp))
    {
        if(strstr(buf, "L1d cache"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }
        }
        else if(strstr(buf, "L1d 缓存"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }

        }
    }
    pclose(fp);

    if(cache_nums == 0)
    {
        int i = 0;
        FILE *pipeLine = popen("dmidecode -t cache", "r");
        if (pipeLine != NULL)
        {
            memset(buf, 0, 1024);
            memset(res_buffer, 0, 64);
            while (fgets(buf, sizeof(buf), pipeLine))
            {
                if(strstr(buf, "L1 Data Cache"))
                {
                    i = 1;
                }

                if(i != 1)
                {
                    continue;
                }
                
                if(strstr(buf, "Installed Size"))
                {
                    strstripspace(buf);
                    sscanf(buf, "Installed Size: %s %*s", res_buffer);
                    cache_nums = atoi(res_buffer);
                    break;
                }
            }
            pclose(pipeLine);
        }
    }
#endif
    return cache_nums;
}

unsigned int kdk_cpu_get_L1i_cache()
{
#ifdef __linux__
    char res_buffer[64] = {0};
    char buf[1024] = {0};
    FILE *fp = NULL;
    int cache_nums = 0;

    fp = popen("lscpu", "r");
    if(!fp)
    {
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp))
    {
        if(strstr(buf, "L1i cache"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }
        }
        else if(strstr(buf, "L1i 缓存"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }
        }
    }
    pclose(fp);

    if(cache_nums == 0)
    {
        int i = 0;
        FILE *pipeLine = popen("dmidecode -t cache", "r");
        if (pipeLine != NULL)
        {
            memset(buf, 0, 1024);
            memset(res_buffer, 0, 64);
            while (fgets(buf, sizeof(buf), pipeLine))
            {
                if(strstr(buf, "L1 Instruction Cache"))
                {
                    i = 1;
                }

                if(i != 1)
                {
                    continue;
                }
                
                if(strstr(buf, "Installed Size"))
                {
                    strstripspace(buf);
                    sscanf(buf, "Installed Size: %s %*s", res_buffer);
                    cache_nums = atoi(res_buffer);
                    break;
                }
            }
            pclose(pipeLine);
        }
    }
#endif
    return cache_nums;
}

unsigned int kdk_cpu_get_L2_cache()
{
#ifdef __linux__
    char res_buffer[64] = {0};
    char buf[1024] = {0};
    FILE *fp = NULL;
    int cache_nums = 0;
    
    fp = popen("lscpu", "r");
    if(!fp)
    {
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp))
    {
        if(strstr(buf, "L2 cache"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }
        }
        else if(strstr(buf, "L2 缓存"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }
        }
    }
    pclose(fp);

    if(cache_nums == 0)
    {
        int i = 0;
        FILE *pipeLine = popen("dmidecode -t cache", "r");
        if (pipeLine != NULL)
        {
            memset(buf, 0, 1024);
            memset(res_buffer, 0, 64);
            while (fgets(buf, sizeof(buf), pipeLine))
            {
                if(strstr(buf, "L2 Cache") || strstr(buf, "L2 - Cache"))
                {
                    i = 1;
                }

                if(i != 1)
                {
                    continue;
                }
                
                if(strstr(buf, "Installed Size"))
                {
                    strstripspace(buf);
                    sscanf(buf, "Installed Size: %s %*s", res_buffer);
                    cache_nums = atoi(res_buffer);
                    break;
                }
            }
            pclose(pipeLine);
        }
    }
#endif
    return cache_nums;
}

unsigned int kdk_cpu_get_L3_cache()
{
#ifdef __linux__
    char res_buffer[64] = {0};
    char buf[1024] = {0};
    FILE *fp = NULL;
    int cache_nums = 0;
    
    fp = popen("lscpu", "r");
    if(!fp)
    {
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp))
    {
        if(strstr(buf, "L3 cache"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }
        }
        else if(strstr(buf, "L3 缓存"))
        {
            sscanf(buf, "%*s %*s %s", res_buffer);
            strstrip(res_buffer, 'K');
            cache_nums = atoi(res_buffer);
            if(strstr(buf, "MiB"))
            {
                cache_nums = cache_nums * 1024;
            }
        }
    }
    pclose(fp);
    
    if(cache_nums == 0)
    {
        int i = 0;
        FILE *pipeLine = popen("dmidecode -t cache", "r");
        if (pipeLine != NULL)
        {
            memset(buf, 0, 1024);
            memset(res_buffer, 0, 64);
            while (fgets(buf, sizeof(buf), pipeLine))
            {
                if(strstr(buf, "L3 Cache") || strstr(buf, "L3 - Cache"))
                {
                    i = 1;
                }

                if(i != 1)
                {
                    continue;
                }
                
                if(strstr(buf, "Installed Size"))
                {
                    strstripspace(buf);
                    sscanf(buf, "Installed Size: %s %*s", res_buffer);
                    cache_nums = atoi(res_buffer);
                    break;
                }
            }
            pclose(pipeLine);
        }
    }
#endif
    return cache_nums;
}
