// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PPPOESECTION_H
#define PPPOESECTION_H

#include "abstractsection.h"

#include <networkmanagerqt/pppoesetting.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
}

class PPPOESection : public AbstractSection
{
    Q_OBJECT

public:
    explicit PPPOESection(NetworkManager::PppoeSetting::Ptr pppoeSetting, QFrame *parent = Q_NULLPTR);
    virtual ~PPPOESection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initUI();
    QString getDeviceInterface();
    virtual bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    NetworkManager::PppoeSetting::Ptr m_pppoeSetting;

    DCC_NAMESPACE::LineEditWidget *m_userName;
    DCC_NAMESPACE::LineEditWidget *m_service;
    DCC_NAMESPACE::LineEditWidget *m_password;
};

#endif /* PPPOESECTION_H */
