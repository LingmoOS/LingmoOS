// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOG_H
#define LOG_H

#include "logger.h"

#define _LOG_FNAME lingmo_cross::path_base(__FILE__)
//#define _LOG_FNLEN lingmo_cross::path_base_len(__FILE__)
#define _LOG_FILELINE _LOG_FNAME,__LINE__

#define _LOG_STREAM(lv)  lingmo_cross::Logger::GetInstance().log(_LOG_FILELINE, lv)


#define DLOG  if (lingmo_cross::g_logLevel <= lingmo_cross::debug)   _LOG_STREAM(lingmo_cross::debug)
#define LOG   if (lingmo_cross::g_logLevel <= lingmo_cross::info)    _LOG_STREAM(lingmo_cross::info)
#define WLOG  if (lingmo_cross::g_logLevel <= lingmo_cross::warning) _LOG_STREAM(lingmo_cross::warning)
#define ELOG  if (lingmo_cross::g_logLevel <= lingmo_cross::error)   _LOG_STREAM(lingmo_cross::error)
#define FLOG  if (lingmo_cross::g_logLevel <= lingmo_cross::fatal)   _LOG_STREAM(lingmo_cross::fatal)

// conditional log
#define DLOG_IF(cond) if (cond) DLOG
#define  LOG_IF(cond) if (cond) LOG
#define WLOG_IF(cond) if (cond) WLOG
#define ELOG_IF(cond) if (cond) ELOG
#define FLOG_IF(cond) if (cond) FLOG

#endif // LOG_H
