// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMSETTINGSITEM_H
#define IMSETTINGSITEM_H

#include "settingsitem.h"
#include "labels/shortenlabel.h"
#include <fcitxqtinputmethoditem.h>

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxIMSettingsItem : public FcitxSettingsItem
{
    Q_OBJECT

public:
    explicit FcitxIMSettingsItem(QString str = QString(), QFrame *parent = nullptr);
    ~FcitxIMSettingsItem();
    void setFcitxItem(const FcitxQtInputMethodItem &item);
    void setFilterStr(QString str);
    void setItemSelected(bool status);
signals:
    void itemClicked(FcitxIMSettingsItem *item);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

public:
    FcitxShortenLabel *m_labelText {nullptr};
    QLabel *m_labelIcon {nullptr};
    QHBoxLayout *m_layout {nullptr};
    FcitxQtInputMethodItem m_item;
};
} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // IMSETTINGSITEM_H
