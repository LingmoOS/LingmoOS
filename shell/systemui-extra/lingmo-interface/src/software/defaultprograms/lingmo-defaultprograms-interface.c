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


#include <stdio.h>
#include <gio/gio.h>
#include <glib.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include "lingmo-defaultprograms-interface.h"

typedef struct _AppInfo
{
    GAppInfo *item;
}Appinfo;//用于存放应用列表信息

//获取给定类型的所有应用程序列表
static Appinfo *lingmo_software_defaultprograms_getapplist(const char *content_type)
{
    GList *applist;
    applist=g_app_info_get_all_for_type(content_type);
    GAppInfo* item;

    if(applist!=NULL)
    {
        guint len=g_list_length(applist);
        Appinfo *appinfo=(Appinfo *)malloc(sizeof(Appinfo)*(len+1));

        //获取应用列表
        for (guint index=0; index < len; index++)
        {
            item = (GAppInfo*) g_list_nth_data(applist, index);
            appinfo[index].item=item;
    //        const char *id=g_app_info_get_name(appinfo[index].item);
    //        const char *ID=g_app_info_get_id(appinfo[index].item);
    //        printf("%s,%s\n", id, ID);

        }

        appinfo[len].item=NULL;
        return appinfo;
    }
    else{
        return NULL;
    }

}

AppList *lingmo_software_defaultprograms_getappidlist(const char *content_type)
{
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    if(appinfo!=NULL)
    {
        gint i=0;
        while(appinfo[i].item!=NULL)
            i++;

        AppList *list=(AppList *)malloc(sizeof(AppList)*(i+1));
        int count=i;
        int index=0;
        for(gint j=0;appinfo[j].item!=NULL;j++)
        {

            const char *id=g_app_info_get_id(appinfo[j].item);
            if(id!=NULL)
            {
                gint len=strlen(id);
                list[index].appid=(char *)malloc(sizeof(char)*(len+1));
                strcpy(list[index].appid,id);
                index++;
            }
            else{
                free(list+count);
                count--;
            }

        }
        list[count].appid=NULL;
        free(appinfo);
        return list;
    }
    else{
        return NULL;
    }

}

//获取当前默认应用
char *lingmo_software_defaultprograms_getdefaultappid(const char *content_type)
{
    GAppInfo *app=g_app_info_get_default_for_type(content_type, false);
    if(app!=NULL)
    {
        const char *id=g_app_info_get_id(app);
        if(id!=NULL)
        {
            gint len=strlen(id);
            char *appid=(char *)malloc(sizeof(char)*(len+1));
            strcpy(appid,id);
            return appid;
        }
        else
        {
            return NULL;
        }

    }
    else{
        return NULL;
    }

}

//获取应用支持的内容类型
Contenttype *lingmo_software_defaultprograms_getappcontenttype(char *appid)
{
    GList *applist=g_app_info_get_all();

    if(applist!=NULL)
    {
        GAppInfo* item;

        guint len=g_list_length(applist);

        //获取应用列表
        for (guint index=0; index < len; index++)
        {
            item = (GAppInfo*) g_list_nth_data(applist, index);
            const char *id=g_app_info_get_id(item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                const char **types=g_app_info_get_supported_types(item);
                if(types!=NULL)
                {
                    int num=0;

                    while(*(types+num))
                    {
                        num++;
                    }
//                    printf("%d\n",num);

                    Contenttype *contenttype=(Contenttype *)malloc(sizeof(Contenttype)*(num+1));

                    for(int j=0;j<num;j++)
                    {
                        gint len=strlen(*(types+j));
                        contenttype[j].type=(char *)malloc(sizeof(char)*(len+1));
                        strcpy(contenttype[j].type,*(types+j));
                    }
                    contenttype[num].type=NULL;

                    return contenttype;
                }
                else return NULL;
            }
        }

    }
    return NULL;

}

//设置给定类型的默认处理程序
bool lingmo_software_defaultprograms_setdefaultapp(const char *content_type,
                                                  char *appid)
{
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    if(appinfo!=NULL)
    {
        bool judge=false;

        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                judge=g_app_info_set_as_default_for_type(appitem,content_type, NULL);
                break;
            }
        }
        free(appinfo);
        return judge;
    }
    else{
        return false;
    }

}

//增加应用程序支持的类型
bool lingmo_software_defaultprograms_addapptype(const char *content_type,
                                               char *appid)
{
    bool judge=false;
    Contenttype *contenttype=lingmo_software_defaultprograms_getappcontenttype(appid);
    if(contenttype[0].type!=NULL)
    {
        const char *content_type1=contenttype[0].type;

        Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type1);

        if(appinfo!=NULL)
        {
            for(gint i=0;appinfo[i].item!=NULL;i++)
            {
                const char *id=g_app_info_get_id(appinfo[i].item);
                int result=strcmp(id,appid);
                if(result==0)
                {
                    GAppInfo *appitem=appinfo[i].item;
                    judge=g_app_info_add_supports_type(appitem,content_type,NULL);
                    break;
                }
            }
            gint j=0;
            while(contenttype[j].type!=NULL)
            {
                free(contenttype[j].type);
                j++;
            }

            free(appinfo);
        }

    }
    free(contenttype);

    return judge;

}

//删除应用程序支持的类型
bool lingmo_software_defaultprograms_delapptype(const char *content_type,
                                               char *appid)
{
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    bool judge=false;
    if(appinfo!=NULL)
    {
        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                judge=g_app_info_remove_supports_type(appitem,content_type,NULL);
                break;
            }
        }
        free(appinfo);
    }

    return judge;
}

//设置默认图像查看器
bool lingmo_software_defaultprograms_setimageviewers(char *appid)
{
    const char *content_type="image/png";
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    bool judge=false;
    if(appinfo!=NULL)
    {
        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "image/bmp", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "image/gif", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "image/jpeg", NULL);
                gboolean ret4=g_app_info_set_as_default_for_type(appitem, "image/png", NULL);
                gboolean ret5=g_app_info_set_as_default_for_type(appitem, "image/tiff", NULL);
                if(ret1==true && ret2==true && ret3==true && ret4==true && ret5==true)
                    judge=true;
                break;
            }
        }
        free(appinfo);
    }

    return judge;
}

//设置默认音频播放器
bool lingmo_software_defaultprograms_setaudioplayers(char *appid)
{
    const char *content_type="audio/x-vorbis+ogg";
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    bool judge=false;
    if(appinfo!=NULL)
    {
        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "audio/mpeg", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "audio/x-mpegurl", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "audio/x-scpls", NULL);
                gboolean ret4=g_app_info_set_as_default_for_type(appitem, "audio/x-vorbis+ogg", NULL);
                gboolean ret5=g_app_info_set_as_default_for_type(appitem, "audio/x-wav", NULL);
                if(ret1==true && ret2==true && ret3==true && ret4==true && ret5==true)
                    judge=true;
                break;
            }
        }
        free(appinfo);
    }

    return judge;
}

//设置默认视频播放器
bool lingmo_software_defaultprograms_setvideoplayers(char *appid)
{
    const char *content_type="video/x-ogm+ogg";
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    bool judge=false;
    if(appinfo!=NULL)
    {
        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "video/mp4", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "video/mpeg", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "video/mp2t", NULL);
                gboolean ret4=g_app_info_set_as_default_for_type(appitem, "video/msvideo", NULL);
                gboolean ret5=g_app_info_set_as_default_for_type(appitem, "video/quicktime", NULL);
                gboolean ret6=g_app_info_set_as_default_for_type(appitem, "video/webm", NULL);
                gboolean ret7=g_app_info_set_as_default_for_type(appitem, "video/x-avi", NULL);
                gboolean ret8=g_app_info_set_as_default_for_type(appitem, "video/x-flv", NULL);
                gboolean ret9=g_app_info_set_as_default_for_type(appitem, "video/x-matroska", NULL);
                gboolean ret10=g_app_info_set_as_default_for_type(appitem, "video/x-mpeg", NULL);
                gboolean ret11=g_app_info_set_as_default_for_type(appitem, "video/x-ogm+ogg", NULL);
                if(ret1==true && ret2==true && ret3==true && ret4==true && ret5==true &&
                        ret6==true && ret7==true && ret8==true && ret9==true && ret10==true && ret11==true)
                    judge=true;
                break;
            }
        }
        free(appinfo);
    }

    return judge;
}

//设置默认网络浏览器
bool lingmo_software_defaultprograms_setwebbrowsers(char *appid)
{
    const char *content_type="x-scheme-handler/http";
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    bool judge=false;
    if(appinfo!=NULL)
    {
        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/http", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/https", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/about", NULL);
                if(ret1==true && ret2==true && ret3==true)
                    judge=true;
                break;
            }
        }
        free(appinfo);
    }

    return judge;
}

//设置默认邮件阅读器
bool lingmo_software_defaultprograms_setmailreaders(char *appid)
{
    const char *content_type="x-scheme-handler/mailto";
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    bool judge=false;
    if(appinfo!=NULL)
    {
        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/mailto", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "application/x-extension-eml", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "message/rfc822", NULL);
                if(ret1==true && ret2==true && ret3==true)
                    judge=true;
                break;
            }
        }
        free(appinfo);
    }

    return judge;
}

//设置默认文件编辑器
bool lingmo_software_defaultprograms_settexteditors(char *appid)
{
    const char *content_type="text/plain";
    Appinfo *appinfo=lingmo_software_defaultprograms_getapplist(content_type);

    bool judge=false;
    if(appinfo!=NULL)
    {
        for(gint i=0;appinfo[i].item!=NULL;i++)
        {
            const char *id=g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if(result==0)
            {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "text/plain", NULL);
                if(ret1==true)
                    judge=true;
                break;
            }
        }
        free(appinfo);
    }

    return judge;
}
