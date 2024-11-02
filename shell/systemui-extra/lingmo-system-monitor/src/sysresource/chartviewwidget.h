#ifndef CHARTVIEWWIDGET_H
#define CHARTVIEWWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QPainterPath>
#include <QPainter>

#include "../controls/kchartview.h"
#include "commoninfo.h"

class ChartViewWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitleStyle {
        TITLE_HORIZONTAL = 0,// 单个模式
        TITLE_VERTICAL = 1   // 多个模式
    };
    explicit ChartViewWidget(QString name, QWidget *parent = nullptr);
    ~ChartViewWidget();

    void setMode(int mode);
    void setActive(bool active);
    QString getNCName();

private:
    int m_mode = TITLE_HORIZONTAL;
    bool m_isActive = false;
    QFont m_font;
    QString m_ifname;   // 网卡名称
    QString m_recv_bps; // 接受速率
    QString m_sent_bps; // 发送速率
    QColor m_recvColor {"#B37FEB"};
    QColor m_sentColor {"#FF7875"};
    KChartView *netChartView = nullptr;

    QList<qreal> m_listData1;
    QList<qreal> m_listData2;
    qreal m_maxData1 = 1;
    qreal m_maxData2 = 1;
    qreal m_maxData;

    void updateWidgetGeometry();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(const QString &mac);

public slots:
    void updateDate(qreal sendRate, qreal recvRate);

};

#endif // CHARTVIEWWIDGET_H
