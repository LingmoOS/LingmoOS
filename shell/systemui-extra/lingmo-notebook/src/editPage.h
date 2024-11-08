/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef EDIT_PAGE_H
#define EDIT_PAGE_H

#include <QWidget>
#include <QPainter>
#include <QMenu>
#include <QPainter>
#include <QColor>
#include <QModelIndex>
#include <QTextList>
#include <QPointer>
#include <QFocusEvent>
#include <QMovie>

#include "noteHead.h"
#include "noteHeadMenu.h"
#include "ui_noteHeadMenu.h"
#include "selectColorPage.h"
#include "setFontColorPage.h"
#include "setFontSizePage.h"
#include "ui_setFontSizePage.h"
#include "lingmo_settings_monitor.h"
#include "mymenu.h"

namespace Ui {
class Edit_page;
}
class Widget;

class EditPage : public QWidget
{
    Q_OBJECT

public:
    EditPage(Widget* p , int noteId, QWidget *parent = nullptr);
    ~EditPage() override;
    Ui::Edit_page *ui;
    Widget* m_notebook = nullptr;
    QColor m_editColor;
    noteHead *m_noteHead = nullptr;
    noteHeadMenu *m_noteHeadMenu = nullptr;
    SetFontSize *m_setSizePage = nullptr ;
    SetFontColor *m_setColorFontPage = nullptr ;

    int m_id;
    int m_noteId;

    bool m_isFullscreen = false;
    bool m_isInsImg = false;

    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void setHints();
    void setWindowKeepAbove();
    void setWindowStatusClear();

private:
    void initSetup();                                                // 初始配置
    void slotsSetup();                                               // 配置槽函数
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);// 字体设置
    void fontChanged(const QFont &f);                                // 当前光标下字体格式获取
    void fontColorChanged(const QColor &c);                          // 当前光标下字体颜色获取
    void initColor();

private:
    SelectColor *m_colorPage = nullptr;
    QColor m_colorNum[6];

    QPointer<QTextList> m_lastBlockList;
    QString  m_windowId;

private slots:
    void cursorPositionChangedSlot();                                // 当前光标位置改变
    void currentCharFormatChangedSlot(const QTextCharFormat &format);// 当前字体格式改变
    void textChangedSlot();                                          // 当前便签页身份id
    void setBoldSlot();                                              // 加粗
    void setItalicSlot();                                            // 斜体
    void setUnderlineSlot();                                         // 下划线
    void setStrikeOutSlot();                                         // 删除线
    void setUnorderedListSlot(bool checked);                         // 无序列表
    void setOrderedListSlot(bool checked);                           // 有序列表
    void list(bool checked, QTextListFormat::Style style);
    void setFontSizeSlot();                                          // 设置字体大小
    void setFontColorSlot(const QColor &color);                      // 设置字体颜色
    // 调色按钮
    void handleBtnSlot(const QColor& color);
    void defaultTextColorSlot();
    void showFullScreenSlot();
    void textRightMenu(QPoint);
    void textForNewEditpageSlot();
    void insertpicture();
    void dropImage(const QImage& image, const QString& format);
    void dropGifImage(const QUrl& url, const QString& filename);
    void subAnimate(int);

signals:
    void texthasChanged(int noteId, int id);
    void colorhasChanged(const QColor &color,int);
    void isEmptyNote(int noteId);
    void requestDel(int noteId);
    void textForNewEditpage();

protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void handleTabletMode(LingmoUISettingsMonitor::TabletStatus status);
    void enableMaximum();
    void disableMaximum();
    QList<QUrl> m_lstUrls;
    QHash<QMovie*,QUrl>m_hashUrls;

};

#endif // EDIT_PAGE_H
