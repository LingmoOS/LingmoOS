/*
    SPDX-FileCopyrightText: 1996 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1999 Mario Weilguni <mweilguni@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kfontchooserdialog.h"

#include <QDialogButtonBox>
#include <QPointer>
#include <QVBoxLayout>

class KFontChooserDialogPrivate
{
public:
    KFontChooser *m_fontChooser = nullptr;
};

KFontChooserDialog::KFontChooserDialog(const KFontChooser::DisplayFlags &flags, QWidget *parent)
    : QDialog(parent)
    , d(new KFontChooserDialogPrivate)
{
    setWindowTitle(tr("Select Font", "@title:window"));
    d->m_fontChooser = new KFontChooser(flags, this);
    d->m_fontChooser->setMinVisibleItems(8);
    d->m_fontChooser->setObjectName(QStringLiteral("fontChooser"));

    connect(d->m_fontChooser, &KFontChooser::fontSelected, this, &KFontChooserDialog::fontSelected);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(d->m_fontChooser);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

KFontChooserDialog::~KFontChooserDialog() = default;

void KFontChooserDialog::setFont(const QFont &font, bool onlyFixed)
{
    d->m_fontChooser->setFont(font, onlyFixed);
}

QFont KFontChooserDialog::font() const
{
    return d->m_fontChooser->font();
}

// If the styleName property is set for a QFont, using setBold(true) would
// lead to Qt using an "emboldended"/synthetic font style instead of using
// the bold style provided by the font itself; the latter looks better than
// the former, also accorgin to upstream, the styleName property is useful
// for fancy font styles, so there is no point in setting it for "Regular"
// fonts. For more details see:
// https://bugreports.qt.io/browse/QTBUG-63792
// https://bugs.kde.org/show_bug.cgi?id=378523
static void stripRegularStyleName(QFont &font)
{
    if (font.weight() == QFont::Normal //
        && (font.styleName() == QLatin1String("Regular") //
            || font.styleName() == QLatin1String("Normal") //
            || font.styleName() == QLatin1String("Book") //
            || font.styleName() == QLatin1String("Roman"))) {
        font.setStyleName(QString());
    }
}

// static
int KFontChooserDialog::getFontDiff(QFont &theFont, KFontChooser::FontDiffFlags &diffFlags, const KFontChooser::DisplayFlags &flags, QWidget *parent)
{
    QPointer<KFontChooserDialog> dialog = new KFontChooserDialog(flags | KFontChooser::ShowDifferences, parent);
    dialog->setObjectName(QStringLiteral("Font Selector"));
    dialog->setFont(theFont, flags & KFontChooser::FixedFontsOnly);

    const int result = dialog->exec();
    if (result == Accepted) {
        theFont = dialog->d->m_fontChooser->font();
        diffFlags = dialog->d->m_fontChooser->fontDiffFlags();
        stripRegularStyleName(theFont);
    }
    delete dialog;
    return result;
}

// static
int KFontChooserDialog::getFont(QFont &theFont, const KFontChooser::DisplayFlags &flags, QWidget *parent)
{
    QPointer<KFontChooserDialog> dialog = new KFontChooserDialog(flags, parent);
    dialog->setObjectName(QStringLiteral("Font Selector"));
    dialog->setFont(theFont, flags & KFontChooser::FixedFontsOnly);

    const int result = dialog->exec();
    if (result == Accepted) {
        theFont = dialog->d->m_fontChooser->font();
        stripRegularStyleName(theFont);
    }
    delete dialog;
    return result;
}

#include "moc_kfontchooserdialog.cpp"
