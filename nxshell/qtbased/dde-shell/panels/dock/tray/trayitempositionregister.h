// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QQmlEngine>
#include <QSize>
#include <QPoint>

namespace docktray {

class TrayItemPositionRegisterAttachedType : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString surfaceId MEMBER m_surfaceId NOTIFY surfaceIdChanged)
    Q_PROPERTY(QString sectionType MEMBER m_sectionType NOTIFY sectionTypeChanged)
    Q_PROPERTY(int visualIndex MEMBER m_visualIndex NOTIFY visualIndexChanged)
    Q_PROPERTY(QSize visualSize MEMBER m_visualSize NOTIFY visualSizeChanged)
    Q_PROPERTY(QPoint visualPosition READ visualPosition NOTIFY visualPositionChanged)
    QML_ANONYMOUS
public:
    TrayItemPositionRegisterAttachedType(QObject *parent);

    QPoint visualPosition() const;

signals:
    void surfaceIdChanged(QString);
    void sectionTypeChanged(QString);
    void visualIndexChanged(int);
    void visualSizeChanged(QSize);
    void visualPositionChanged();

private:
    void registerVisualSize();

    QString m_surfaceId;
    QString m_sectionType;
    int m_visualIndex = -1;
    QSize m_visualSize;
};

// -------------------------------------------------------

class TrayItemPositionRegister : public QObject
{
    Q_OBJECT
    QML_ATTACHED(TrayItemPositionRegisterAttachedType)
    QML_ELEMENT
public:
    static TrayItemPositionRegisterAttachedType *qmlAttachedProperties(QObject *object)
    {
        return new TrayItemPositionRegisterAttachedType(object);
    }
};

}
