/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#ifndef SEARCHPAGESECTION_H
#define SEARCHPAGESECTION_H
#include <QScrollArea>
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include "result-view.h"
#include "search-plugin-iface.h"
#include "best-list-view.h"

namespace LingmoUISearch {
class ResultScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    ResultScrollBar(QWidget *parent = nullptr);
    ~ResultScrollBar() = default;

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

Q_SIGNALS:
    void scrollBarAppeared();
    void scrollBarIsHideen();
};

class ResultArea : public QScrollArea
{
    Q_OBJECT
public:
    ResultArea(QWidget *parent = nullptr);
    ~ResultArea() = default;
    void appendWidet(ResultWidget *);
    void insertWidget(ResultWidget *widget, int index);
    bool removeWidget(const QString& pluginName);
    bool moveWidget(const QString& pluginName, int index);
    void setVisibleList(const QStringList &);
    void pressEnter();
    void pressDown();
    void pressUp();
    int getVScrollBarWidth();

    void setResultSelection();
    bool getSelectedState();

    void sendKeyPressSignal(QString &pluginID);

public Q_SLOTS:
    void onWidgetSizeChanged();
    void setSelectionInfo(QString &pluginID);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool viewportEvent(QEvent *event);


private:
    void initUi();
    void initConnections();
    void setupConnectionsForWidget(ResultWidget *);

    QWidget * m_widget = nullptr;
    QVBoxLayout * m_mainLyt = nullptr;
    BestListWidget * m_bestListWidget = nullptr;
    QList<ResultWidget *> m_widgetList;
    TitleLabel * m_titleLabel = nullptr;
    ResultScrollBar *m_scrollBar = nullptr;

    bool m_detail_open_state = false;
    bool m_isSelected = false;
    QString m_selectedPluginID;
    QPoint m_pressPoint;
    bool m_isClicked =false;

Q_SIGNALS:
    void startSearch(const QString &);
    void stopSearch();
    void currentRowChanged(const QString &, const SearchPluginIface::ResultInfo&);
    void keyPressChanged(const QString &, const SearchPluginIface::ResultInfo&);
    void clearSelectedRow();
    void resizeHeight(int height);
    void resizeWidth(const int &);
    void rowClicked();
    void scrollBarAppeared();
    void scrollBarIsHideen();
};

class DetailWidget : public QWidget
{
    Q_OBJECT
public:
    DetailWidget(QWidget *parent = nullptr);
    ~DetailWidget() = default;

public Q_SLOTS:
    void updateDetailPage(const QString &plugin_name, const SearchPluginIface::ResultInfo &info);
protected:
     void paintEvent(QPaintEvent *event) override;
private:
    void clearLayout(QLayout *);
    QVBoxLayout * m_mainLyt = nullptr;
    QString m_currentPluginId;
    QWidget *m_detailPage = nullptr;
    int m_radius = 8;
};

class DetailArea : public QScrollArea
{
    Q_OBJECT
public:
    DetailArea(QWidget *parent = nullptr);
    ~DetailArea() = default;
private:
    void initUi();
    DetailWidget * m_detailWidget = nullptr;

Q_SIGNALS:
    void setWidgetInfo(const QString&, const SearchPluginIface::ResultInfo&);
};
}

#endif // SEARCHPAGESECTION_H
