/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef ICONVIEWDELEGATE_H
#define ICONVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <explor-core_global.h>
#include <QFileSystemWatcher>

class QPushButton;

namespace Peony {

class DesktopIconViewDelegate;
class DesktopIndexWidget;

namespace DirectoryView {

class IconView;
class IconViewIndexWidget;
class IconViewTextHelper;

class IconViewDelegate : public QStyledItemDelegate
{
    friend class IconViewIndexWidget;

    Q_OBJECT
public:
    explicit IconViewDelegate(QObject *parent = nullptr);
    ~IconViewDelegate() override;
    IconView *getView() const;

    void setMaxLineCount(int count = 0);
    const QBrush selectedBrush() const;

   //初始化option
    void initIndexOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const;
    void setStartDrag(bool isDrag);
    void setSearchKeyword(QString regFindKeyWords);

    const QString getRegFindKeyWords() const;

Q_SIGNALS:
    void isEditing(bool editing) const;
    void requestDone(QWidget *editor);
    void updateIndexWidget(const QStyleOptionViewItem &option) const;

public Q_SLOTS:
    void setCutFiles(const QModelIndexList &indexes);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    //edit
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void setIndexWidget(const QModelIndex &index, QWidget *widget) const;

private Q_SLOT:
    void slot_finishEdit();/* 编辑完成 */

private:
    QModelIndexList m_cut_indexes;

    QModelIndex m_index_widget_index;
    QWidget *m_index_widget;

    QPushButton *m_styled_button;
    bool m_isStartDrag = false; ;  //是否是拖拽item

    QString m_regFindKeyWords;
    QFileSystemWatcher *m_watcher = nullptr;
};

class PEONYCORESHARED_EXPORT IconViewTextHelper
{
    friend class IconViewDelegate;
    friend class IconViewIndexWidget;
    friend class Peony::DesktopIndexWidget;
    friend class Peony::DesktopIconViewDelegate;
    /*!
     * \brief getTextRectForIndex
     * \return the adjusted text rect to be painted.
     */
    static QSize getTextSizeForIndex(const QStyleOptionViewItem &option,
                                     const QModelIndex &index,
                                     int horizalMargin = 0,
                                     int maxLineCount = 4);
    static void paintText(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index,
                          int textMaxHeight,
                          int horizalMargin = 0,
                          int maxLineCount = 4, bool useSystemPalette = true, const QColor &customColor = Qt::transparent);

    static void paintText(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          int textMaxHeight,
                          int xOffset,
                          const QString &regFindKeyWords,
                          int horizalMargin = 0,
                          int maxLineCount = 4) ;

    static qreal drawText(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          int textMaxHeight,
                          int xOffset,
                          const QString &regFindKeyWords,
                          int horizalMargin = 0,
                          int maxLineCount = 4) ;
};

}

}

#endif // ICONVIEWDELEGATE_H
