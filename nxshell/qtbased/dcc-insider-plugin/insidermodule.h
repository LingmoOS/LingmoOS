// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <interface/pagemodule.h>

namespace DCC_NAMESPACE {
class ItemModule;
}

class QStandardItemModel;
class InsiderModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit InsiderModule(QObject * parent = nullptr);
    ~InsiderModule();

//    virtual QWidget *page() override;

protected:
    virtual void active() override;
    virtual void deactive() override;

private:
    void installDisplayManager(const QString packageName);
    void checkEnabledDisplayManager();
    void switchDisplayManager(bool isNew);
    void hideInputMethodSwitch(bool hide);
    void installInputMethod(const QString &packageName);
    void checkEnabledInputMethod();
    void switchInputMethod(bool isNew);
    void installDDEShell();

    // Display Manager
    QStandardItemModel * m_availableDm;
    DCC_NAMESPACE::ItemModule * m_dmList;

    // Input Method
    DCC_NAMESPACE::ItemModule * m_imTitle;
    QStandardItemModel * m_availableIM;
    DCC_NAMESPACE::ItemModule * m_imList;
    bool m_isLightdm = false;
};
