/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include <QAbstractItemView>
#include <explorer-qt/volume-manager.h>

class ComputerProxyModel;
class QRubberBand;

class ComputerView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit ComputerView(QWidget *parent = nullptr);

    bool eventFilter(QObject *object, QEvent *event);

    virtual QRect visualRect(const QModelIndex &index) const;
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    virtual QModelIndex indexAt(const QPoint &point) const;

    virtual QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers);

    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;

    virtual bool isIndexHidden(const QModelIndex &index) const;

    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const;

    void setModel(QAbstractItemModel *model);

    QString tryGetVolumeUriFromMountTarget(const QString &mountTargetUri);
    QString tryGetVolumeRealUriFromUri(const QString &uri);

    void refresh();
    bool getRightDoubleClickState();
    void setRightDoubleClickState(bool flag);

Q_SIGNALS:
    void updateLocationRequest(const QString &uri);

protected:
    void updateEditorGeometries();
    void updateGeometries();
    void resizeEvent(QResizeEvent *event);

    void paintEvent(QPaintEvent *e);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

    void layoutVolumeIndexes(const QModelIndex &volumeParentIndex);
    void layoutRemoteIndexes(const QModelIndex &remoteParentIndex);
    void layoutNetworkIndexes(const QModelIndex &networkParentIndex);

    void adjustLayout();

protected:
    void doLayout();

public:
    QTimer *m_touch_active_timer = nullptr;

private:
    ComputerProxyModel *m_model;
    QRubberBand *m_rubberBand;
    QPoint m_lastPressedPoint;
    QPoint m_lastPressedLogicPoint;
    QRect m_logicRect;
    bool m_isLeftButtonPressed = false;
    bool m_isRightButonDClick = false;
    bool m_isShowNetwork = true;

    QModelIndex m_hoverIndex;

    int m_scrollStep = 100;

    int m_totalHeight = 0;
    int m_totalWidth = 0;
    int m_hSpacing = 20;
    int m_vSpacing = 20;
    int m_tabPadding = 26;

    QSize m_volumeItemFixedSize = QSize(256, 108);
    QSize m_remoteItemFixedSize = QSize(108, 144);
    QSize m_networkItemFixedSize = QSize(108, 144);

    QHash<QModelIndex, QRect> m_rect_cache;

    QTimer *m_timer = nullptr;

};

#endif // COMPUTERVIEW_H
