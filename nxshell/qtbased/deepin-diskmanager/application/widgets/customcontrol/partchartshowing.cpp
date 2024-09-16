// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "partchartshowing.h"

#include <DFontSizeManager>

#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>

#define RADIUS 8
#define SPACE 11
#define RIGHTSPACE 3

PartChartShowing::PartChartShowing(QWidget *parent)
    : QWidget(parent)
{
    QColor color("#70BEFF"); //浅蓝色
    QColor color1("#4293FF"); //深蓝色
    QColor color2("#6751E4"); //紫色
    QColor color3("#FA7404"); //橙色
    QColor color4("#FFD027"); //黄色
    QColor color5("#2CCBBE"); //绿色
    m_baseColor = QVector<QColor> {color, color1, color2, color3, color4, color5};
    setMouseTracking(true);
    m_showTipTimer = new QTimer(this);
    connect(m_showTipTimer, &QTimer::timeout, this, &PartChartShowing::showTipTimerSlot);
}

void PartChartShowing::transInfos(const double &totals, const QVector<double> sizeInfo)
{
    m_total = totals;
    m_partSize = sizeInfo;
}

void PartChartShowing::transFlag(int flag, int value)
{
    m_flag = flag;
    m_sumValue = value;
    update();
}

void PartChartShowing::showTipTimerSlot()
{
    //    qDebug() << hover;
    emit sendMoveFlag(m_hover, m_number2, m_posX);
    m_showTipTimer->stop();
}

void PartChartShowing::paintEvent(QPaintEvent *event)
{
    //绘制整个分区的空闲空间以及文字颜色
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    QRect rect;
    rect.setX(x());
    rect.setY(y());
    rect.setWidth(width());
    QRect paintRect = QRect(0, 10, rect.width(), 35);
    QPainterPath paintPath;

    paintPath.moveTo(paintRect.bottomRight() - QPoint(0, RADIUS));
    paintPath.lineTo(paintRect.topRight() + QPoint(0, RADIUS));
    paintPath.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(RADIUS * 2, 0)),
                          QSize(RADIUS * 2, RADIUS * 2)),
                    0, 90);
    paintPath.lineTo(paintRect.topLeft() + QPoint(RADIUS, 0));
    paintPath.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(RADIUS * 2, RADIUS * 2)), 90, 90);
    paintPath.lineTo(paintRect.bottomLeft() - QPoint(0, RADIUS));
    paintPath.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, RADIUS * 2)),
                          QSize(RADIUS * 2, RADIUS * 2)),
                    180, 90);
    paintPath.lineTo(paintRect.bottomRight() - QPoint(RADIUS, 0));
    paintPath.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(RADIUS * 2, RADIUS * 2)),
                          QSize(RADIUS * 2, RADIUS * 2)),
                    270, 90);

    QColor fillColor = QColor(palette().dark().color()); //this->palette().color(DPalette::Normal, DPalette::Highlight)
    painter.setBrush(QBrush(fillColor));
    painter.fillPath(paintPath, fillColor);
    painter.setPen(QPen(QColor(fillColor), 3));
    painter.drawLine(paintRect.topLeft().x() + RADIUS, paintRect.topLeft().y() + RADIUS, paintRect.topLeft().x() + RADIUS + paintRect.width(), paintRect.bottomLeft().y() - RADIUS);
    QColor tipColor = QColor(palette().dark().color());

    painter.setPen(tipColor);
    int height = 58 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
    painter.drawRect(x(), y() + height, 10, 10);

    painter.setPen(palette().text().color());
    QRect textRect(x() + 15, y() + 52, 150, 50);

    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
    font.setWeight(QFont::Medium);
    painter.setFont(font);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        painter.setPen(QColor("#001A2E"));
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        painter.setPen(QColor("#C0C0C0"));
    }
    painter.drawText(textRect, tr("Unallocated"));

    QWidget::paintEvent(event);

    addPaint(&painter);
    painter.restore();
}

void PartChartShowing::addPaint(QPainter *painter)
{
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);
    QRect rect;
    rect.setX(x());
    rect.setY(y());
    rect.setWidth(width());
    QRect paintRect = QRect(3, 10, rect.width() - 4, 35);
    QVector<QPainterPath> path;
    for (int i = 0; i < m_partSize.size(); i++) {
        QPainterPath painterPath;
        path.append(painterPath);
    }

    QPainterPath paintPath;
    double sum = 0.00;
    //绘制默认选中状态
    if (m_flag == 1) {
        painter->setPen(QPen(QColor(palette().highlight().color()), 2));
        painter->drawRoundedRect(paintRect, 8, 8);
    }

    for (int i = 0; i < m_partSize.size(); i++) {
        double widths = (m_partSize.at(i) / m_total) * (paintRect.width() - RADIUS);
        double width1 = 0.00;
        widths = widths - RIGHTSPACE;
        if (m_partSize.at(i) / m_total < 0.01 || widths < RADIUS) {
            widths = 8;
        }
        sum = sum + m_partSize.at(i);
        m_sums = sum;
        //i=0,绘制第一个分区，判断ｎｕｍ绘制选中状态
        if (i == 0) {
            if (static_cast<int>(sum) < static_cast<int>(m_total)) {
                path[0].moveTo(paintRect.topLeft() + QPoint(RADIUS, 0));
                path[0].arcTo(QRect(QPoint(paintRect.topLeft()), QSize(RADIUS * 2, RADIUS * 2)), 90, 90);
                path[0].lineTo(paintRect.bottomLeft() - QPoint(0, RADIUS));
                path[0].arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, RADIUS * 2)),
                                    QSize(RADIUS * 2, RADIUS * 2)),
                              180, 90);
                path[0].lineTo(paintRect.bottomLeft() + QPoint((static_cast<int>(widths) + RADIUS), 0));
                path[0].lineTo(paintRect.topLeft() + QPoint((static_cast<int>(widths) + RADIUS), 0));
                path[0].lineTo(paintRect.topLeft() + QPoint(RADIUS, 0));
            }
            //            qDebug() << path[0].currentPosition().x();
            if (static_cast<int>(sum) >= static_cast<int>(m_total)) {
                path[0].moveTo(paintRect.bottomRight() - QPoint(0, RADIUS));
                path[0].lineTo(paintRect.topRight() + QPoint(0, RADIUS));
                path[0].arcTo(QRect(QPoint(paintRect.topRight() - QPoint(RADIUS * 2, 0)),
                                    QSize(RADIUS * 2, RADIUS * 2)),
                              0, 90);
                path[0].lineTo(paintRect.topLeft() + QPoint(RADIUS, 0));
                path[0].arcTo(QRect(QPoint(paintRect.topLeft()), QSize(RADIUS * 2, RADIUS * 2)), 90, 90);
                path[0].lineTo(paintRect.bottomLeft() - QPoint(0, RADIUS));
                path[0].arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, RADIUS * 2)),
                                    QSize(RADIUS * 2, RADIUS * 2)),
                              180, 90);
                path[0].lineTo(paintRect.bottomRight() - QPoint(RADIUS, 0));
                path[0].arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(RADIUS * 2, RADIUS * 2)),
                                    QSize(RADIUS * 2, RADIUS * 2)),
                              270, 90);
            }

            painter->fillPath(path[0], QBrush(m_baseColor.at(0)));
            if (m_number == 0) {
                if ((static_cast<int>(widths) == 8) && m_partSize.size() == 2) {
                    //                    qDebug() << (static_cast<int>(widths) - static_cast<int>((8 - widths)));
                    QPainterPath seclect1path;
                    seclect1path.moveTo(paintRect.topLeft() + QPoint(RADIUS, 0));
                    seclect1path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(RADIUS * 2, RADIUS * 2)), 90, 90);
                    seclect1path.lineTo(paintRect.bottomLeft() - QPoint(0, RADIUS));
                    seclect1path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, RADIUS * 2)),
                                             QSize(RADIUS * 2, RADIUS * 2)),
                                       180, 90);
                    seclect1path.lineTo(paintRect.bottomLeft() + QPoint((static_cast<int>(widths) - 3 + RADIUS), 0));
                    seclect1path.lineTo(paintRect.topLeft() + QPoint((static_cast<int>(widths) - 3 + RADIUS), 0));
                    seclect1path.lineTo(paintRect.topLeft() + QPoint(RADIUS, 0));
                    painter->setBrush(QBrush(m_baseColor.at(0)));
                    painter->setPen(QPen(this->palette().color(DPalette::Normal, DPalette::Highlight), 2));
                    painter->drawPath(seclect1path);
                } else {
                    painter->setBrush(QBrush(m_baseColor.at(0)));
                    painter->setPen(QPen(this->palette().color(DPalette::Normal, DPalette::Highlight), 2));
                    painter->drawPath(path[0]);
                }

                m_number = -1;
                m_flag = 0;
            }

        } else if (sum + 1 < m_total && i > 0) { //绘制除了第一个分区和最后一个分区以及空闲分区的填充和选中状态
            //            qDebug() << static_cast<int>(sum) << static_cast<int>(total);
            width1 = (m_partSize.at(i - 1) / m_total) * (paintRect.width() - RADIUS);
            width1 = width1 - RIGHTSPACE;
            if (width1 < 8 || m_partSize.at(i - 1) / m_total < 0.01) {
                width1 = 8;
            }
            //未避免设置最小宽度带来的图形溢出,在所画图形宽度即将到圆角时,不予新增分区
            if (path[i - 1].currentPosition().x() + width1 + widths > paintRect.width() - 2 * RADIUS) {
                emit judgeLastPartition();
            }
            path[i].moveTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1), 0));
            path[i].lineTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1 + widths), 0));
            path[i].lineTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1 + widths), paintRect.height() - 1));
            path[i].lineTo(path[i - 1].currentPosition() + QPoint((static_cast<int>(width1)), paintRect.height() - 1));
            path[i].lineTo(path[i - 1].currentPosition() + QPoint((static_cast<int>(width1)), 0));
            QColor fillColor;
            if (i > m_baseColor.size() - 1) {
                fillColor = m_baseColor.at(i % (m_baseColor.size() - 1));
            } else {
                fillColor = m_baseColor.at(i);
            }
            painter->fillPath(path[i], QBrush(fillColor));
            //            if (m_number > 0 && m_number != -1 && m_number == i) {
            if (m_number > 0 && m_number == i) {
                painter->setBrush(fillColor);
                painter->setPen(QPen(this->palette().color(DPalette::Normal, DPalette::Highlight), 2));
                if (width1 < 8) {
                    width1 = 8;
                }
                if (i == m_partSize.size() - 1) {
                    painter->drawRect(static_cast<int>(path[m_number - 1].currentPosition().x() + width1), static_cast<int>(path[m_number - 1].currentPosition().y()), static_cast<int>(widths), paintRect.height() - 1);
                } else {
                    painter->drawRect(static_cast<int>(path[m_number - 1].currentPosition().x() + width1), static_cast<int>(path[m_number - 1].currentPosition().y()), static_cast<int>(widths - 1), paintRect.height() - 1);
                }
                m_flag = 0;
            }
        } else if (m_sumValue >= 100) { //绘制最后一个分区当超过整个分区容量的时候以及选中状态
            //            qDebug() << "sumvalue" << sumvalue;
            double width = ((m_partSize.at(m_partSize.size() - 2) / m_total)) * (paintRect.width() - RADIUS) - RIGHTSPACE;
            if (m_partSize.at(m_partSize.size() - 2) / m_total < 0.01) {
                width = 8;
            }
            path[m_partSize.size() - 1].moveTo(path[m_partSize.size() - 2].currentPosition() + QPoint(static_cast<int>((m_partSize.at(m_partSize.size() - 2) / m_total) * (paintRect.width() - RADIUS)) - 1, 0));
            path[m_partSize.size() - 1].lineTo(paintRect.topRight() - QPoint(RADIUS, 0));
            path[m_partSize.size() - 1].arcTo(QRect(QPoint(paintRect.topRight() - QPoint(RADIUS * 2, 0)),
                                                    QSize(RADIUS * 2, RADIUS * 2)),
                                              90, -90);
            path[m_partSize.size() - 1].lineTo(paintRect.bottomRight() - QPoint(0, RADIUS));

            path[m_partSize.size() - 1].arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(RADIUS * 2, RADIUS * 2)),
                                                    QSize(RADIUS * 2, RADIUS * 2)),
                                              0, -90);
            path[m_partSize.size() - 1].lineTo(path[m_partSize.size() - 2].currentPosition() + QPoint(static_cast<int>(width), paintRect.height() - 1));
            path[m_partSize.size() - 1].lineTo(path[m_partSize.size() - 2].currentPosition() + QPoint(static_cast<int>(width), 0));
            QColor fillColor;
            if (i > m_baseColor.size() - 1) {
                fillColor = m_baseColor.at(i % (m_baseColor.size() - 1));
            } else {
                fillColor = m_baseColor.at(i);
            }
            painter->fillPath(path[m_partSize.size() - 1], QBrush(fillColor));
            if (m_number == m_partSize.size() - 1) {
                painter->setPen(QPen(this->palette().color(DPalette::Normal, DPalette::Highlight), 2));
                painter->setBrush(fillColor);
                QPainterPath selectPath;
                if (static_cast<int>((m_partSize.at(m_partSize.size() - 2) / m_total) * (paintRect.width() - RADIUS)) - 1 <= 8) {
                    //                    qDebug() << 8 - (static_cast<int>((partsize.at(partsize.size() - 2) / total) * (paintRect.width() - RADIUS)) - 1);
                    if (static_cast<int>((m_partSize.at(m_partSize.size() - 2) / m_total) * (paintRect.width() - RADIUS)) - 1 == 7) {
                        selectPath.moveTo(path[m_partSize.size() - 2].currentPosition() + QPoint(static_cast<int>((m_partSize.at(m_partSize.size() - 2) / m_total) * (paintRect.width() - RADIUS)) + (8 - (static_cast<int>((m_partSize.at(m_partSize.size() - 2) / m_total) * (paintRect.width() - RADIUS)) + 1)), 0));
                    } else {
                        selectPath.moveTo(path[m_partSize.size() - 2].currentPosition() + QPoint(static_cast<int>((m_partSize.at(m_partSize.size() - 2) / m_total) * (paintRect.width() - RADIUS)) + (8 - (static_cast<int>((m_partSize.at(m_partSize.size() - 2) / m_total) * (paintRect.width() - RADIUS)) - 1)), 0));
                    }
                    selectPath.lineTo(paintRect.topRight() - QPoint(RADIUS, 0));
                    selectPath.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(RADIUS * 2, 0)),
                                           QSize(RADIUS * 2, RADIUS * 2)),
                                     90, -90);
                    selectPath.lineTo(paintRect.bottomRight() - QPoint(0, RADIUS));

                    selectPath.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(RADIUS * 2, RADIUS * 2)),
                                           QSize(RADIUS * 2, RADIUS * 2)),
                                     0, -90);
                    selectPath.lineTo(path[m_partSize.size() - 2].currentPosition() + QPoint(static_cast<int>(width), paintRect.height() - 1));
                    selectPath.lineTo(path[m_partSize.size() - 2].currentPosition() + QPoint(static_cast<int>(width), 0));
                    painter->drawPath(selectPath);
                } else {
                    painter->drawPath(path[m_partSize.size() - 1]);
                }
                m_number = -1;
            }
        }
    }
    //绘制空闲分区以及选中状态
    if ((m_flag == 3 && sum < m_total)) {
        double width2 = (m_partSize.at(m_partSize.size() - 1) / m_total) * (paintRect.width() - RADIUS) - RIGHTSPACE;
        if (m_partSize.at(m_partSize.size() - 1) / m_total < 0.01) {
            width2 = 8;
        }
        painter->setPen(QPen(palette().color(DPalette::Normal, DPalette::Highlight), 2));
        paintPath.moveTo(paintRect.bottomRight() - QPoint(0, RADIUS));
        paintPath.lineTo(paintRect.topRight() + QPoint(0, RADIUS));
        paintPath.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(RADIUS * 2, 0)),
                              QSize(RADIUS * 2, RADIUS * 2)),
                        0, 90);
        paintPath.lineTo(QPoint(static_cast<int>(m_allPath[m_partSize.size() - 1].currentPosition().x()) + static_cast<int>(width2), 10));
        paintPath.lineTo(QPoint(static_cast<int>(m_allPath[m_partSize.size() - 1].currentPosition().x()) + static_cast<int>(width2), paintRect.height() + 9));
        paintPath.lineTo(paintRect.bottomRight() - QPoint(RADIUS, 0));
        paintPath.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(RADIUS * 2, RADIUS * 2)),
                              QSize(RADIUS * 2, RADIUS * 2)),
                        270, 90);
        painter->drawPath(paintPath);
        m_flag = 0;
    }
    m_allPath = path;
}

void PartChartShowing::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    int x = event->pos().x();
    int y = event->pos().y();
    if (m_partSize.size() == 0) {
        //判断第一个分区的鼠标点击
        if (x > 0 && x < width() && y > 10 && y < 45) {
            m_flag = 1;
            update();
        }
    } else if (m_partSize.size() > 0) {
        for (int i = 0; i < m_partSize.size(); i++) {
            double width = ((m_partSize.at(i) / m_total) * (this->width() - SPACE)) - RIGHTSPACE;
            if (m_partSize.at(i) / m_total < 0.01) {
                width = 8;
            }
            //判断除了第一个和最后一个的鼠标点击
            if ((x > m_allPath[i].currentPosition().x() && x < (m_allPath[i].currentPosition().x() + width) && y > 10 && y < 45) || (m_partSize.size() == 1 && y > 10 && y < 40 && (m_sumValue >= 100 || m_sums >= m_total - 0.01))) {
                m_flag = 2;
                m_number = i;
                update();
            }
            //鼠标点击判断最后一个分区点击
            if (x > m_allPath[m_partSize.size() - 1].currentPosition().x() && y > 10 && y < 45 && i == m_partSize.size() - 1 && (m_sumValue >= 100 || m_sums >= m_total)) {
                m_flag = 2;
                m_number = i;
                update();
            }
        }
        //            qDebug() << flag;
        //空闲分区
        double width1 = ((m_partSize.at(m_partSize.size() - 1) / m_total) * (width() - SPACE)) - RIGHTSPACE;
        if (m_partSize.size() > 1) {
            if (m_partSize.at(m_partSize.size() - 1) / m_total < 0.01) {
                width1 = 8;
            }
        }
        //判断空闲分区鼠标点击
        if (x > m_allPath[m_partSize.size() - 1].currentPosition().x() + width1 && y > 10 && y < 45 && int(m_sums) < int(m_total)) {
            m_flag = 3;
            //                i = -1;
            m_number = -1;
            update();
        }
    }
    //判断在分区条之外的鼠标点击
    if (!(x > 0 && x < width() && y > 10 && y < 45)) {
        m_flag = 0;
        m_number = -1;
        update();
    }

    emit sendFlag(m_flag, m_number, x);
}

void PartChartShowing::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    m_posX = x;
    //判断整个分区的鼠标悬浮
    if (m_partSize.size() == 0) {
        if (x > 0 && x < width() && y > 10 && y < 45) {
            m_number2 = -1;
            m_showTipFlag++;
            if (m_showTipFlag == 1) {
                m_hover = 1;
                m_showTipTimer->start(200);
            }
            m_showTipFlag = -2;
        }

    } else if (m_partSize.size() > 0) { //判断除了第一个分区的鼠标悬浮
        int number = 0;
        for (int i = 0; i < m_partSize.size(); i++) {
            double width = ((m_partSize.at(i) / m_total) * (this->width() - SPACE)) - RIGHTSPACE;
            if (m_partSize.at(i) / m_total < 0.01) {
                width = 8;
            }
            if ((x > m_allPath[i].currentPosition().x() && x < (m_allPath[i].currentPosition().x() + width) && y > 10 && y < 40) || (m_partSize.size() == 1 && y > 10 && y < 40 && (m_sumValue >= 100 || m_sums >= m_total))) {
                m_number2 = i;
                m_showTipFlag++;

                if (m_showTipFlag == 1 || m_isShowFlag == true || number != i) {
                    m_hover = 2;
                    m_showTipTimer->start(200);
                }
                m_showTipFlag = -2;
                m_isShowFlag = false;
                number = i;
            }
        }
        //空闲分区
        double width = ((m_partSize.at(m_partSize.size() - 1) / m_total) * (this->width() - SPACE)) - RIGHTSPACE;
        if (m_partSize.size() > 1) {
            if (m_partSize.at(m_partSize.size() - 1) / m_total < 0.01) {
                width = 8;
            }
        }
        if (x > m_allPath[m_partSize.size() - 1].currentPosition().x() + width && y > 10 && y < 45 && int(m_sums) < int(m_total)) {
            m_number2 = -1;
            m_showTipFlag++;
            if (m_showTipFlag == 1 || m_isShowFlag == false) {
                m_hover = 3;
                m_showTipTimer->start(200);
            }
            m_showTipFlag = -2;
            m_isShowFlag = true;
        }
    }
    if (!(x > 0 && x < width() && y > 10 && y < 45)) {
        m_showTipFlag = 0;
    }
}
