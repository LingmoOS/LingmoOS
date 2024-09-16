// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QMap>

const QString service = "com.deepin.pc.manager.session.daemon";
const QString path = "/com/deepin/pc/manager/session/daemon";

#define MB 1 << 10
#define GB 1 << 20
#define TB 1 << 30
#define KB_SECTION(temp) ((temp >= 0 && temp < (MB)) ? true : false)
#define MB_SECTION(temp) ((temp >= (MB) && temp < (GB)) ? true : false)
#define GB_SECTION(temp) ((temp >= (GB) && temp < (TB)) ? true : false)
#define TB_SECTION(temp) ((temp >= (TB)) ? true : false)
//解决转换解析的desktop与系统desktop不匹配问题

QString getSystemDesktop(QString desktop);
