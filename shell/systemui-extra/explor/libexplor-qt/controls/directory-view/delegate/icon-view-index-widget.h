/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef ICONVIEWINDEXWIDGET_H
#define ICONVIEWINDEXWIDGET_H

#include <QWidget>

#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QTextEdit>
#include <QTimer>

#include <memory>

namespace Peony {

class FileInfo;

namespace DirectoryView {

class IconViewDelegate;

class IconViewIndexWidget : public QWidget
{
    friend class IconViewDelegate;
    Q_OBJECT
public:
    explicit IconViewIndexWidget(const IconViewDelegate *delegate,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index,
                                 QWidget *parent = nullptr);

    ~IconViewIndexWidget() override;

    bool eventFilter(QObject *watched, QEvent *event) override;
    void clearModelIndex();

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

    void adjustPos();

private:
    QStyleOptionViewItem m_option;
    QModelIndex m_index;
    QTextEdit *m_edit;
    const IconViewDelegate *m_delegate;

    std::weak_ptr<FileInfo> m_info;

    QTimer m_edit_trigger;

    bool m_is_dragging = false;

    bool b_elide_text = false;
    const int ELIDE_TEXT_LENGTH = 32;
};

}

}


#endif // ICONVIEWINDEXWIDGET_H
