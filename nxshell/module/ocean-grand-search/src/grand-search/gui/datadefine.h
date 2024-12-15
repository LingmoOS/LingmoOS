// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATADEFINE_H
#define DATADEFINE_H

#include <QString>
#include <QObject>
#include <QPluginLoader>

// 组-列表对象名
#define GroupObjName_App            "Group_App"
#define GroupObjName_Setting        "Group_Setting"
#define GroupObjName_Video          "Group_Video"
#define GroupObjName_Audio          "Group_Audio"
#define GroupObjName_Picture        "Group_Picture"
#define GroupObjName_Web            "Group_Web"
#define GroupObjName_Document       "Group_Document"
#define GroupObjName_Folder         "Group_Folder"
#define GroupObjName_File           "Group_File"

// 组-中文词典翻译
#define GroupName_Best              QObject::tr("Best match")
#define GroupName_App               QObject::tr("Applications")
#define GroupName_Setting           QObject::tr("Settings")
#define GroupName_Video             QObject::tr("Videos")
#define GroupName_Audio             QObject::tr("Music")
#define GroupName_Picture           QObject::tr("Pictures")
#define GroupName_Document          QObject::tr("Documents")
#define GroupName_Folder            QObject::tr("Folders")
#define GroupName_File              QObject::tr("Files")
#define GroupName_Web               QObject::tr("Web")
#define GroupName_Inference         QObject::tr("AI Search")

#define GROUP_MAX_SHOW              5   // 每组默认显示数量

namespace GrandSearch {

//预览插件信息
struct PreviewPluginInfo {
    QString name;               // 预览插件名称
    QStringList mimeTypes;      // 插件支持的所有文件类型，mimeType
    QString path;               // 插件*.so所在路径
    QPluginLoader* pPlugin;     // 预览插件实例对象
    bool bValid;                // 插件是否有效
    QString version;            // 插件适配版本号

    PreviewPluginInfo() {
        reset();
    }

    void reset() {
        name = "";
        mimeTypes.clear();
        path = "";
        pPlugin = nullptr;
        bValid = false;
        version = "";
    }
};

typedef QList<PreviewPluginInfo> PreviewPluginInfoList;

}

#endif // DATADEFINE_H
