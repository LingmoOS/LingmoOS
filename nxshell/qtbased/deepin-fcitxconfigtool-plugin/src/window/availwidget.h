// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVAILWIDGET_H
#define AVAILWIDGET_H

#include <QWidget>
#include <fcitxqtinputmethoditem.h>

namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxSettingsGroup;
} // namespace widgets
} // namespace dcc_fcitx_configtool
class QVBoxLayout;

QString languageName(const QString &langCode);

class AvailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AvailWidget(QWidget *parent = nullptr);
    ~AvailWidget();
    /**
     *  获取选中item
     **/
    const FcitxQtInputMethodItem &getSeleteIm() const { return m_selectItem; }

public slots:
    /**
     *  搜索输入法
     * @param const QString&
     **/
    void onSearchIM(const QString &);
    /**
     *  更新界面
     * @param FcitxQtInputMethodItemList
     **/
    void onUpdateUI(FcitxQtInputMethodItemList);
    /**
     *  更新界面
     **/
    void onUpdateUI();
    /**
     *  移除获取选中item
     **/
    void removeSeleteIm(const FcitxQtInputMethodItem &);
    /**
     *  添加 add list
     **/
    void addSeleteIm(const FcitxQtInputMethodItem &);

    /**
     *  清除item选中状态
     **/
    void clearItemStatus();
signals:
    /**
     *  选中状态信号
     * @param true 选中 false 未选中
     **/
    void seleteIM(bool);

private:
    /**
     *  初始化界面
     **/
    void initUI();
    /**
     *  初始化信号槽
     **/
    void initConnect();
    /**
     *  清除选中,清除过滤
     **/
    void clearItemStatusAndFilter(dcc_fcitx_configtool::widgets::FcitxSettingsGroup *group, const bool &falg = false);

private:
    dcc_fcitx_configtool::widgets::FcitxSettingsGroup *m_allIMGroup {nullptr};//可用输入法列表界面
    dcc_fcitx_configtool::widgets::FcitxSettingsGroup *m_searchIMGroup {nullptr};//搜索输入法列表界面
    FcitxQtInputMethodItem m_selectItem; //选中输入法
    FcitxQtInputMethodItemList m_allAvaiIMlList; //可用输入法列表
    QString m_searchStr; //当前搜索字符串
    QVBoxLayout *m_mainLayout;
};

#endif // AVAILWIDGET_H
