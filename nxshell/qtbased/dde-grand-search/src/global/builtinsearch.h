// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILTINSEARCH_H
#define BUILTINSEARCH_H

// 定义内置搜索项的名称
#define GRANDSEARCH_CLASS_FILE_DEEPIN "com.deepin.dde-grand-search.file-deepin"
#define GRANDSEARCH_CLASS_FILE_FSEARCH "com.deepin.dde-grand-search.file-fsearch"
#define GRANDSEARCH_CLASS_APP_DESKTOP "com.deepin.dde-grand-search.app-desktop"
#define GRANDSEARCH_CLASS_SETTING_CONTROLCENTER "com.deepin.dde-grand-search.dde-control-center-setting"
#define GRANDSEARCH_CLASS_WEB_STATICTEXT "com.deepin.dde-grand-search.web-statictext"
#define GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC "com.deepin.dde-grand-search.generalfile-semantic"

// AI搜索子项
#define GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC".analysis"
#define GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC".vector"

// 搜索项所属组
#define GRANDSEARCH_GROUP_BEST               "com.deepin.dde-grand-search.group.best-match"
#define GRANDSEARCH_GROUP_APP                "com.deepin.dde-grand-search.group.application"
#define GRANDSEARCH_GROUP_SETTING            "com.deepin.dde-grand-search.group.dde-control-center-setting"
#define GRANDSEARCH_GROUP_WEB                "com.deepin.dde-grand-search.group.web"

#define GRANDSEARCH_GROUP_FOLDER             "com.deepin.dde-grand-search.group.folder"

#define GRANDSEARCH_GROUP_FILE               "com.deepin.dde-grand-search.group.files"
#define GRANDSEARCH_GROUP_FILE_VIDEO         "com.deepin.dde-grand-search.group.files.video"
#define GRANDSEARCH_GROUP_FILE_AUDIO         "com.deepin.dde-grand-search.group.files.audio"
#define GRANDSEARCH_GROUP_FILE_PICTURE       "com.deepin.dde-grand-search.group.files.picture"
#define GRANDSEARCH_GROUP_FILE_DOCUMNET      "com.deepin.dde-grand-search.group.files.document"

#define GRANDSEARCH_GROUP_FILE_INFERENCE    "com.deepin.dde-grand-search.group.files.inference"

/****************** 扩展数据 *********************************/
// 搜索项在组内的显示等级
#define GRANDSEARCH_PROPERTY_ITEM_LEVEL      "itemLevel"
#define GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST   1
#define GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND  2
#define GRANDSEARCH_PROPERTY_ITEM_LEVEL_THIRD   3

// 权重计算方法
#define GRANDSEARCH_PROPERTY_WEIGHT_METHOD            "weightMethod"
#define GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE  "localFileMethod"
#define GRANDSEARCH_PROPERTY_WEIGHT_METHOD_APP        "desktopAppMethod"
#define GRANDSEARCH_PROPERTY_WEIGHT_METHOD_SETTING    "settingMethod"

// 权重属性
#define GRANDSEARCH_PROPERTY_ITEM_WEIGHT    "itemWeight"

// 拖尾数据
#define GRANDSEARCH_PROPERTY_ITEM_TAILER    "itemTailer"

// 匹配的到具体信息
#define GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT    "itemMatchedContext"

/****************** 扩展数据 END ******************************/

#define DEF_BUILTISEARCH_NAMES  \
static const QStringList predefBuiltinSearches { \
GRANDSEARCH_CLASS_FILE_DEEPIN, \
GRANDSEARCH_CLASS_FILE_FSEARCH, \
GRANDSEARCH_CLASS_APP_DESKTOP, \
GRANDSEARCH_CLASS_WEB_STATICTEXT, \
GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC \
}

#endif // BUILTINSEARCH_H
