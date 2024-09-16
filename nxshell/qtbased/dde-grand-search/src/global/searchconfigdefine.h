// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHCONFIGDEFINE_H
#define SEARCHCONFIGDEFINE_H

// 全局搜索后端的名称
#define GRANDSEARCH_DAEMON_NAME "dde-grand-search-daemon"
#define GRANDSEARCH_NAME "dde-grand-search"

// 搜索配置版本
#define GRANDSEARCH_VERSION_GROUP           "Version_Group"
#define GRANDSEARCH_VERSION_CONFIG          "version.config"
#define GRANDSEARCH_VERSION_CONFIG_CURRENT  "0.1"

// 搜索范围
#define GRANDSEARCH_SEARCH_GROUP            "Search_Group"

// 拖尾
#define GRANDSEARCH_TAILER_GROUP            "Tailer_File_Group"
#define GRANDSEARCH_TAILER_PARENTDIR        "tailer.file.parentDir"
#define GRANDSEARCH_TAILER_TIMEMODEFIED     "tailer.file.timeModified"

// 搜索体验计划
#define GRANDSEARCH_PLAN_GROUP              "Plan_Group"
#define GRANDSEARCH_PLAN_EXPERIENCE         "plan.experience"

// 路径黑名单
#define GRANDSEARCH_BLACKLIST_GROUP         "Blacklist_Group"
#define GRANDSEARCH_BLACKLIST_PATH          "Blacklist.path"

// 最佳匹配项
#define GRANDSEARCH_CUSTOM_GROUP                        "Custom_Group"
#define GRANDSEARCH_CUSTOM_BESTMATCH                    "custom.bestMatch"
#define GRANDSEARCH_CUSTOM_BESTMATCH_FIRSTITEMLIMIT     "custom.bestMatch.firstItemLimit"
#define GRANDSEARCH_CUSTOM_BESTMATCH_FIRSTWAITTIME      "custom.bestMatch.firstWaitTime"
#define GRANDSEARCH_CUSTOM_BESTMATCH_MAXCOUNT           "custom.bestMatch.maxCount"

// 搜索引擎
#define GRANDSEARCH_WEB_GROUP                       "Web_Group"
#define GRANDSEARCH_WEB_SEARCHENGINE                "web.searchEngine"
#define GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE         "Google"
#define GRANDSEARCH_WEB_SEARCHENGINE_BING           "Bing"
#define GRANDSEARCH_WEB_SEARCHENGINE_BAIDU          "Baidu"
#define GRANDSEARCH_WEB_SEARCHENGINE_YAHOO          "Yahoo"
#define GRANDSEARCH_WEB_SEARCHENGINE_360            "360"
#define GRANDSEARCH_WEB_SEARCHENGINE_360AI          "360AI"
#define GRANDSEARCH_WEB_SEARCHENGINE_SOGOU          "Sogou"
#define GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM         "Custom"
#define GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM_ADDR    "web.searchEngine.customAddr"

// AI搜索引擎
#define GRANDSEARCH_SEMANTIC_GROUP  "Semantic_Group"

// 配置切换界面参数
#define SWITCHWIDGETWIDTH       476
#define SWITCHWIDGETHEIGHT      36
#define SWITCHWIDGETICONSIZE    36
#define CHECKBOXITEMWIDTH       476
#define CHECKBOXITEMHEIGHT      36
#define COMBOBOXWIDGETWIDTH     476
#define COMBOBOXWIDGETHEIGHT    48
#define DETAILCHECKBOXITEMHEIGHT 57
#define DETAILSWITCHWIDGETHEIGHT 68

#endif // SEARCHCONFIGDEFINE_H
