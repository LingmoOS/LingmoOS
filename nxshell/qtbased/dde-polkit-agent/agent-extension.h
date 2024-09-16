// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AGENTEXTENSION_H
#define AGENTEXTENSION_H

#include <QtCore>
#include <QButtonGroup>

namespace dpa {

class AgentExtensionProxy;
class AgentExtension {

public:
    /**
     * @brief initialize initializes the extension with an proxy object.
     * @param proxy is used to make contact with the application main part.
     *
     * Extensions usually fetch usefull information from the proxy.
     */
    virtual void initialize(AgentExtensionProxy *proxy) = 0;

    /**
     * @brief finalize tells the extension to do some destruction or resource release.
     */
    virtual void finalize() = 0;

    /**
     * @brief interestedActions declares interested actions that it will take effect on.
     */
    virtual QStringList interestedActions() const = 0;

    /**
     * @brief description returns what the extension basically do.
     *
     * It will show with the polkit agent dialog to tell people the potential risks.
     */
    virtual QString description() const = 0;

    /**
     * @brief options returns options left to user to choose.
     */
    virtual QButtonGroup* options() = 0;

    /**
     * @brief extendedDo is where extension actually do the dirty work.
     */
    virtual void extendedDo() = 0;
};

}

QT_BEGIN_NAMESPACE

#define AgentExtensionPluginIID "com.deepin.dpa.AgentExtension"

Q_DECLARE_INTERFACE(dpa::AgentExtension, AgentExtensionPluginIID)

QT_END_NAMESPACE

#endif // AGENTEXTENSION_H
