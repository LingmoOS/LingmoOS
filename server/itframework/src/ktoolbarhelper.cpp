/*
    SPDX-FileCopyrightText: 2017 Alexander Potashev <aspotashev@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ktoolbarhelper_p.h"

#include <QVector>

#include <KLocalizedString>

namespace KToolbarHelper
{
QString i18nToolBarName(const QDomElement &element)
{
    QDomElement textElement;
    bool textElementFound = false;
    const QVector<QString> textKeys = {QStringLiteral("text"), QStringLiteral("Text")};
    for (const QString &key : textKeys) {
        QDomNode textNode = element.namedItem(key);
        if (textNode.isElement()) {
            textElement = textNode.toElement();
            textElementFound = true;
            break;
        }
    }

    if (!textElementFound) {
        return element.attribute(QStringLiteral("name"));
    }

    QByteArray domain = textElement.attribute(QStringLiteral("translationDomain")).toUtf8();
    QByteArray text = textElement.text().toUtf8();
    QByteArray context = textElement.attribute(QStringLiteral("context")).toUtf8();

    if (domain.isEmpty()) {
        domain = element.ownerDocument().documentElement().attribute(QStringLiteral("translationDomain")).toUtf8();
        if (domain.isEmpty()) {
            domain = KLocalizedString::applicationDomain();
        }
    }
    QString i18nText;
    if (!text.isEmpty() && !context.isEmpty()) {
        i18nText = i18ndc(domain.constData(), context.constData(), text.constData());
    } else if (!text.isEmpty()) {
        i18nText = i18nd(domain.constData(), text.constData());
    }
    return i18nText;
}

} // namespace KToolbarHelper
