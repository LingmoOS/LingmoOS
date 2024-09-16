// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HYPERLINKLABEL_H
#define HYPERLINKLABEL_H

#include <QLabel>
#include <QFontMetrics>
#include <QRegion>

namespace GrandSearch {

class HyperlinkLabel : public QLabel
{
    Q_OBJECT
public:
    explicit HyperlinkLabel(const QString &leadintText, const QString &hyperlink, const QString &endText, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    QRegion drawText(QString drawText, int &startX, int &startY, int &curRow);
    int displayLength(const int startX, QString showTxt);

private:
    QString m_leadingText;        // 超链接之前的说明字符
    QString m_hyperlink;          // 超链接
    QString m_endText;            // 超链接之后的说明字符
    QString m_showText;           // leadingText + hyperlink + endText
    QStringList m_showTextList;   // 列表保存所有字符

    QFontMetrics m_fontMetrics;   // 绘制字符的字体属性
    QRegion m_hyperlinkRegion;    // 超链接的绘制区域
};

}

#endif // HYPERLINKLABEL_H
