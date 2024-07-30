/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSERVICEGROUPPRIVATE_H
#define KSERVICEGROUPPRIVATE_H

#include "kservicegroup.h"
#include <ksycocaentry_p.h>

#include <QStringList>

class KServiceGroupPrivate : public KSycocaEntryPrivate
{
public:
    K_SYCOCATYPE(KST_KServiceGroup, KSycocaEntryPrivate)

    explicit KServiceGroupPrivate(const QString &path)
        : KSycocaEntryPrivate(path)
        , m_bNoDisplay(false)
        , m_bShowEmptyMenu(false)
        , m_bShowInlineHeader(false)
        , m_bInlineAlias(false)
        , m_bAllowInline(false)
        , m_inlineValue(4)
        , m_bDeep(false)
        , m_childCount(-1)
    {
    }

    KServiceGroupPrivate(QDataStream &str, int offset)
        : KSycocaEntryPrivate(str, offset)
        , m_bNoDisplay(false)
        , m_bShowEmptyMenu(false)
        , m_bShowInlineHeader(false)
        , m_bInlineAlias(false)
        , m_bAllowInline(false)
        , m_inlineValue(4)
        , m_bDeep(false)
        , m_childCount(-1)

    {
    }

    void save(QDataStream &s) override;

    QString name() const override
    {
        return path;
    }

    void load(const QString &cfg);
    void load(QDataStream &s);

    int childCount() const;

    KServiceGroup::List entries(KServiceGroup *group, bool sort, bool excludeNoDisplay, bool allowSeparators, bool sortByGenericName);
    /**
     * This function parse attributes into menu
     */
    void parseAttribute(const QString &item, bool &showEmptyMenu, bool &showInline, bool &showInlineHeader, bool &showInlineAlias, int &inlineValue);

    bool m_bNoDisplay : 1;
    bool m_bShowEmptyMenu : 1;
    bool m_bShowInlineHeader : 1;
    bool m_bInlineAlias : 1;
    bool m_bAllowInline : 1;
    int m_inlineValue;
    QStringList suppressGenericNames;
    QString directoryEntryPath;
    QStringList sortOrder;
    QString m_strCaption;
    QString m_strIcon;
    QString m_strComment;

    KServiceGroup::List m_serviceList;
    bool m_bDeep;
    QString m_strBaseGroupName;
    mutable int m_childCount;
};

class KServiceSeparator : public KSycocaEntry // krazy:exclude=dpointer (dummy class)
{
public:
    typedef QExplicitlySharedDataPointer<KServiceSeparator> Ptr;

public:
    /**
     * Construct a service separator
     */
    KServiceSeparator();
    ~KServiceSeparator() override;
};

#endif
