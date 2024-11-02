#include "chartviewwidget.h"
#include <QLabel>
#include <QHBoxLayout>

const int margin = 6;
const int spacing = 6;
const int sectionSize = 6;
const int TextSpacing = 12;
const int allDatecount = 30;

ChartViewWidget::ChartViewWidget(QString name, QWidget *parent)
    : m_ifname(name),
      QWidget(parent)
{
    QFont ft1;
    ft1.setPixelSize(14);

    m_font = ft1;

    netChartView = new KChartView(this);
    netChartView->setAxisTitle("0.0 b/s");
    KChartAttr attr;
    attr.colorForeground = m_recvColor;
    attr.colorAlpha = 154;
    KChartAttr attr1;
    attr1.colorForeground = m_sentColor;
    attr1.colorAlpha = 154;
    netChartView->addItemAttr(attr);
    netChartView->addItemAttr(attr1);

    this->updateWidgetGeometry();

}

ChartViewWidget::~ChartViewWidget()
{

}

QString ChartViewWidget::getNCName()
{
    return m_ifname;
}

void ChartViewWidget::updateDate(qreal sendRate, qreal recvRate)
{
    // 获取32次接收与发送速率中的最大值
    m_listData1 << sendRate;
    if (m_listData1.count() > allDatecount + 1) {
        m_listData1.pop_front();
    }
    qreal maxdata1 = *std::max_element(m_listData1.begin(), m_listData1.end());
    if (maxdata1 > 0 && maxdata1 != m_maxData1) {
        m_maxData1 = maxdata1 *1.1;
        m_maxData = qMax(m_maxData1, m_maxData2);
    }

    m_listData2 << recvRate;
    if (m_listData2.count() > allDatecount + 1) {
        m_listData2.pop_front();
    }
    qreal maxdata2 = *std::max_element(m_listData2.begin(), m_listData2.end());
    if (maxdata2 > 0 && maxdata2 != m_maxData2) {
        m_maxData2 = maxdata2 *1.1;
        m_maxData = qMax(m_maxData1, m_maxData2);
    }

    netChartView->updateAxisRTitle(commom::format::Format_Memory(m_maxData, KDK_KILOBYTE) + "/s");

    m_recv_bps = commom::format::Format_Memory(recvRate, KDK_KILOBYTE) + "/s";
    m_sent_bps = commom::format::Format_Memory(sendRate, KDK_KILOBYTE) + "/s";

    double resultSend = 0;
    double resultRecv = 0;
    kdkVolumeBaseNumericalConvert(sendRate, KDK_KILOBYTE, KDK_MEGABYTE, &resultSend);
    kdkVolumeBaseNumericalConvert(recvRate, KDK_KILOBYTE, KDK_MEGABYTE, &resultRecv);

    netChartView->onUpdateData(resultSend, 1);
    netChartView->onUpdateData(resultRecv, 0);

    this->update();
}

void ChartViewWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(m_font);

    const QPalette &palette = QPalette();

    const QColor &textColor = palette.color(QPalette::Text);
    if (m_isActive) {
        painter.setPen(Qt::NoPen);
        QColor selectColor = palette.color(QPalette::Highlight);
        selectColor.setAlphaF(0.1);

        painter.setBrush(selectColor);
        painter.drawRoundedRect(rect(), 8, 8);

        painter.setPen(palette.color(QPalette::Highlight));
    } else {
        painter.setPen(textColor);
    }

    QRect ifnameRect(curXMargin, margin / 2, painter.fontMetrics().width(m_ifname), painter.fontMetrics().height());
    painter.drawText(ifnameRect, Qt::AlignLeft | Qt::AlignVCenter, m_ifname);

    painter.setPen(textColor);
    if (m_mode == TITLE_HORIZONTAL) {
        QString strLeftBrackets = "(";
        QRect leftBracketsRect(ifnameRect.right() + spacing, ifnameRect.y(), painter.fontMetrics().width(strLeftBrackets), painter.fontMetrics().height());
        painter.drawText(leftBracketsRect, Qt::AlignLeft | Qt::AlignVCenter, strLeftBrackets);

        QString strRecvData = tr("Receive");
        strRecvData = strRecvData + " " + m_recv_bps;
        QRect recvRect(leftBracketsRect.right() + spacing * 2 + sectionSize, ifnameRect.y(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignLeft | Qt::AlignVCenter, strRecvData);

        QString strSentData = tr("Send");
        strSentData = strSentData + " " + m_sent_bps + " )";
        QRect sentRect(sectionSize + recvRect.right() + 3 * spacing, ifnameRect.y(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
        painter.drawText(sentRect, Qt::AlignLeft | Qt::AlignVCenter, strSentData);

        painter.setPen(Qt::NoPen);
        painter.setBrush(m_recvColor);
        painter.drawEllipse(leftBracketsRect.right() + spacing, leftBracketsRect.y() + qCeil((leftBracketsRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);

        painter.setBrush(m_sentColor);
        painter.drawEllipse(recvRect.right() + 2 * spacing, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    } else {

        QString strRecvData = tr("Receive");
        strRecvData = strRecvData + " " + m_recv_bps;
        QRect recvRect(margin + spacing + sectionSize, ifnameRect.bottom(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignLeft | Qt::AlignVCenter, strRecvData);

        QString strSentData = tr("Send");
        strSentData = strSentData  + " " + m_sent_bps;
        QRect sentRect(margin + spacing + sectionSize, recvRect.bottom(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
        painter.drawText(sentRect, Qt::AlignLeft | Qt::AlignVCenter, strSentData);

        painter.setPen(Qt::NoPen);
        painter.setBrush(m_recvColor);
        painter.drawEllipse(margin, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);

        painter.setBrush(m_sentColor);
        painter.drawEllipse(margin, sentRect.y() + qCeil((sentRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    }

}

void ChartViewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateWidgetGeometry();
}

void ChartViewWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    m_isActive = true;
    emit clicked(m_ifname);
    update();
}

void ChartViewWidget::setMode(int mode)
{
    m_mode = mode;
    this->update();
}

void ChartViewWidget::setActive(bool active)
{
    m_isActive = active;
    this->update();
}

void ChartViewWidget::updateWidgetGeometry()
{
    int fontHeight = QFontMetrics(m_font).height();
    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 :  margin;
    if (m_mode == TITLE_HORIZONTAL) {
        netChartView->setGeometry(curXMargin, TextSpacing, this->width() - 2 * curXMargin, this->height() - TextSpacing - margin);
    } else {
        netChartView->setGeometry(curXMargin, fontHeight * 2 + TextSpacing, this->width() - 2 * curXMargin, this->height() - fontHeight * 2 - TextSpacing - margin);
    }
    netChartView->update();
}
