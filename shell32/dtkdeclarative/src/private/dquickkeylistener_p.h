// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKKEYLISTENER_P_H
#define DQUICKKEYLISTENER_P_H

#include <dtkdeclarative_global.h>

#include <DObject>
#include <QObject>
#include <QtQml/qqml.h>

class QQuickItem;
DQUICK_BEGIN_NAMESPACE

class DQuickKeyListenerPrivate;
class DQuickKeyListener : public QObject, DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DQuickKeyListener)
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(QStringList keys READ keys WRITE setKeys NOTIFY keysChanged)
    Q_PROPERTY(int maxKeyCount READ maxKeyCount WRITE setMaxKeyCount NOTIFY maxKeyCountChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(KeySequenceListener)
#endif

public:
    explicit DQuickKeyListener(QObject *parent = nullptr);

    QQuickItem *target() const;
    void setTarget(QQuickItem *item);

    const QStringList &keys() const;
    void setKeys(const QStringList &keys);

    int maxKeyCount() const;
    void setMaxKeyCount(int count);

    Q_INVOKABLE void clearKeys();

Q_SIGNALS:
    void targetChanged();
    void keysChanged();
    void maxKeyCountChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

DQUICK_END_NAMESPACE

#endif // DQUICKKEYLISTENER_P_H
