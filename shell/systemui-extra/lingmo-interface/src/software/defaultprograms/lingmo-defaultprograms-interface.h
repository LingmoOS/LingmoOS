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


#ifndef LINGMODEFAULTPROGRAMSINTERFACE_H
#define LINGMODEFAULTPROGRAMSINTERFACE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _ContentType
{
    char *type;
}Contenttype;//用于存放应用支持的内容类型

typedef struct _Applist
{
    char *appid;
}AppList;//用于存放应用列表

/**
 * Gets the list of applications.
 * @content_type is the content-type the application supported.
 * @return the struct pointer.
 */
AppList *lingmo_software_defaultprograms_getappidlist(const char *content_type);

/**
 * Gets the current default application.
 * @content_type is the content-type the application supported.
 * @return the default application id.
 */
char *lingmo_software_defaultprograms_getdefaultappid(const char *content_type);

/**
 * Gets the content type supported by the application.
 * @appid is the application id.
 * return the struct pointer.
 */
Contenttype *lingmo_software_defaultprograms_getappcontenttype(char *appid);

/**
 * Adds a content type to the application information.
 * @content_type is the type that need to be added，Some content types are suggested as follows:
 * web browsers: "x-scheme-handler/http"
 * mail readers: "x-scheme-handler/mailto"
 * image viewers: "image/png"
 * media players: "audio/x-vorbis+ogg"
 * video players: "video/x-ogm+ogg"
 * text_editors: "text/plain".
 * @appid is the application id.
 * return ture on success, or false on error.
 */
bool lingmo_software_defaultprograms_addapptype(const char *content_type,
                                               char *appid);
/**
 * Removes a supported type from an application.
 * @content_type is a supported type.
 * @appid is the application id.
 * return ture on success, or false on error.
 */
bool lingmo_software_defaultprograms_delapptype(const char *content_type,
                                               char *appid);

/**
 * Set the default web browser.
 * @appid is the application id.
 */
bool lingmo_software_defaultprograms_setwebbrowsers(char *appid);

/**
 * Set the default mail readers.
 * @appid is the application id.
 */
bool lingmo_software_defaultprograms_setmailreaders(char *appid);

/**
 * Set the default image viewers.
 * @appid is the application id.
 */
bool lingmo_software_defaultprograms_setimageviewers(char *appid);

/**
 * Set the default audio players.
 * @appid is the application id.
 */
bool lingmo_software_defaultprograms_setaudioplayers(char *appid);

/**
 * Set the default video players.
 * @appid is the application id.
 */
bool lingmo_software_defaultprograms_setvideoplayers(char *appid);

/**
 * Set the default text editors.
 * @appid is the application id.
 */
bool lingmo_software_defaultprograms_settexteditors(char *appid);


#ifdef __cplusplus
}
#endif

#endif // LINGMODEFAULTPROGRAMSINTERFACE_H
