// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOG_H
#define LOG_H

#include "logger.h"

#define _LOG_FNAME deepin_cross::path_base(__FILE__)
//#define _LOG_FNLEN deepin_cross::path_base_len(__FILE__)
#define _LOG_FILELINE _LOG_FNAME,__LINE__

#define _LOG_STREAM(lv)  deepin_cross::Logger::GetInstance().log(_LOG_FILELINE, lv)


#define DLOG  if (deepin_cross::g_logLevel <= deepin_cross::debug)   _LOG_STREAM(deepin_cross::debug)
#define LOG   if (deepin_cross::g_logLevel <= deepin_cross::info)    _LOG_STREAM(deepin_cross::info)
#define WLOG  if (deepin_cross::g_logLevel <= deepin_cross::warning) _LOG_STREAM(deepin_cross::warning)
#define ELOG  if (deepin_cross::g_logLevel <= deepin_cross::error)   _LOG_STREAM(deepin_cross::error)
#define FLOG  if (deepin_cross::g_logLevel <= deepin_cross::fatal)   _LOG_STREAM(deepin_cross::fatal)

// conditional log
#define DLOG_IF(cond) if (cond) DLOG
#define  LOG_IF(cond) if (cond) LOG
#define WLOG_IF(cond) if (cond) WLOG
#define ELOG_IF(cond) if (cond) ELOG
#define FLOG_IF(cond) if (cond) FLOG

#endif // LOG_H
