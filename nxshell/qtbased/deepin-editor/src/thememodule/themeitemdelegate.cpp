// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "themeitemdelegate.h"
#include "themelistmodel.h"
#include "../common/utils.h"
#include <QPainter>
#include <QDebug>

ThemeItemDelegate::ThemeItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{

}

ThemeItemDelegate::~ThemeItemDelegate()
{
}

void ThemeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QString &themePath = index.data(ThemeListModel::ThemePath).toString();
    const QString &themeName = index.data(ThemeListModel::ThemeName).toString();
    QVariantMap jsonMap = Utils::getThemeMapFromPath(themePath);
    const QString &importColor = jsonMap["text-styles"].toMap()["Import"].toMap()["text-color"].toString();
    const QString &stringColor = jsonMap["text-styles"].toMap()["String"].toMap()["text-color"].toString();
    const QString &builtInColor = jsonMap["text-styles"].toMap()["BuiltIn"].toMap()["text-color"].toString();
    const QString &keywordColor = jsonMap["text-styles"].toMap()["Keyword"].toMap()["text-color"].toString();
    const QString &commentColor = jsonMap["text-styles"].toMap()["Comment"].toMap()["text-color"].toString();
    const QString &functionColor = jsonMap["text-styles"].toMap()["Function"].toMap()["text-color"].toString();
    const QString &normalColor = jsonMap["text-styles"].toMap()["Normal"].toMap()["text-color"].toString();
    const QString &otherColor = jsonMap["text-styles"].toMap()["Others"].toMap()["text-color"].toString();
    const QString &backgroundColor = jsonMap["editor-colors"].toMap()["background-color"].toString();

    const QString &frameNormalColor = index.data(ThemeListModel::FrameNormalColor).toString();
    const QString &frameSelectedColor = index.data(ThemeListModel::FrameSelectedColor).toString();

    const QRect &rect = option.rect;

    painter->setRenderHint(QPainter::Antialiasing, true);

    QFont font;
    font.setPointSize(10);
    painter->setFont(font);

    int paddingX = 15;
    int paddingY = 8;

    // draw background.
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(QRect(rect.x() + paddingX,
                                        rect.y() + paddingY,
                                        rect.width() - paddingX * 2,
                                        rect.height() - paddingY * 2),
                             m_frameRadius, m_frameRadius);

    painter->setOpacity(0.8);
    painter->fillPath(backgroundPath, QColor(backgroundColor));

    // draw frame.
    QPainterPath framePath;
    framePath.addRoundedRect(QRect(rect.x() + paddingX,
                                   rect.y() + paddingY,
                                   rect.width() - paddingX * 2,
                                   rect.height() - paddingY * 2),
                             m_frameRadius, m_frameRadius);
    QPen framePen;

    if (option.state & QStyle::State_Selected) {
        painter->setOpacity(1);
        framePen = QPen(QColor(frameSelectedColor), 2);
    } else {
        painter->setOpacity(0.3);
        framePen = QPen(QColor(frameNormalColor), 1);
    }

    painter->setPen(framePen);
    painter->drawPath(framePath);

    // draw syntax highlight.
    painter->setOpacity(1);
    QFontMetrics fm(font);
    int lineHeight = 20;

    int includeX = paddingX + 8;
    int includeY = paddingY + 8;
    painter->setPen(QPen(QColor(otherColor)));
    painter->drawText(QRect(rect.x() + includeX, rect.y() + includeY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "#include");

    int headerX = includeX + fm.width("#include ");
    int headerY = includeY;
    painter->setPen(QPen(QColor(importColor)));
    painter->drawText(QRect(rect.x() + headerX, rect.y() + headerY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "\"deepin.h\"");

    int keywordX = includeX;
    int keywordY = includeY + lineHeight;
    painter->setPen(QPen(QColor(keywordColor)));
    painter->drawText(QRect(rect.x() + keywordX, rect.y() + keywordY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "QString");

    int functionX = includeX + fm.width("QString ");
    int functionY = includeY + lineHeight;
    painter->setPen(QPen(QColor(functionColor)));
    painter->drawText(QRect(rect.x() + functionX, rect.y() + functionY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "theme");

    int functionArgX = includeX + fm.width("QString theme");
    int functionArgY = includeY + lineHeight;
    painter->setPen(QPen(QColor(normalColor)));
    painter->drawText(QRect(rect.x() + functionArgX, rect.y() + functionArgY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "() {");

    int commentX = includeX + fm.width("QStr");
    int commentY = includeY + lineHeight * 2;
    painter->setPen(QPen(QColor(commentColor)));
    painter->drawText(QRect(rect.x() + commentX, rect.y() + commentY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "// Return theme name.");

    int builtInX = includeX + fm.width("QStr");
    int builtInY = includeY + lineHeight * 3;
    painter->setPen(QPen(QColor(builtInColor)));
    painter->drawText(QRect(rect.x() + builtInX, rect.y() + builtInY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "return");

    int stringX = includeX + fm.width("QStr") + fm.width("return ");
    int stringY = includeY + lineHeight * 3;
    painter->setPen(QPen(QColor(stringColor)));
    painter->drawText(QRect(rect.x() + stringX, rect.y() + stringY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, QString("\"%1\"").arg(themeName));

    int semicolonX = includeX + fm.width("QStr") + fm.width("return ") + fm.width(QString("\"%1\"").arg(themeName));
    int semicolonY = includeY + lineHeight * 3;
    painter->setPen(QPen(QColor(normalColor)));
    painter->drawText(QRect(rect.x() + semicolonX, rect.y() + semicolonY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, ";");

    int bracketX = includeX;
    int bracketY = includeY + lineHeight * 4;
    painter->setPen(QPen(QColor(normalColor)));
    painter->drawText(QRect(rect.x() + bracketX, rect.y() + bracketY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, "}");
}

QSize ThemeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(-1, 130);
}
