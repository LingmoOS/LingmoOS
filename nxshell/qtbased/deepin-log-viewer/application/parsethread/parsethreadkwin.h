// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PARSETHREADKWIN_H
#define PARSETHREADKWIN_H

#include "parsethreadbase.h"

/**
 * @brief The ParseThreadKwin class  窗管日志获取线程
 */
class ParseThreadKwin :  public ParseThreadBase
{
    Q_OBJECT
public:
    explicit ParseThreadKwin(QObject *parent = nullptr);
    ~ParseThreadKwin() override;

protected:
    void run() override;

    void handleKwin();
};

#endif  // PARSETHREADKWIN_H
