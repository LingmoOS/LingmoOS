// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNOMEKEYRINGEXTENTION_H
#define GNOMEKEYRINGEXTENTION_H

#include <QObject>
#include <QButtonGroup>
#include <QCheckBox>
#include <QTranslator>

#include <dpa/agent-extension-proxy.h>
#include <dpa/agent-extension.h>

class  GnomeKeyringExtention : public QObject, dpa::AgentExtension
{
    Q_OBJECT
    Q_INTERFACES(dpa::AgentExtension)
    Q_PLUGIN_METADATA(IID AgentExtensionPluginIID FILE "dpa-ext-gnomekeyring.json")
public:
    explicit GnomeKeyringExtention(QObject * parent = 0);

    void initialize(dpa::AgentExtensionProxy *proxy) Q_DECL_OVERRIDE;
    void finalize() Q_DECL_OVERRIDE;

    QStringList interestedActions() const Q_DECL_OVERRIDE;
    QString description() const Q_DECL_OVERRIDE;

    QButtonGroup* options() Q_DECL_OVERRIDE;

    void extendedDo() Q_DECL_OVERRIDE;

private:
    dpa::AgentExtensionProxy *m_proxy;

    QPointer<QCheckBox> m_checkBtn;
    QPointer<QTranslator> m_translator;

    void emptyKeyringPassword(const QString &password);
    void restoreKeyringPassword(const QString &password);

    void setKeyringPassword(const QString current, const QString newPass);
};

#endif // GNOMEKEYRINGEXTENTION_H
