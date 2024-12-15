// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PARSETHREADKERN_H
#define PARSETHREADKERN_H

#include "parsethreadbase.h"

/**
 * @brief The ParseThreadKern class  内核日志获取线程
 */
class ParseThreadKern :  public ParseThreadBase
{
    Q_OBJECT
public:
    explicit ParseThreadKern(QObject *parent = nullptr);
    ~ParseThreadKern() override;

protected:
    void run() override;

    void handleKern();

    qint64 formatDateTime(QString m, QString d, QString t);
    qint64 formatDateTime(QString y, QString t);

private:
    //日志显示时间(毫秒)
    qint64 iTime { 0 };
    //所有日志文件路径
    QStringList m_FilePath;
};

#endif  // PARSETHREADKERN_H
