// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "LongTextLabel.h"

// Dtk头文件
#include <DFontSizeManager>
#include <DApplication>
#include <DSysInfo>

// Qt库文件
#include <QLoggingCategory>
#include <QAbstractTextDocumentLayout>
#include <QDomDocument>

DCORE_USE_NAMESPACE

LongTextLabel::LongTextLabel(DWidget *parent)
    : DLabel(parent)
    , m_width(width())
{
    setAlignment(Qt::AlignVCenter);
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
}

void LongTextLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QTextDocument docText;
    painter.save();

    QFont fo = painter.font();
    QFontMetrics fm(fo);

    QString html;
    QString OS;

    QRegExp reg("[\\s\\S]*</a>");
    if (reg.indexIn(this->text()) != -1) {
        html = reg.cap(0);

        reg.setPattern("<[\\s\\S]*>([\\s\\S]*)</a>");

        if (reg.indexIn(html) != -1) {
            OS = reg.cap(1);
        }
    }

    QString src = this->text();
    if (!html.isEmpty() && !OS.isEmpty()) {
        src  = OS + this->text().remove(html);
    }

    QString text = fm.elidedText(src, Qt::TextElideMode::ElideRight, this->rect().width());

    QString dst = text;
    if (!html.isEmpty() && !OS.isEmpty()) {
        dst = html + text.remove(OS);
    }

    docText.setHtml(dst);
    docText.setDefaultFont(fo);
    docText.drawContents(&painter, this->rect());
    painter.restore();
//    return DLabel::paintEvent(event);
}
