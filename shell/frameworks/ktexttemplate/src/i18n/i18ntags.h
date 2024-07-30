/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef I18N_TAGS_H
#define I18N_TAGS_H

#include "taglibraryinterface.h"

#include "i18n.h"
#include "i18nc.h"
#include "i18ncp.h"
#include "i18np.h"
#include "l10n_filesize.h"
#include "l10n_money.h"
#include "with_locale.h"

using namespace KTextTemplate;

class I18nTagLibrary : public QObject, public TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextTemplate::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.kde.KTextTemplate.TagLibraryInterface")
public:
    I18nTagLibrary()
    {
    }

    QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = {}) override
    {
        Q_UNUSED(name);

        QHash<QString, AbstractNodeFactory *> nodeFactories;
        nodeFactories.insert(QStringLiteral("i18n"), new I18nNodeFactory());
        nodeFactories.insert(QStringLiteral("i18n_var"), new I18nVarNodeFactory());
        nodeFactories.insert(QStringLiteral("i18nc"), new I18ncNodeFactory());
        nodeFactories.insert(QStringLiteral("i18nc_var"), new I18ncVarNodeFactory());
        nodeFactories.insert(QStringLiteral("i18np"), new I18npNodeFactory());
        nodeFactories.insert(QStringLiteral("i18np_var"), new I18npVarNodeFactory());
        nodeFactories.insert(QStringLiteral("i18ncp"), new I18ncpNodeFactory());
        nodeFactories.insert(QStringLiteral("i18ncp_var"), new I18ncpVarNodeFactory());
        nodeFactories.insert(QStringLiteral("l10n_money"), new L10nMoneyNodeFactory());
        nodeFactories.insert(QStringLiteral("l10n_money_var"), new L10nMoneyVarNodeFactory());
        nodeFactories.insert(QStringLiteral("l10n_filesize"), new L10nFileSizeNodeFactory());
        nodeFactories.insert(QStringLiteral("l10n_filesize_var"), new L10nFileSizeVarNodeFactory());
        nodeFactories.insert(QStringLiteral("with_locale"), new WithLocaleNodeFactory());
        // TODO: Compat and block tags
        return nodeFactories;
    }
};

#endif
