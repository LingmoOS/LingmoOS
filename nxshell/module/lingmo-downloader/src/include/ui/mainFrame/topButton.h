// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file topButton.h
 *
 * @brief 主界面顶部按钮加搜索框
 *
 * @date 2020-06-09 10:00
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <DIconButton>
#include <DPushButton>
#include <DSearchEdit>
#include <DLabel>
#include <QWidget>

class SearchWidget : public DTK_WIDGET_NAMESPACE::DSearchEdit
{
    Q_OBJECT
public:
    SearchWidget(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
signals:
    void keyPressed(Qt::Key);
};

/**
 * @class TopButton
 * @brief 顶部按钮和搜索框
*/
class TopButton : public QWidget
{
    Q_OBJECT
public:
    explicit TopButton(QWidget *parent = nullptr);

    /**
     * @brief 获取查找的内容
     * @return 查找的内容
    */
    Dtk::Widget::DSearchEdit *getSearchEdit()
    {
        return m_searchEdit;
    }

    /**
     * @brief 清空查找文本框
    */
    void clearSearchText()
    {
        m_searchEdit->clear();
    }

    /**
     * @brief 启用新建任务按键
     * @param enable： 启用开关
    */
    bool enableCreateTaskBtn(bool enable)
    {
        m_newDownloadBtn->setEnabled(enable);
        return true;
    }

    /**
     * @brief 启用开始按键
     * @param enable： 启用开关
    */
    bool enableStartBtn(bool enable)
    {
        m_startDownloadBtn->setEnabled(enable);
        return true;
    }

    /**
     * @brief 启用暂停按键
     * @param enable： 启用开关
    */
    bool enablePauseBtn(bool enable)
    {
        m_pauseDownloadBtn->setEnabled(enable);
        return true;
    }

    /**
     * @brief 启用删除按键
     * @param enable： 启用开关
    */
    bool enableDeleteBtn(bool enable)
    {
        m_deleteDownloadBtn->setEnabled(enable);
        return true;
    }

    QPoint getSearchEditPosition();

public slots:
    /**
     * @brief 当前显示列表改变
     * @param index： 列表索引
    */
    void onTableChanged(int index);

private:
    /**
     * @brief 初始化界面
    */
    void Init();
    /**
     * @brief 初始化信号槽
    */
    void InitConnections();

protected:
    void mousePressEvent(QMouseEvent *event) override;
signals:
    /**
     * @brief 新建下载信号
    */
    void newDownloadBtnClicked();

    /**
     * @brief 暂停下载信号
    */
    void pauseDownloadBtnClicked();

    /**
     * @brief 开始下载信号
    */
    void startDownloadBtnClicked();

    /**
     * @brief 删除下载信号
    */
    void deleteDownloadBtnClicked();

    /**
     * @brief 搜索框焦点改变信号
    */
    void SearchEditFocus(bool focus);

    /**
     * @brief 搜索框内容改变信号
    */
    void SearchEditTextChange(QString text);

    /**
     * @brief 搜索框按键按下
    */
    void SearchEditKeyPressed(Qt::Key);

private:
    Dtk::Widget::DLabel *m_iconLable;
    Dtk::Widget::DIconButton *m_newDownloadBtn;
    Dtk::Widget::DIconButton *m_startDownloadBtn;
    Dtk::Widget::DIconButton *m_stopDownloadBtn;
    Dtk::Widget::DIconButton *m_pauseDownloadBtn;
    Dtk::Widget::DIconButton *m_deleteDownloadBtn;
    SearchWidget *m_searchEdit;
    QCompleter *m_completer;
};

#endif // TOOLBAR_H
