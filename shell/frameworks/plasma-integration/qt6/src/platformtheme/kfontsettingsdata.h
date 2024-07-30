/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000, 2006 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KFONTSETTINGSDATA_H
#define KFONTSETTINGSDATA_H

#include <QDBusVariant>
#include <QFont>
#include <QObject>
#include <ksharedconfig.h>

struct KFontData {
    const char *ConfigGroupKey;
    const char *ConfigKey;
    const char *FontName;
    int Size;
    int Weight;
    QFont::StyleHint StyleHint;
    const char *StyleName;
};

class KFontSettingsData : public QObject
{
    Q_OBJECT
public:
    // if adding a new type here also add an entry to DefaultFontData
    enum FontTypes {
        GeneralFont = 0,
        FixedFont,
        ToolbarFont,
        MenuFont,
        WindowTitleFont,
        TaskbarFont,
        SmallestReadableFont,
        FontTypesCount,
    };

public:
    KFontSettingsData();
    ~KFontSettingsData() override;

public Q_SLOTS:
    void dropFontSettingsCache();

private Q_SLOTS:
    void delayedDBusConnects();
    void slotPortalSettingChanged(const QString &group, const QString &key, const QDBusVariant &value);

public: // access, is not const due to caching
    QFont *font(FontTypes fontType);

private:
    QString readConfigValue(const QString &group, const QString &key, const QString &defaultValue = QString()) const;

    bool mUsePortal;
    QFont *mFonts[FontTypesCount];
    KSharedConfigPtr mKdeGlobals;
};

#endif // KFONTSETTINGSDATA_H
