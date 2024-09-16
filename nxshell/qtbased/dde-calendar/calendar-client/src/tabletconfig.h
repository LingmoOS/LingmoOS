// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TABLETCONFIG_H
#define TABLETCONFIG_H

#include <QObject>

/**
 * @brief The TabletConfig class
 * 平板配置类
 */
class TabletConfig : public QObject
{
    Q_OBJECT
public:
    explicit TabletConfig(QObject *parent = nullptr);

    static bool isTablet();
    static void setIsTablet(bool isTablet);

signals:

public slots:
private:
    static bool m_isTablet; //是否为平板模式
};

#endif // TABLETCONFIG_H
