/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd. <https://www.qt.io/licensing/>

    This file is part of the Qt Quick Controls module of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial
*/

#ifndef KQUICKPADDING_H
#define KQUICKPADDING_H

#include <QObject>

#include <qqmlregistration.h>

class KQuickPadding : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(int left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(int top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(int right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(int bottom READ bottom WRITE setBottom NOTIFY bottomChanged)

    int m_left;
    int m_top;
    int m_right;
    int m_bottom;

public:
    KQuickPadding(QObject *parent = nullptr)
        : QObject(parent)
        , m_left(0)
        , m_top(0)
        , m_right(0)
        , m_bottom(0)
    {
    }

    int left() const
    {
        return m_left;
    }
    int top() const
    {
        return m_top;
    }
    int right() const
    {
        return m_right;
    }
    int bottom() const
    {
        return m_bottom;
    }

public Q_SLOTS:
    void setLeft(int arg)
    {
        if (m_left != arg) {
            m_left = arg;
            Q_EMIT leftChanged();
        }
    }
    void setTop(int arg)
    {
        if (m_top != arg) {
            m_top = arg;
            Q_EMIT topChanged();
        }
    }
    void setRight(int arg)
    {
        if (m_right != arg) {
            m_right = arg;
            Q_EMIT rightChanged();
        }
    }
    void setBottom(int arg)
    {
        if (m_bottom != arg) {
            m_bottom = arg;
            Q_EMIT bottomChanged();
        }
    }

Q_SIGNALS:
    void leftChanged();
    void topChanged();
    void rightChanged();
    void bottomChanged();
};

#endif // QQUICKPADDING_H
