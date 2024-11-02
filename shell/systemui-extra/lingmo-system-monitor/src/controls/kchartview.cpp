#include "kchartview.h"
#include "../macro.h"

#include <QPainter>
#include <QPainterPath>

KChartView::KChartView(QWidget *parent)
    : QWidget(parent)
{
    m_dataMax = 100;
    setAttribute(Qt::WA_TranslucentBackground);
    m_colorBgLine = QColor("#78788050");
    initUI();
}

KChartView::~KChartView()
{
}

qreal KChartView::getCurData(int nItemIndex)
{
    if (nItemIndex >= 0 && nItemIndex < m_listChartAttr.size()) {
        if (m_listChartAttr[nItemIndex].listData.isEmpty()) {
            return 0;
        }
        return m_listChartAttr[nItemIndex].listData.at(m_listChartAttr[nItemIndex].listData.size()-1);
    }
    return 0;
}

qreal KChartView::getRealMaxData(qreal newData, int nItemIndex)
{
    qreal realMaxData = newData;
    if (nItemIndex < 0 || nItemIndex >= m_listChartAttr.size())
        return realMaxData;
    for (int n = 0; n < m_listChartAttr[nItemIndex].listData.size(); n++) {
        if (realMaxData < m_listChartAttr[nItemIndex].listData[n]) {
            realMaxData = m_listChartAttr[nItemIndex].listData[n];
        }
    }
    return realMaxData;
}

void KChartView::setMaxData(qreal qMaxData)
{
    if (qMaxData<=0)
        return;
    m_dataMax = qMaxData;
}

void KChartView::addItemAttr(KChartAttr chartAttr)
{
    QLineSeries* upLineSeries = new QLineSeries(this);
    QLineSeries* lowLineSeries = new QLineSeries(this);
    QAreaSeries* areaSeries = new QAreaSeries(this);
    areaSeries->setPen(chartAttr.colorForeground);
    QColor desColor = chartAttr.colorForeground;
    desColor.setAlpha(chartAttr.colorAlpha);
    areaSeries->setBrush(QBrush(desColor));
    areaSeries->setUpperSeries(upLineSeries);
    areaSeries->setLowerSeries(lowLineSeries);
    m_chart->addSeries(areaSeries);

    areaSeries->attachAxis(m_valueAxisX);
    areaSeries->attachAxis(m_valueAxisY);
    m_listChartAttr.append(chartAttr);
    m_listAreaSeries.append(areaSeries);
    m_listLowLineSeries.append(lowLineSeries);
    m_listUpLineSeries.append(upLineSeries);
}

void KChartView::onUpdateData(qreal fData, int nItemIndex)
{
    if (nItemIndex < 0 || nItemIndex >= m_listChartAttr.size())
        return;
    if (fData > m_dataMax) {
        fData = m_dataMax;
    }
    m_listChartAttr[nItemIndex].listData.append(fData);
    while (m_listChartAttr[nItemIndex].listData.size() > TEMP_CHARTSDATA_MAX) {
        m_listChartAttr[nItemIndex].listData.pop_front();
    }
    QList<QPointF> listUp;
    QList<QPointF> listLow;
    for (int n = 0; n < m_listChartAttr[nItemIndex].listData.size(); n++) {
        QPointF pointUp;
        QPointF pointLow;
        pointUp.setX(n);
        pointUp.setY(m_listChartAttr[nItemIndex].listData[n]*100.0/m_dataMax);
        listUp.append(pointUp);
        pointLow.setX(n);
        pointLow.setY(0);
        listLow.append(pointLow);
    }

    m_listUpLineSeries[nItemIndex]->clear();
    m_listUpLineSeries[nItemIndex]->replace(listUp);

    m_listLowLineSeries[nItemIndex]->clear();
    m_listLowLineSeries[nItemIndex]->replace(listLow);

    m_listAreaSeries[nItemIndex]->setUpperSeries(m_listUpLineSeries[nItemIndex]);
    m_listAreaSeries[nItemIndex]->setLowerSeries(m_listLowLineSeries[nItemIndex]);
}

void KChartView::setAxisTitle(QString axisRTitle, QColor color, QString axisLTitle)
{
    m_RadioColor = color;
    m_axisLTitle = axisLTitle;
    m_axisRTitle = axisRTitle;
}

void KChartView::updateAxisRTitle(QString title)
{
    m_axisRTitle = title;
    update();
}

void KChartView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void KChartView::drawAxisText(QPainter *painter)
{
    QFont ft1;
    ft1.setPixelSize(14);

    auto palette = QPalette();
    QColor color = palette.color(QPalette::ToolTipText);
    color.setAlphaF(0.3);
    painter->setPen(color);

    painter->setFont(ft1);
    painter->drawText(0, 0, this->width(), painter->fontMetrics().height(), Qt::AlignRight | Qt::AlignVCenter, m_axisRTitle);

    QRect bottomTextRect(0, this->height() - painter->fontMetrics().height(), this->width(), painter->fontMetrics().height());
    painter->drawText(bottomTextRect, Qt::AlignRight | Qt::AlignVCenter, tr("60 seconds"));
    painter->drawText(bottomTextRect, Qt::AlignLeft | Qt::AlignVCenter, "0");

    if (m_axisLTitle != "") {
        int spacing = 10;
        int sectionSize = 6;
        const QColor &textColor = palette.color(QPalette::Text);
        painter->setPen(textColor);
        QRect titleRect(sectionSize + spacing, 0, painter->fontMetrics().width(m_axisLTitle), painter->fontMetrics().height());
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_axisLTitle);
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_RadioColor);
        painter->drawEllipse(0, titleRect.y() + qCeil((titleRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    }
}

void KChartView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    QPainterPath framePath;
    QFont ft1;
    ft1.setPixelSize(14);
    painter.setFont(ft1);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(m_colorBgLine));
    if (m_axisRTitle == "") {
        QRect desRect(1,1,this->width()-2, this->height()-2);
        framePath.addRoundedRect(desRect,4,4);
        painter.drawPath(framePath);
        int distance = 10;
        int lineCount = desRect.height()/distance;
        for (int i = 1; i < lineCount; i++) {
            painter.drawLine(desRect.x(), desRect.y() + distance * i, desRect.right(), desRect.y() + distance * i);
        }
    } else {
        QRect desRect(1,1+painter.fontMetrics().height(),this->width()-2,this->height() - 2*painter.fontMetrics().height());
        framePath.addRoundedRect(desRect,4,4);
        painter.drawPath(framePath);
        int distance = 10;
        int lineCount = desRect.height()/distance;
        for (int i = 1; i < lineCount; i++) {
            painter.drawLine(desRect.x(), desRect.y() + distance * i, desRect.right(), desRect.y() + distance * i);
        }
        drawAxisText(&painter);
        m_chartLayout->setContentsMargins(0, painter.fontMetrics().height(), 0, painter.fontMetrics().height());
    }

    painter.restore();
    QWidget::paintEvent(event);
}

void KChartView::initUI()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(3,0,0,0);
    m_mainLayout->setSpacing(0);
    m_chartLayout = new QHBoxLayout();
    m_chartLayout->setContentsMargins(0,0,0,0);
    m_chartLayout->setMargin(0);
    m_chartLayout->setSpacing(0);

    m_chart = new QChart();
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(-8,-8,-20,-8));
    m_chartView = new QChartView(this);
    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_chartView->setContentsMargins(0,0,0,0);
    m_chartLayout->addWidget(m_chartView);

    m_chart->setBackgroundVisible(false);
    m_chartView->setStyleSheet("background: transparent");

    m_valueAxisX = new QValueAxis(this);
    m_valueAxisY = new QValueAxis(this);
    m_valueAxisX->setRange(0, TEMP_CHARTSDATA_MAX);
    m_valueAxisX->setVisible(false);
   // m_valueAxisX->setReverse();
    m_valueAxisY->setRange(0, 100.0);
    m_valueAxisY->setVisible(false);

    m_chart->addAxis(m_valueAxisX, Qt::AlignBottom);
    m_chart->addAxis(m_valueAxisY, Qt::AlignLeft);

    m_mainLayout->addLayout(m_chartLayout);
    this->setLayout(m_mainLayout);
}

