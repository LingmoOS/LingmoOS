// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include <DBackgroundGroup>
#include <fcitxqtinputmethoditem.h>
#include <QFrame>
#include <QTimer>
#include <QDateTime>

#include "translucentframe.h"

class QVBoxLayout;

DWIDGET_USE_NAMESPACE

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxSettingsItem;
class FcitxSettingsHeaderItem;
class FcitxSettingsHead;

class FcitxSettingsGroup : public FcitxTranslucentFrame
{
    Q_OBJECT

public:
    enum BackgroundStyle {
        ItemBackground = 0,
        GroupBackground,
        NoneBackground,
        NonSwitchMode
    };

    explicit FcitxSettingsGroup(QFrame *parent = nullptr, BackgroundStyle bgStyle = ItemBackground);
    explicit FcitxSettingsGroup(const QString &title, QFrame *parent = nullptr);
    ~FcitxSettingsGroup();

    FcitxSettingsHeaderItem *headerItem() { return m_headerItem; }
    void setHeaderVisible(const bool visible);

    FcitxSettingsItem *getItem(int index);
    void insertWidget(QWidget *widget);
    void insertItem(const int index, FcitxSettingsItem *item);
    void appendItem(FcitxSettingsItem *item);
    void appendItem(FcitxSettingsItem *item, BackgroundStyle bgStyle);
    void removeItem(FcitxSettingsItem *item);
    int indexOf(FcitxSettingsItem *item);
    void moveItem(FcitxSettingsItem *item, const int index);
    void setSpacing(const int spaceing);
    bool isPressed() {return m_isPressed;}
    void setSwitchAble(bool b){m_switchAble = b;}
    int itemCount() const;
    void clear();
    QVBoxLayout *getLayout() const { return m_layout; }
    void switchItem(int start, int end);
    void setVerticalPolicy();
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void switchPosition(FcitxQtInputMethodItem item, int end);

private:
    BackgroundStyle m_bgStyle {ItemBackground};
    QVBoxLayout *m_layout;
    FcitxSettingsHeaderItem *m_headerItem;
    FcitxSettingsItem* m_lastItem {nullptr};
    DTK_WIDGET_NAMESPACE::DBackgroundGroup *m_bggroup {nullptr};
    bool m_isPressed {false};
    int m_selectIndex {0};
    bool m_switchAble {false};
    int m_lastYPosition{0};
    QDateTime m_time;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // SETTINGSGROUP_H
