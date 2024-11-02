#ifndef NETVIEWSCROLLAREA_H
#define NETVIEWSCROLLAREA_H

#include <QScrollArea>
#include <QMap>

#include "chartviewwidget.h"

class NetViewScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit NetViewScrollArea(QWidget *parent = nullptr);
    void updateWidgetGeometry();
    QString getCurrentNCName();

signals:
    void netifItemClicked(const QString &mac);

private:
    void showItemOnlyeOne();
    void showItemDouble();
    void showItemLgDouble();

protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void fontChanged(const QFont &font);

    void onModelUpdate();
    void onSetItemActiveStatus(const QString &mac);

private:
    bool m_initStatus = false;
    QWidget *m_centralWidget;
    QString m_currentMac = "";
    QStringList cardlist;
    QMultiMap<QString, ChartViewWidget *> m_mapItemView;
};

#endif // NETVIEWSCROLLAREA_H
