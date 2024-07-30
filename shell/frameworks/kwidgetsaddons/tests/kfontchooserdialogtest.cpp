/*
    SPDX-FileCopyrightText: 1996 Bernd Johannes Wuebben <wuebben@math.cornell.edu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kfontchooserdialog.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("KFontChooserDialogTest"));

    QApplication app(argc, argv);

    QFont font;
    // Use a font with an unusual styleName
    font.fromString(QStringLiteral("Noto Sans,13,-1,5,87,1,0,0,0,0,Black Italic"));

    qDebug() << "Default use case, all bells and whistles";
    int nRet = KFontChooserDialog::getFont(font);
    qDebug() << font.toString();

    qDebug() << "Only show monospaced fonts, FixedOnly checkbox is _not_ shown";
    nRet = KFontChooserDialog::getFont(font, KFontChooser::FixedFontsOnly);
    qDebug() << font.toString();

    KFontChooser::FontDiffFlags diffFlags;
    qDebug() << "ShowDifferences mode";
    nRet = KFontChooserDialog::getFontDiff(font, diffFlags);
    qDebug() << font.toString();

    qDebug() << "ShowDifferences mode and only showing monospaced fonts (the FixedOnly checkbox is _not_ shown)";
    nRet = KFontChooserDialog::getFontDiff(font, diffFlags, KFontChooser::FixedFontsOnly);
    qDebug() << font.toString();

    return nRet;
}
