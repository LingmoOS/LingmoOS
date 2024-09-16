// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include "tray_module_interface.h"

class QLabel;

namespace dss {
namespace module {

class NetworkModule : public QObject
    , public TrayModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.deepin.dde.shell.Modules.Tray" FILE "network.json")
    Q_INTERFACES(dss::module::TrayModuleInterface)

public:
    explicit NetworkModule(QObject *parent = nullptr);
    ~NetworkModule() override;

    void init() override;

    inline QString key() const override { return objectName(); }
    QWidget *content() override { return m_networkWidget; }
    virtual inline QString icon() const override { return ":/img/default_avatar.svg"; }

    virtual QWidget *itemWidget() const override;
    virtual QWidget *itemTipsWidget() const override;
    virtual const QString itemContextMenu() const override;
    virtual void invokedMenuItem(const QString &menuId, const bool checked) const override;
    virtual bool isNeedInitPlugin() const { return true; }

private:
    void initUI();

private:
    QWidget *m_networkWidget;
    QLabel *m_tipLabel;
    QLabel *m_itemLabel;
};

} // namespace module
} // namespace dss
#endif // LOGIN_MODULE_H
