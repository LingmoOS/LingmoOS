// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSITEM_H
#define SETTINGSITEM_H

#include <DFrame>
#include <QLabel>
#include <QApplication>
#include <DStyle>
#include <QFrame>
#include <QVBoxLayout>
namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxSettingsItem : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool isErr READ isErr DESIGNABLE true SCRIPTABLE true)

public:
    explicit FcitxSettingsItem(QWidget *parent = nullptr);

    bool isErr() const;
    virtual void setIsErr(const bool err = true);

    void addBackground();
    bool isDraged() {return m_isDraged;}
    void setDraged(bool b) {m_isDraged = b;}
protected:
    void resizeEvent(QResizeEvent *event) override;
protected:
    bool m_isErr;

    DTK_WIDGET_NAMESPACE::DFrame *m_bgGroup {nullptr};
private:
    bool m_isDraged {false};
};

} // namespace widgets
} // namespace dcc

#endif // SETTINGSITEM_H
