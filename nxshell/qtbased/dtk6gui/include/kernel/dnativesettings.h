// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNATIVESETTINGS_H
#define DNATIVESETTINGS_H

#include <dtkgui_global.h>
#include <DObject>

#include <QObject>

DGUI_BEGIN_NAMESPACE

class DNativeSettingsPrivate;
class DNativeSettings : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DNativeSettings)
    Q_PROPERTY(QByteArrayList allKeys READ allKeys WRITE __setAllKeys NOTIFY allKeysChanged)
public:
    explicit DNativeSettings(quint32 window, const QByteArray &domain = QByteArray(), QObject *parent = nullptr);

    bool isValid() const;
    QByteArrayList allKeys() const;

    QVariant getSetting(const QByteArray &name) const;
    void setSetting(const QByteArray &name, const QVariant &value);

Q_SIGNALS:
    void allKeysChanged();
    void propertyChanged(const QByteArray &name, const QVariant &value);

protected:
    DNativeSettings(DNativeSettingsPrivate &dd, const QMetaObject *metaObject, quint32 window, QObject *parent);
    DNativeSettings(const QMetaObject *metaObject, quint32 window, const QByteArray &domain, QObject *parent);

    bool init(const QMetaObject *metaObject, quint32 window);

private:
    void __setAllKeys(const QByteArrayList &keys);

    friend class DNativeSettingsPrivate;
};

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug debug, const DTK_GUI_NAMESPACE::DNativeSettings &settings);
QT_END_NAMESPACE

#endif // DNATIVESETTINGS_H
