// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLIPBOARDITEM_H
#define CLIPBOARDITEM_H

#include "commoniconbutton.h"
#include "widget/tipswidget.h"

#include <QWidget>

class ClipboardItem : public QWidget
{
    Q_OBJECT

public:
    explicit ClipboardItem(QWidget *parent = nullptr);

    QWidget *tipsWidget();
    const QString contextMenu() const;
    void invokeMenuItem(const QString menuId, const bool checked);
    void refreshIcon();

protected:
    void resizeEvent(QResizeEvent *e) override;

signals:
    void requestHideApplet();

private:
    void init();

private:
    TipsWidget *m_tipsLabel;
    CommonIconButton *m_icon;
    QPixmap m_iconPixmap;
};

#endif // CLIPBOARDITEM_H
