// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNATIVESETTINGS_H
#define DNATIVESETTINGS_H

#define protected public
#include <private/qobject_p.h>
#undef protected

#include "global.h"

#include <QSet>
#include <private/qmetaobjectbuilder_p.h>

DPP_BEGIN_NAMESPACE

class DPlatformSettings;
class DNativeSettings : public QAbstractDynamicMetaObject
{
public:
    explicit DNativeSettings(QObject *base, DPlatformSettings *settings, bool global_settings);
    ~DNativeSettings();

    static QByteArray getSettingsProperty(QObject *base);
    bool isValid() const;

private:
    void init(const QMetaObject *meta_object);

    int createProperty(const char *, const char *) override;
    int metaCall(QMetaObject::Call, int _id, void **) override;
    bool isRelaySignal() const;

    static void onPropertyChanged(const QByteArray &name, const QVariant &property, DNativeSettings *handle);
    static void onSignal(const QByteArray &signal, qint32 data1, qint32 data2, DNativeSettings *handle);

    QObject *m_base;
    QMetaObject *m_metaObject = nullptr;
    QMetaObjectBuilder m_objectBuilder;
    int m_firstProperty;
    int m_propertyCount;
    // propertyChanged信号的index
    int m_propertySignalIndex;
    // VALID_PROPERTIES属性的index
    int m_flagPropertyIndex;
    // ALL_KEYS属性的index
    int m_allKeysPropertyIndex;
    // 用于转发base对象产生的信号的槽，使用native settings的接口将其发送出去. 值为0时表示不转发base对象的所有信号
    int m_relaySlotIndex = 0;
    DPlatformSettings *m_settings = nullptr;
    bool m_isGlobalSettings = false;

    static QHash<QObject*, DNativeSettings*> mapped;
};

DPP_END_NAMESPACE

#endif // DNATIVESETTINGS_H
