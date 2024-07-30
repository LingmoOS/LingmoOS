/*
    SPDX-FileCopyrightText: 2007-2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGLOADERHANDLER_P_H
#define KCONFIGLOADERHANDLER_P_H

#include <QXmlStreamAttributes>

class ConfigLoaderHandler
{
public:
    ConfigLoaderHandler(KConfigLoader *config, ConfigLoaderPrivate *d);

    bool parse(QIODevice *input);

    void startElement(const QStringView localName, const QXmlStreamAttributes &attrs);
    void endElement(const QStringView localName);

private:
    void addItem();
    void resetState();

    KConfigLoader *m_config;
    ConfigLoaderPrivate *d;
    int m_min;
    int m_max;
    QString m_name;
    QString m_key;
    QString m_type;
    QString m_label;
    QString m_default;
    QString m_cdata;
    QString m_whatsThis;
    KConfigSkeleton::ItemEnum::Choice m_choice;
    QList<KConfigSkeleton::ItemEnum::Choice> m_enumChoices;
    bool m_haveMin;
    bool m_haveMax;
    bool m_inChoice;
};

#endif
