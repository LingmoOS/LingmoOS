/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PHABRICATORPURPOSEQUICKPLUGIN_H
#define PHABRICATORPURPOSEQUICKPLUGIN_H

#include <QQmlExtensionPlugin>

class PhabricatorPurposeQuickPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // PHABRICATORPURPOSEQUICKPLUGIN_H
