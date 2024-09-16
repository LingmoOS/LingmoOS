// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUAL_KEYBOARD_MODULE_H
#define VIRTUAL_KEYBOARD_MODULE_H

#include "tray_module_interface.h"

#include <QLabel>

class TipContentWidget;
class KeyboardIconWidget;

using namespace dss::module;
class VirtualKeyboardModule : public QObject, public TrayModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.deepin.dde.shell.Modules.Tray" FILE "virtualkeyboard.json")
    Q_INTERFACES(dss::module::TrayModuleInterface)

public:
    explicit VirtualKeyboardModule(QObject *parent = nullptr);
    ~VirtualKeyboardModule();

    virtual bool isNeedInitPlugin() const;
    void init() override;

    inline QString key() const override { return objectName(); }
    QWidget *content() override;
    virtual QString icon() const override;

    virtual QWidget *itemWidget() const override;
    virtual QWidget *itemTipsWidget() const override;
    virtual const QString itemContextMenu() const override;
    virtual void invokedMenuItem(const QString &menuId, const bool checked) const override;

private:
    TipContentWidget *m_tipContentWidget;
    KeyboardIconWidget *m_keyboardIconWidget;
};

#endif // VIRTUAL_KEYBOARD_MODULE_H
