// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pixmaplabel.h"
#include "constants.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QTextLayout>
#include <QTextOption>
#include <QTextLine>
#include <QVBoxLayout>
#include <QFontMetrics>

PixmapLabel::PixmapLabel(QPointer<ItemData> data,QWidget *parent)
    : DLabel(parent)
    , m_istext(false)
    , m_data(data)
{

}

/*!
 */
void PixmapLabel::setText(bool is_text)
{
    m_istext = is_text; // it label is text
}

/*!
 * \~chinese \name setPixmapList
 * \~chinese \brief 设置剪切板中显示的内容
 * \~chinese \param list 存放图片的容器
 */
void PixmapLabel::setPixmapList(const QList<QPixmap> &list)
{
    m_pixmapList = list;
}

/*!
 * \~chinese \name minimumSizeHint
 * \~chinese \brief 推荐显示的最小大小(宽度为180,高度为100)
 */
QSize PixmapLabel::minimumSizeHint() const
{
    return QSize(FileIconWidth, FileIconHeight);
}

/*!
 * \~chinese \name sizeHint
 * \~chinese \brief 推荐显示的大小
 */
QSize PixmapLabel::sizeHint() const
{
    return QSize(ItemWidth - ContentMargin * 2, ItemHeight - ItemTitleHeight);
}


QPair<QString, int> PixmapLabel::getNextValidString(const QStringList &list, int from)
{
    if (from < 0 || from > list.size() - 1)
        return QPair<QString, int>("", list.size() - 1);

    for (int i = from; i < list.size(); ++i) {
        if (!list.at(i).trimmed().isEmpty()) {
            return QPair<QString, int>(list.at(i).trimmed(), i + 1);
        }
    }

    return QPair<QString, int>("", list.size() - 1);
}

void PixmapLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);

    QStyle *style = QWidget::style();
    QStyleOption opt;
    opt.initFrom(this);

    //drawPixmaps
    if (m_pixmapList.size() == 1) {
        QPixmap pix = m_pixmapList[0];
        qreal scale = Globals::GetScale(pix.size(), FileIconWidth, FileIconHeight);
        int x = int(width() - pix.size().width() / scale) / 2;
        int y = int(height() - pix.size().height() / scale) / 2;

        if (!isEnabled())
            pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
        QPixmap newPix = pix.scaled(pix.size() / scale, Qt::KeepAspectRatio);
        style->drawItemPixmap(&painter, QRect(QPoint(x, y), newPix.size()), Qt::AlignCenter, newPix);
    } else {
        for (int i = 0 ; i < m_pixmapList.size(); ++i) {
            QPixmap pix = m_pixmapList[i];
            if (pix.size() == QSize(0, 0))
                continue;
            int x = 0;
            int y = 0;
            qreal scale = Globals::GetScale(pix.size(), FileIconWidth, FileIconHeight);
            if (!(m_pixmapList.size() % 2)) {//奇数个和偶数个计算方法不一样
                x = int(width() - (pix.size().width() / scale + PixmapxStep)) / 2 + i * PixmapxStep;
                y = int(height() - (pix.size().height() / scale + PixmapyStep)) / 2 + i * PixmapyStep;
            } else {
                x = int(width() - pix.size().width() / scale) / 2 + (i - 1) * PixmapxStep;
                y = int(height() - pix.size().height() / scale) / 2 + (i - 1) * PixmapyStep;
            }

            if (!isEnabled())
                pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
            QPixmap newPix = pix.scaled(pix.size() / scale, Qt::KeepAspectRatio);
            style->drawItemPixmap(&painter, QRect(QPoint(x, y), newPix.size()), Qt::AlignCenter, newPix);
        }
    }

    //draw lines
    if (m_istext) {
        //drawText
        QStringList labelTexts = m_data->get_text();
        int lineNum = labelTexts.length() > 4 ? 4 : labelTexts.length();
        int lineHeight = (height() - TextContentTopMargin) / lineNum;
        for (int i  = 0 ; i < lineNum; ++i) {
            QPoint start(0, (i + 1)*lineHeight + TextContentTopMargin);
            QPoint end(width(), (i + 1)*lineHeight + TextContentTopMargin);
            painter.setPen(QPen(palette().color(QPalette::Shadow), 2));
            painter.drawLine(start, end);
        }

        int maxLineCount = labelTexts.length() > 4 ? 4 : labelTexts.length();
        int textIndex = 0;
        int lineFrom = 0;
        for (int rectIndex = 0; textIndex < labelTexts.length(); rectIndex++, textIndex++) {
            if (textIndex > (maxLineCount - 1)) {
                break;
            }
            QRect textRect(0, rectIndex * lineHeight + TextContentTopMargin, width(), lineHeight);
            QTextOption option;
            option.setAlignment(Qt::AlignBottom);
            option.setWrapMode(QTextOption::NoWrap);//设置文本不能换行
            painter.setPen(palette().color(QPalette::Text));

            QPair<QString, int> pair = getNextValidString(labelTexts, lineFrom);
            lineFrom = pair.second;
            QString str = pair.first.trimmed();
            if (lineFrom == maxLineCount && maxLineCount == 4) {
                str.replace(str.size() - 3, 3, "...");
            }
            if (rectIndex == (maxLineCount-1) && maxLineCount == 4) {
                QString lastStr = pair.first.trimmed();
                pair = getNextValidString(labelTexts, lineFrom);
                lineFrom = pair.second;
                lastStr += pair.first.trimmed();
                painter.drawText(textRect,fontMetrics().elidedText(lastStr,Qt::ElideRight,width()-2),option);
            }
            else {
                painter.drawText(textRect, pair.first.trimmed(), option);
            }
        }
    }
    return DLabel::paintEvent(event);
}
