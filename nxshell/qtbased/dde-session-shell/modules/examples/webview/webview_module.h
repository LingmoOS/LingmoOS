// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include "tray_module_interface.h"
#include "webview_content.h"

namespace dss {
namespace module {

class WebviewModule : public QObject
    , public TrayModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.deepin.dde.shell.Modules.Tray" FILE "webview.json")
    Q_INTERFACES(dss::module::TrayModuleInterface)

public:
    explicit WebviewModule(QObject *parent = nullptr);
    ~WebviewModule() override;

    void init() override;

    inline QString key() const override { return objectName(); }
    QWidget *content() override { return m_webviewContent; }
    inline QString icon() const override { return ":/img/deepin-system-monitor.svg"; }

private:
    void initUI();

private:
    WebviewContent *m_webviewContent;
};

} // namespace module
} // namespace dss
#endif // LOGIN_MODULE_H
