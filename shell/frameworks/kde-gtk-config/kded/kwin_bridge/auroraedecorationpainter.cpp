/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QFile>
#include <QStandardPaths>
#include <QSvgRenderer>

#include "auroraedecorationpainter.h"

const QString AuroraeDecorationPainter::s_auroraeThemesPath =
    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/aurorae/themes/");

AuroraeDecorationPainter::AuroraeDecorationPainter(const QString &themeName)
    : DecorationPainter()
    , m_themeName(themeName)
    , m_themePath(s_auroraeThemesPath + themeName + '/')
{
}

void AuroraeDecorationPainter::paintButton(QPainter &painter, const QString &buttonType, const QString &buttonState) const
{
    const QString buttonFileName = buttonTypeToFileName(buttonType);
    const QString elementIdName = buttonStateToElementId(buttonState);

    const QString buttonFilePath = m_themePath + buttonFileName;

    QSvgRenderer buttonRenderer;
    buttonRenderer.load(buttonFilePath) || buttonRenderer.load(buttonFilePath + "z");
    buttonRenderer.render(&painter, elementIdName, DecorationPainter::ButtonGeometry);
}

QString AuroraeDecorationPainter::buttonTypeToFileName(const QString &buttonType) const
{
    if (buttonType == QStringLiteral("maximized")) {
        static const QString restoreButtonFileName = QStringLiteral("restore.svg");
        if (QFile(m_themePath + restoreButtonFileName).exists()) {
            return restoreButtonFileName;
        } else {
            return QStringLiteral("maximize.svg");
        }
    } else {
        return buttonType + QStringLiteral(".svg");
    }
}

QString AuroraeDecorationPainter::buttonStateToElementId(const QString &buttonState) const
{
    // Aurorae themes do not contain pressed and hovered inactive states,
    // so we put backdrop option to the end, so that first we use pressed and hover ones
    if (buttonState.contains(QStringLiteral("active"))) {
        return QStringLiteral("pressed-center");
    } else if (buttonState.contains(QStringLiteral("hover"))) {
        return QStringLiteral("hover-center");
    } else if (buttonState.contains(QStringLiteral("backdrop"))) {
        return QStringLiteral("inactive-center");
    } else {
        return QStringLiteral("active-center");
    }
}
