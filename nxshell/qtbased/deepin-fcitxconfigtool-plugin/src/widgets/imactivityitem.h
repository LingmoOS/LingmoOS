// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMACTIVITYITEM_H
#define IMACTIVITYITEM_H
#include "settingsitem.h"
#include "labels/shortenlabel.h"
#include "labels/clicklabel.h"
#include <fcitxqtinputmethoditem.h>
#include <DToolButton>

using namespace dcc_fcitx_configtool::widgets;
using namespace Dtk::Widget;
namespace dcc_fcitx_configtool {
namespace widgets {

class ToolButton : public DToolButton
{
    Q_OBJECT
public:
    using DToolButton::DToolButton;

protected:
    void paintEvent(QPaintEvent *e) override;
};

class FcitxIMActivityItem : public FcitxSettingsItem
{
    Q_OBJECT
public:
    enum itemPosition{
        firstItem = 0,
        lastItem = -1,
        onlyoneItem = -2,
        otherItem = 1
    };
    FcitxIMActivityItem(FcitxQtInputMethodItem item, itemPosition index, QWidget *parent = nullptr);
    ~FcitxIMActivityItem() override;
    void editSwitch(const bool &flag);
    void setSelectStatus(const bool &flag);
    void setIndex(itemPosition i) {m_index = i;}
signals:
    void upBtnClicked(FcitxQtInputMethodItem);
    void downBtnClicked(FcitxQtInputMethodItem);
    void configBtnClicked(FcitxQtInputMethodItem);
    void deleteBtnClicked(FcitxQtInputMethodItem);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
private slots:
    void onUpItem();
    void onDownItem();
    void onConfigItem();
    void onDeleteItem();
public:
    FcitxQtInputMethodItem m_item;
    FcitxShortenLabel *m_labelText {nullptr};
    QHBoxLayout *m_layout {nullptr};
    DToolButton *m_upBtn {nullptr};
    DToolButton *m_downBtn {nullptr};
    DToolButton *m_configBtn {nullptr};
    ClickLabel *m_deleteLabel {nullptr};
    bool m_isEdit {false};
private:
    bool m_isEnter {false};
    itemPosition m_index; //0:第一个， -1：最后一个， -2：只有一个
    bool m_isPressed {false};
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // IMACTIVITYITEM_H
