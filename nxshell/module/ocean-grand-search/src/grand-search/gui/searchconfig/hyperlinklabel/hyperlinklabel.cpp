// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hyperlinklabel.h"

#include <QDebug>
#include <QPainter>
#include <QPalette>
#include <QtMath>
#include <QLayout>
#include <QMouseEvent>
#include <QDesktopServices>

using namespace GrandSearch;

HyperlinkLabel::HyperlinkLabel(const QString &leadintText, const QString &hyperlink, const QString &endText, QWidget *parent)
    : QLabel(parent)
    , m_leadingText(leadintText)
    , m_hyperlink(hyperlink)
    , m_endText(endText)
    , m_fontMetrics(this->font())
{
    m_showText = m_leadingText + m_hyperlink + m_endText;
    m_showTextList << m_leadingText << m_hyperlink << m_endText;

    setText(m_showText);
    setWordWrap(true);
}

void HyperlinkLabel::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    m_hyperlinkRegion = QRegion();

    // 每次绘制前更新，确保使用最新字体
    m_fontMetrics = QFontMetrics(this->font());

    int curRow = 0;
    int startX = 0;
    int startY = 0;
    for (auto txt : m_showTextList) {
        QRegion region = drawText(txt, startX, startY, curRow);
        if (txt == m_hyperlink)
            m_hyperlinkRegion = region;       // 更新超链接的绘制区域
    }
}

void HyperlinkLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (m_hyperlinkRegion.contains(ev->pos())) {
        QDesktopServices::openUrl(QUrl(m_hyperlink));
    }
    return QLabel::mouseReleaseEvent(ev);
}

QRegion HyperlinkLabel::drawText(QString drawText, int &startX, int &startY, int &curRow)
{
    QRegion region;
    QSize curSize;
    QPainter p(this);

    while (!drawText.isEmpty()) {
        int tmpSize = displayLength(startX, drawText);      // 当前行能够显示的字符数量
        QString curTxt = drawText.left(tmpSize);            // 当前行准备显示的字符

        curSize = m_fontMetrics.size(Qt::TextSingleLine, curTxt);
        startY = curRow * curSize.height();
        QRect curRect(startX, startY, curSize.width(), curSize.height());
        region = region.united(curRect);                    // 追加当前字符绘制区域
        p.drawText(curRect, curTxt);

        drawText = drawText.mid(tmpSize);                   // 剩余待显示的字符

        // 全部显示后，不能增加行，不能重置开始位置，以便后续计算开始位置
        if (!drawText.isEmpty()) {
            curRow++;
            startX = 0;
        }
    }

    // 找到最后一行的结束位置
    startX += curSize.width();

    return region;
}

int HyperlinkLabel::displayLength(const int startX, QString showTxt)
{
    int txtWidth = m_fontMetrics.size(Qt::TextSingleLine, showTxt).width();
    int labelWidth = width() - startX;  // 当前行剩余的可用宽度

    // 已有空间足够显示所有字符，则直接返回字符大小
    if (labelWidth > txtWidth)
        return showTxt.size();

#if 0
    // 循环减少字符进行判断
    while (labelWidth <= txtWidth) {
        showTxt = showTxt.left(showTxt.count()-1);
        txtWidth = fontMetrics.size(Qt::TextSingleLine, showTxt).width();
    }
    return showTxt.size();
#else
    // 二分查找合适位置进行判断
    int left = 0;
    int right = showTxt.size();
    int mid = right / 2;
    QString tmpStr;

    while (0 != right && left != right) {
        tmpStr = showTxt.left(mid);
        txtWidth = m_fontMetrics.size(Qt::TextSingleLine, tmpStr).width();

        // 字符宽度与语言有关，不能使用相等判断，增减一个字符时宽度变化较大
        if (labelWidth < txtWidth) {    // 剩余空间小于字符宽度
            // 尝试减少一个字符
            tmpStr = showTxt.left(mid - 1);
            txtWidth = m_fontMetrics.size(Qt::TextSingleLine, tmpStr).width();
            if (labelWidth > txtWidth) {
                // 减少一个字符后宽度小于剩余空间，则返回减少后的大小
                return mid - 1;
            }

            // 减少已有字符的一半，继续判断
            right = mid;
            mid = (left + right) / 2;
        } else {    // 剩余空间大于等于字符宽度
            // 尝试增加一个字符
            tmpStr = showTxt.left(mid + 1);
            txtWidth = m_fontMetrics.size(Qt::TextSingleLine, tmpStr).width();
            if (labelWidth < txtWidth) {
                // 增加一个字符后宽度超过剩余控件，则返回增加前的大小
                return mid;
            }

            // 增加剩余字符的一半，继续判断
            left = mid;
            mid = (left + right) / 2;
        }
    }
    return 0;
#endif
}
