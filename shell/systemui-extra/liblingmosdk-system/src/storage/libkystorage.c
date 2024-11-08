#include "libkystorage.h"
#include "libkylog.h"
#include "../packages/libkypackages.h"
#include "magic.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h> //for io

#include <pwd.h>
#include <grp.h>

#include <dirent.h>

typedef enum _BrowserEnum
{
    None,
    Qax,
    FireFox,
    Chrome,
    Chromium
} BrowserEnum;

static char *_kdk_storage_get_Cache(BrowserEnum type)
{
    char *home = getenv("HOME");
    char result[1024];
    memset(result, 0, 1024);
    switch (type)
    {
    case Qax:
    {
        sprintf(result, "%s/.config/qaxbrowser/Default/Cache", home);
    }
    break;
    case FireFox:
    {
        char profiles[1024];
        sprintf(profiles, "%s/.mozilla/firefox/profiles.ini", home);
        FILE *fp = fopen(profiles, "r");
        if (NULL == fp)
        {
            klog_err("%s -> Read firefox profile failed: \n", __func__, strerror(errno));
            break;
        }
        char line[1024];
        int start = 0, def = 0;
        char configs[10][64];
        memset(configs, 0, 10 * 64);
        char path[64];
        memset(path, 0, 64);
        while (fgets(line, sizeof(line), fp))
        {
            line[strcspn(line, "\n")] = 0;

            // Parse key and value
            char *key = strtok(line, "=");
            char *value = strtok(NULL, "=");
            if (key != NULL && value != NULL)
            {
                if (0 == strcmp(key, "StartWithLastProfile"))
                    start = atoi(value);
                if (0 == strcmp(key, "Path"))
                {
                    strncpy(path, value, strlen(value));
                }
                if (0 == strcmp(key, "Default"))
                {
                    def = atoi(value);
                }
            }
            else
            {
                memcpy(configs[def], path, 64);
                def = 0;
                memset(path, 0, 64);
            }
        }
        fclose(fp);
        if (start > 0)
            sprintf(result, "%s/.mozilla/firefox/%s/cache2", home, configs[start]);
        else
        {
            klog_err("%s -> Not have default profile\n", __func__);
            return NULL;
        }
    }
    break;
    case Chrome:
    {
        sprintf(result, "%s/.config/google-chrome/Default/Cache", home);
    }
    break;
    case Chromium:
    {
        sprintf(result, "%s/.config/chromium/Default/Cache", home);
    }
    break;
    default:
    {
        klog_err("%s -> Unsupported browser\n", __func__);
    }
    break;
    }
    return strlen(result) > 0 ? strdup(result) : NULL;
}

static char *_kdk_storage_get_cookie(BrowserEnum type)
{
    char *home = getenv("HOME");
    char result[1024];
    memset(result, 0, 1024);
    switch (type)
    {
    case Qax:
    {
        sprintf(result, "%s/.config/qaxbrowser/Default/Cookies", home);
    }
    break;
    case FireFox:
    {
        char profiles[1024];
        sprintf(profiles, "%s/.mozilla/firefox/profiles.ini", home);
        FILE *fp = fopen(profiles, "r");
        if (NULL == fp)
        {
            klog_err("%s -> Read firefox profile failed: \n", __func__, strerror(errno));
            break;
        }
        char line[1024];
        int start = 0, def = 0;
        char configs[10][64];
        memset(configs, 0, 10 * 64);
        char path[64];
        memset(path, 0, 64);
        while (fgets(line, sizeof(line), fp))
        {
            line[strcspn(line, "\n")] = 0;

            // Parse key and value
            char *key = strtok(line, "=");
            char *value = strtok(NULL, "=");
            if (key != NULL && value != NULL)
            {
                if (0 == strcmp(key, "StartWithLastProfile"))
                    start = atoi(value);
                if (0 == strcmp(key, "Path"))
                {
                    strncpy(path, value, strlen(value));
                }
                if (0 == strcmp(key, "Default"))
                {
                    def = atoi(value);
                }
            }
            else
            {
                memcpy(configs[def], path, 64);
                def = 0;
                memset(path, 0, 64);
            }
        }
        fclose(fp);
        if (start > 0)
            sprintf(result, "%s/.mozilla/firefox/%s/cookies.sqlite", home, configs[start]);
        else
        {
            klog_err("%s -> Not have default profile\n", __func__);
            return NULL;
        }
    }
    break;
    case Chrome:
    {
        sprintf(result, "%s/.config/google-chrome/Default/Cookies", home);
    }
    break;
    case Chromium:
    {
        sprintf(result, "%s/.config/chromium/Default/Cookies", home);
    }
    break;
    default:
    {
        klog_err("%s -> Unsupported browser\n", __func__);
    }
    break;
    }
    return strlen(result) > 0 ? strdup(result) : NULL;
}

char *kdk_storage_get_default_cookie_path()
{
    char *default_browser = kdk_package_get_default_browser();
    if (NULL == default_browser)
        return NULL;

    klog_info("%s -> Default Browser %s\n", __func__, default_browser);
    char *result = NULL;
    if (strstr(default_browser, "Qax"))
        result = _kdk_storage_get_cookie(Qax);
    else if (strstr(default_browser, "Chrome"))
        result = _kdk_storage_get_cookie(Chrome);
    else if (strstr(default_browser, "Firefox"))
        result = _kdk_storage_get_cookie(FireFox);
    else if (strstr(default_browser, "Chromium"))
        result = _kdk_storage_get_cookie(Chromium);
    else
        result = _kdk_storage_get_cookie(None);

    return result;
}

char *kdk_storage_get_default_desktop_path()
{
    char path[1024];
    sprintf(path, "%s/%s", getenv("HOME"), "桌面");
    if (0 == access(path, F_OK))
        return strdup(path);
    sprintf(path, "%s/%s", getenv("HOME"), "desktop");
    if (0 == access(path, F_OK))
        return strdup(path);
    return NULL;
}

char *kdk_storage_get_default_documents_path()
{
    char path[1024];
    sprintf(path, "%s/%s", getenv("HOME"), "文档");
    if (0 == access(path, F_OK))
        return strdup(path);
    sprintf(path, "%s/%s", getenv("HOME"), "documents");
    if (0 == access(path, F_OK))
        return strdup(path);
    return NULL;
}

char *kdk_storage_get_default_internetCache_path(char *name)
{
    if (NULL == name)
    {
        name = kdk_package_get_default_browser();
        if (NULL == name)
            return NULL;
    }
    char *result = NULL;
    if (strstr("qax", name))
        result = _kdk_storage_get_Cache(Qax);
    else if (strstr("chrome", name))
        result = _kdk_storage_get_Cache(Chrome);
    else if (strstr("firefox", name))
        result = _kdk_storage_get_Cache(FireFox);
    else if (strstr("chromeium", name))
        result = _kdk_storage_get_Cache(Chromium);
    else
        result = _kdk_storage_get_Cache(None);
    return result;
}

char *kdk_storage_get_default_download_path()
{
    char path[1024];
    sprintf(path, "%s/%s", getenv("HOME"), "下载");
    if (0 == access(path, F_OK))
        return strdup(path);
    sprintf(path, "%s/%s", getenv("HOME"), "download");
    if (0 == access(path, F_OK))
        return strdup(path);
    return NULL;
}

char *kdk_storage_get_file_ContentType(char *file_name)
{
    magic_t magic_cookie;
    const char *mime = NULL;

    magic_cookie = magic_open(MAGIC_MIME_TYPE);
    if (magic_cookie == NULL)
    {
        klog_err("%s -> Unable to initialize magic library\n", __func__);
        return NULL;
    }

    if (magic_load(magic_cookie, NULL) != 0)
    {
        klog_err("%s -> Cannot load magic database - %s\n", __func__, magic_error(magic_cookie));
        goto out;
    }

    mime = magic_file(magic_cookie, file_name);
    if (mime == NULL)
    {
        klog_err("%s -> Cannot determine file type\n", __func__);
        goto out;
    }
    mime = strdup(mime);

out:
    magic_close(magic_cookie);
    return mime;
}

char *kdk_storage_get_file_dateChange(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed\n", __func__, strerror(errno));
        return NULL;
    }

    char time[32];
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_ctime));

    return strdup(time);
}

char *kdk_storage_get_file_dateModify(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed\n", __func__, strerror(errno));
        return NULL;
    }

    char time[32];
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));

    return strdup(time);
}

char *kdk_storage_get_file_dateAccess(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed\n", __func__, strerror(errno));
        return NULL;
    }

    char time[32];
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_atime));

    return strdup(time);
}

long kdk_storage_get_file_size(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return 0;
    }

    return file_stat.st_size;
}

int kdk_storage_read_file(const char *file_name, char *buffer, unsigned long size)
{
    if (-1 == access(file_name, F_OK))
    {
        klog_err("%s -> File not exists\n", __func__);
        return -1;
    }

    if (kdk_storage_get_file_size(file_name) > (size - 1))
    {
        klog_err("%s -> The buffer is insufficient\n", __func__);
        return -1;
    }

    int fd = open(file_name, O_RDONLY);
    if (-1 == fd)
    {
        klog_err("%s -> Open file failed: %s\n", __func__, strerror(errno));
        return -1;
    }

    int i = 0, ret = 1, count = 0;
    while (ret > 0)
    {
        ret = read(fd, buffer + (i++ * 2048), 2048);
        if (-1 == ret)
        {
            klog_err("%s -> Read file failed: %s\n", __func__, strerror(errno));
            memset(buffer, 0, count);
            count = -1;
            break;
        }
        count += ret;
    }
    close(fd);
    return count;
}

int kdk_storage_write_file(const char *file_name, const char *data, unsigned long length)
{
    if (-1 == access(file_name, F_OK))
    {
        klog_err("%s -> File not exists\n", __func__);
        return -1;
    }

    int fd = open(file_name, O_WRONLY | O_APPEND);
    if (-1 == fd)
    {
        klog_err("%s -> Open file failed: %s\n", __func__, strerror(errno));
        return -1;
    }

    int ret = write(fd, data, length);
    if (-1 == ret)
    {
        klog_err("%s -> Write file failed: %s\n", __func__, strerror(errno));
    }

    close(fd);
    return ret;
}

int kdk_storage_move_file(const char *source, const char *destination)
{
    int ret = rename(source, destination);
    if (-1 == ret)
    {
        klog_err("%s -> Move file failed: %s\n", __func__, strerror(errno));
    }
    return ret;
}

char *kdk_storage_get_file_permissions(const char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }
    unsigned int permission = 0;
    permission |= ((file_stat.st_mode & S_IRWXU) << 2);
    permission |= ((file_stat.st_mode & S_IRWXG) << 1);
    permission |= ((file_stat.st_mode & S_IRWXO));

    char result[4];
    sprintf(result, "%x", permission);
    return strdup(result);
}

char *kdk_storage_get_file_owner(const char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    struct passwd *pw = getpwuid(file_stat.st_uid);
    if (NULL == pw)
    {
        klog_err("%s -> Getting passwd failed: %s\n", __func__, strerror(errno));
        return NULL;
    }
    char *result = strdup(pw->pw_name);
    endpwent();
    return result;
}

char *kdk_storage_get_file_group(const char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    struct group *gr = getgrgid(file_stat.st_uid);
    if (NULL == gr)
    {
        klog_err("%s -> Getting group failed: %s\n", __func__, strerror(errno));
        return NULL;
    }
    char *result = strdup(gr->gr_name);
    endgrent();
    return result;
}

FileInfo *kdk_storage_list_files(const char *path)
{
    FileInfo *info = NULL;
    DIR *dir;
    struct dirent *entry;

    // 打开当前目录
    dir = opendir(path);
    if (dir == NULL)
    {
        klog_err("%s -> Open directory fialed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    // 读取目录中的文件和子目录
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") ^ strcmp(entry->d_name, ".."))
            continue;

        FileInfo *tmp = calloc(1, sizeof(FileInfo));
        if (NULL == tmp)
        {
            klog_err("%s -> Memory error\n", __func__);
            kdk_storage_free_file_info(info);
            info = NULL;
            break;
        }

        strcpy(tmp->name, entry->d_name);
        tmp->is_dir = (DT_DIR == entry->d_type) ? 1 : 0;

        tmp->next = info;
        info = tmp;
    }

    // 关闭目录
    closedir(dir);
    return info;
}

void kdk_storage_free_file_info(FileInfo *info)
{
    FileInfo *tmp = info;
    while (info)
    {
        tmp = info;
        info = info->next;
        free(tmp);
    }
}

char *kdk_storage_resolve_symbolic_link(const char *file_name)
{
    char *result = NULL;

    struct stat file_stat;
    if (lstat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    if (S_ISLNK(file_stat.st_mode))
    {
        char link_target[1024];
        memset(link_target, 0, 1024);
        ssize_t len = readlink(file_name, link_target, sizeof(link_target) - 1);
        if (-1 == len)
        {
            klog_err("%s -> Read link failed: %s\n", __func__, strerror(errno));
            return NULL;
        }
        result = strdup(link_target);
    }
    else if (file_stat.st_nlink > 1)
    {
        // 硬链接
    }
    else
    {
        klog_err("%s -> %s is neither a symbolic link nor a hard link\n", __func__, file_name);
    }

    return result;
}
