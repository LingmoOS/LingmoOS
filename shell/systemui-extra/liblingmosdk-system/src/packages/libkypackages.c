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

#define LIBDPKG_VOLATILE_API
#include "libkypackages.h"
#include <libkylog.h>
#include <cstring-extension.h>
#include <libgen.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#define KDK_DEKSTOP_FILE_PATH "/usr/share/applications/"
#define KDK_LOCALE_DEKSTOP_FILE_PATH "/.local/share/applications/"
#define KDK_SNAPD_DEKSTOP_FILE_PATH "/var/lib/snapd/desktop/applications/"
#define KDK_FLATPAK_DEKSTOP_FILE_PATH "/var/lib/flatpak/exports/share/applications/"
#define PROGRAM_SIZE 1024

#ifdef __linux__
#define DPKG_DB "/var/lib/dpkg/status"
#endif

#define PACKAGE_INCREASE_STEP 50

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strlen("/"));
}

static inline char *lookup(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strlen(key) + 1;
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static inline char *read_key(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strlen(key);
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static int get_desktop_info(char *dir_path, kdk_startmenu_list *programlist)
{
    char canonical_filename[4096] = {"\0"};
    if (!realpath(dir_path, canonical_filename) || !verify_file(canonical_filename))
    {
        return 0;
    }
    DIR *dir = opendir(canonical_filename);
    struct dirent *dir_t = NULL;
    if (dir != NULL)
    {
        while ((dir_t = readdir(dir)) != NULL)
        {
            if ((strcmp(dir_t->d_name, ".") == 0) || (strcmp(dir_t->d_name, "..") == 0))
                continue;
            if (DT_DIR == dir_t->d_type)
                continue;

            kdk_startmenu_t *curinfo = NULL;
            if (strstr(dir_t->d_name, ".desktop"))
            {
                char fpath[PROGRAM_SIZE] = {0};
                sprintf(fpath, "%s/%s", canonical_filename, dir_t->d_name);

                int is_start_menu_app = 1;
                char name[128] = {"None"};
                char version[128] = {"None"};
                char company[128] = {"None"};
                char cmd[128] = {"None"};
                char param[128] = {"None"};
                char icon[128] = {"None"};
                char buffer[PROGRAM_SIZE] = {0};

                char filename[4096] = {"\0"};
                if (!realpath(fpath, filename) || !verify_file(filename))
                {
                    return 0;
                }

                

                FILE *fp = fopen(filename, "r");
                if (NULL == fp)
                {
                    return 0;
                }

                while (fgets(buffer, PROGRAM_SIZE, fp))
                {
                    char *val = NULL;
                    if ((val = read_key(buffer, "Name=")))
                    {
                        strcpy(name, val);
                    }

                    if ((val = read_key(buffer, "Version=")))
                    {
                        strcpy(version, val);
                    }

                    if ((val = read_key(buffer, "Company=")))
                    {
                        strcpy(company, val);
                    }

                    if ((val = read_key(buffer, "Exec=")))
                    {
                        char *p = val;
                        char tmp[128] = "\0";
                        strcpy(tmp, val);
                        char *token = strsep(&p, " ");
                        if (strcmp(token, "sudo") == 0)
                        {
                            strcpy(cmd, tmp);
                        }
                        else
                        {
                            strcpy(cmd, token);
                            if (p != NULL)
                                strcpy(param, p);
                        }
                    }

                    if ((val = read_key(buffer, "Icon=")))
                    {
                        strcpy(icon, val);
                    }

                    // 判断是否是显示在开始菜单的应用程序
                    if ((val = read_key(buffer, "NoDisplay=")))
                    {
                        if (0 == strcmp(val, "true"))
                        {
                            is_start_menu_app = 0;
                        }
                    }
                }
                fclose(fp);

                if (is_start_menu_app)
                {
                    kdk_startmenu_t **tmp = (kdk_startmenu_t **)realloc(programlist->list, PROGRAM_SIZE * sizeof(kdk_startmenu_t *));
                    if (!tmp)
                        return 0;

                    curinfo = (kdk_startmenu_t *)calloc(1, sizeof(kdk_startmenu_t));
                    if (!curinfo)
                        return 0;

                    programlist->list = tmp;
                    programlist->list[programlist->nums] = curinfo;
                    programlist->nums++;

                    curinfo->name = strdup(name);
                    curinfo->version = strdup(version);
                    curinfo->company = strdup(company);
                    curinfo->cmd = strdup(cmd);
                    curinfo->param = strdup(param);
                    curinfo->icon = strdup(icon);
                }
            }
        }
        closedir(dir);
    }
    return 1;
}

static char *read_default_app(const char *filetype)
{
    char *homeDir = getenv("HOME");
    char filepath[PROGRAM_SIZE] = {0};
    sprintf(filepath, "%s/.config/mimeapps.list", homeDir);
    FILE *fp = fopen(filepath, "r");
    char line[1024], *val = NULL;
    if (NULL != fp)
    {
        while (fgets(line, 1024, fp))
        {
            if ((val = lookup(line, filetype)))
                break;
        }
        fclose(fp);
    }

    if (NULL == val)
    {
        fp = fopen("/usr/share/applications/lingmo-mimeapps.list", "r");
        if (NULL != fp)
        {
            while (fgets(line, 1024, fp))
            {
                if ((val = lookup(line, filetype)))
                    break;
            }
            fclose(fp);
        }
    }
    if (NULL == val)
    {
        fp = fopen("/usr/share/applications/mimeinfo.cache", "r");
        if (NULL != fp)
        {
            while (fgets(line, 1024, fp))
            {
                if ((val = lookup(line, filetype)))
                    break;
            }
            fclose(fp);
        }
    }

    if (NULL == val)
        return strdup("None");

    strstrip(val, ';');
    char desktop_file[1024];
    sprintf(desktop_file, "/usr/share/applications/%s", val);

    char canonical_filename[4096] = {"\0"};
    if (!realpath(desktop_file, canonical_filename) || !verify_file(canonical_filename))
    {
        return NULL;
    }

    fp = fopen(canonical_filename, "r");
    if(NULL == fp)
        return NULL;

    while (fgets(line, 1024, fp))
    {
        val = line;
        if (strstartswith(val, "Name=") == 0)
        {
            val += strlen("Name=");
            strstripspace(val);
            break;
        }
    }
    fclose(fp);

    return val ? strdup(val) : NULL;
}

kdk_package_list *kdk_package_get_packagelist()
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    kdk_package_list *packagelist = (kdk_package_list *)calloc(1, sizeof *packagelist);
    if (!packagelist)
        goto err_out;

    char buffer[4097];
    kdk_package_t *curpackage = NULL;
    size_t maxpackage = 0;
    do
    {
        if (!fgets(buffer, 4096, fp))
            continue;
        char *val;
        if ((val = lookup(buffer, "Package")))
        {
            curpackage = (kdk_package_t *)calloc(1, sizeof(kdk_package_t));
            if (!curpackage)
                goto err_out;
            curpackage->name = strdup(val);
            if (packagelist->nums == maxpackage)
            {
                maxpackage += PACKAGE_INCREASE_STEP;
                kdk_package_t **tmp = (kdk_package_t **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_t *));
                if (!tmp)
                {
                    free(curpackage);
                    goto err_out;
                }
                packagelist->list = tmp;
            }
            packagelist->list[packagelist->nums] = curpackage;
            packagelist->nums++;
        }
        else if ((val = lookup(buffer, "Status")))
        {
            if (strcmp(val, "install ok installed") == 0)
            {
                curpackage->status = strdup("ii");
            }
            else if (strcmp(val, "deinstall ok config-files") == 0)
            {
                curpackage->status = strdup("rc");
            }
        }
        else if ((val = lookup(buffer, "Section")))
        {
            curpackage->section = strdup(val);
        }
        else if ((val = lookup(buffer, "Version")))
        {
            curpackage->version = strdup(val);
        }
        else if ((val = lookup(buffer, "Installed-Size")))
        {
            curpackage->size_kb = atol(val);
        }
        else if ((val = lookup(buffer, "Depends")))
        {
        }
    } while (feof(fp) == 0);

    fclose(fp);
    return packagelist;

err_out:
#ifdef __linux__
    fclose(fp);
#endif
    kdk_package_free_packagelist(packagelist);
    return NULL;
}

static void _free_package_t(kdk_package_t *package)
{
    if (package->name)
        free(package->name);
    if (package->section)
        free(package->section);
    if (package->version)
        free(package->version);
    if (package->status)
        free(package->status);
    if (package->depends)
        free(package->depends);
}

void kdk_package_free_packagelist(kdk_package_list *pl)
{
    if (!pl)
        return;

    if (pl->list)
    {
        for (int i = 0; i < pl->nums; i++)
        {
            _free_package_t(pl->list[i]);
            free(pl->list[i]);
        }
        free(pl->list);
    }
    free(pl);
}

int kdk_package_install_package(const char *packagePath)
{
}

int kdk_package_remove_package(const char *name, const char *version)
{
}

char *kdk_package_get_version(const char *name)
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    char buffer[4097];
    char *val = NULL;
    int flag = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
                flag = 1;
        }
        else if (flag && (val = lookup(buffer, "Version")))
            break;
    }
    fclose(fp);
    return val ? strdup(val) : NULL;
}

int kdk_package_is_installed(const char *name, const char *version)
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (!fp)
        return -1;

    char buffer[4097];
    char *val;
    int flag = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
            {
                flag = 1;
                break;
            }
        }
    }
    fclose(fp);
    return flag;
}

char *kdk_package_get_description(const char *name)
{
    if (NULL == name)
        return NULL;

    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    char buffer[4097], result[4096];
    char *val = NULL;
    int flag = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
                flag = 1;
        }
        else if (flag && (val = lookup(buffer, "Description")))
        {
            strcat(result, val);
            // strcat(result,"\n");
            // while (fgets(buffer,4096, fp))
            // {
            //     if ((val = lookup(buffer, "Homepage")))
            //         break;
            //     strcat(result, buffer);
            // }
            break;
        }
    }
    fclose(fp);
    return val ? strdup(result) : NULL;
}

char **kdk_package_get_code_path(const char *name)
{
    if (NULL == name)
        return NULL;

    char **result = calloc(1, sizeof(char *));
    char cmd[128];
    sprintf(cmd, "dpkg -L %s", name);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return 0;

    int i = 0;
    char line[1024];
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        struct stat statbuf;
        if (stat(line, &statbuf) != 0)
        {
            fclose(fp);
            return 0;
        }
        if (S_ISREG(statbuf.st_mode))
        {
            char *dir_name = dirname(line);
            if ((i > 0) && (0 == strcmp(result[i - 1], dir_name)))
                continue;
            char **tmp = realloc(result, sizeof(char *) * (i + 2));
            if (NULL == tmp)
            {
                for (; i; i--)
                {
                    free(result[i]);
                }
                free(result);
                result = NULL;
                break;
            }
            result = tmp;
            result[i++] = strdup(dir_name);
            result[i] = NULL;
        }
    }
    fclose(fp);
    return result;
}

int kdk_package_get_file_count(const char *name)
{
    if (NULL == name)
        return 0;

    int result = 0;
    char cmd[128];
    sprintf(cmd, "dpkg -L %s", name);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return 0;

    char line[1024];
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        struct stat statbuf;
        if (stat(line, &statbuf) != 0)
        {
            fclose(fp);
            return 0;
        }
        if (S_ISREG(statbuf.st_mode))
            result++;
    }
    fclose(fp);
    return result;
}

int kdk_package_is_removable(const char *name)
{
    return 0;
}

int kdk_package_verify_disk_space(const char *path)
{
    if (NULL == path)
        return 0;

    int result = 0;

    char cmd[128];
    sprintf(cmd, "dpkg -I %s", path);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return 0;

    char line[1024];
    char *val = NULL;
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        if ((val = lookup(line, "Installed-Size")))
        {
            break;
        }
    }
    fclose(fp);

    if (NULL == val)
        return 0;

    FILE *mounts = fopen("/proc/mounts", "r");

    char partition[100], mount_point[100];
    while (fscanf(mounts, "%s %s", partition, mount_point) != EOF)
    {
        if (strcmp(mount_point, "/") == 0)
        {
            break;
        }
    }
    fclose(mounts);

    struct statfs fs_stat;
    if (statfs(partition, &fs_stat) < 0)
    {
        return 0;
    }
    long install_size = atol(val) * 1024;
    long avail_space = fs_stat.f_bavail * fs_stat.f_bsize;

    return avail_space > install_size;
}

char *kdk_package_get_default_browser(void)
{
    return read_default_app("x-scheme-handler/http");
}

char *kdk_package_get_default_image_viewer(void)
{
    return read_default_app("image/bmp");
}

char *kdk_package_get_default_audio_player(void)
{
    return read_default_app("audio/mpeg");
}

char *kdk_package_get_default_video_player(void)
{
    return read_default_app("video/mp4");
}

char *kdk_package_get_default_pdf_viewer(void)
{
    return read_default_app("application/pdf");
}

char *kdk_package_get_default_word_viewer(void)
{
    return read_default_app("application/wps-office.docx");
}

char *kdk_package_get_default_excel_viewer(void)
{
    return read_default_app("application/wps-office.xls");
}

char *kdk_package_get_default_ppt_viewer(void)
{
    return read_default_app("application/wps-office.ppt");
}

kdk_startmenu_list *kdk_package_get_startmenu_list(void)
{
    kdk_startmenu_list *programlist = (kdk_startmenu_list *)calloc(1, sizeof(kdk_startmenu_list));
    if (!programlist)
        return NULL;

    get_desktop_info(KDK_DEKSTOP_FILE_PATH, programlist);

    char *homeDir = getenv("HOME");
    char filepath[PROGRAM_SIZE] = {0};
    sprintf(filepath, "%s%s", homeDir, KDK_LOCALE_DEKSTOP_FILE_PATH);
    get_desktop_info(filepath, programlist);

    get_desktop_info(KDK_SNAPD_DEKSTOP_FILE_PATH, programlist);

    get_desktop_info(KDK_FLATPAK_DEKSTOP_FILE_PATH, programlist);

    return programlist;
}

void kdk_package_free_startmenu_list(kdk_startmenu_list *list)
{
    if(NULL == list)
        return;
    for(int i = 0; i < list->nums; i++)
        free(list->list[i]);
    free(list);
}
