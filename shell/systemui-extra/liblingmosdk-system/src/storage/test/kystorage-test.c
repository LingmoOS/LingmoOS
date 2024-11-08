#include "../libkystorage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char *cookie_path = kdk_storage_get_default_cookie_path();
    if (cookie_path)
    {
        printf("Cookie path: %s\n", cookie_path);
        free(cookie_path);
    }

    char *desktop_path = kdk_storage_get_default_desktop_path();
    if (desktop_path)
    {
        printf("Desktop path: %s\n", desktop_path);
        free(desktop_path);
    }

    char *documents_path = kdk_storage_get_default_documents_path();
    if (documents_path)
    {
        printf("Documents path: %s\n", documents_path);
        free(documents_path);
    }

    char *cache_path = kdk_storage_get_default_internetCache_path("firefox");
    if (cache_path)
    {
        printf("Cache path: %s\n", cache_path);
        free(cache_path);
    }

    char *download_path = kdk_storage_get_default_download_path();
    if (download_path)
    {
        printf("Download path: %s\n", download_path);
        free(download_path);
    }

    char *content_type = kdk_storage_get_file_ContentType("/home/lingmo/test.yaml");
    if (content_type)
    {
        printf("Content type: %s\n", content_type);
        free(content_type);
    }

    char *changed_date = kdk_storage_get_file_dateChange("/home/lingmo/test.yaml");
    if (changed_date)
    {
        printf("File change date: %s\n", changed_date);
        free(changed_date);
    }

    char *modify_date = kdk_storage_get_file_dateModify("/home/lingmo/test.yaml");
    if (modify_date)
    {
        printf("File modify date: %s\n", modify_date);
        free(modify_date);
    }

    char *access_date = kdk_storage_get_file_dateAccess("/home/lingmo/test.yaml");
    if (access_date)
    {
        printf("File access date: %s\n", access_date);
        free(access_date);
    }

    long size = kdk_storage_get_file_size("/home/lingmo/test.yaml");
    printf("File size: %ld\n", size);

    char buffer[size + 1];
    memset(buffer, 0, size + 1);
    int ret = kdk_storage_read_file("/home/lingmo/test.yaml", buffer, size + 1);
    if (-1 != ret)
    {
        printf("Read file success\n");
    }

    ret = kdk_storage_write_file("/home/lingmo/dest.yaml", buffer, size);
    if (-1 != ret)
    {
        printf("Write file success\n");
    }

    // ret = kdk_storage_move_file("/home/lingmo/dest.yaml", "/home/lingmo/yaml/test.yaml");
    // if (-1 != ret)
    // {
    //     printf("Move file success\n");
    // }

    char *permission = kdk_storage_get_file_permissions("/home/lingmo/test.yaml");
    if (permission)
    {
        printf("Permission: %s\n", permission);
        free(permission);
    }

    char *owner = kdk_storage_get_file_owner("/home/lingmo/test.yaml");
    if (owner)
    {
        printf("Owner: %s\n", owner);
        free(owner);
    }

    char *group = kdk_storage_get_file_group("/home/lingmo/test.yaml");
    if (group)
    {
        printf("Group: %s\n", group);
        free(group);
    }

    FileInfo *info = kdk_storage_list_files("/home/lingmo");
    if (info)
    {
        FileInfo *tmp = info;
        while (tmp)
        {
            printf("\t%s: %s\n", tmp->is_dir ? "SubFolder" : "SubFile", tmp->name);
            tmp = tmp->next;
        }
        kdk_storage_free_file_info(info);
    }

    char *link = kdk_storage_resolve_symbolic_link("/usr/lib/libkysset.so");
    if (link)
    {
        printf("Link file: %s\n", link);
        free(link);
    }
}