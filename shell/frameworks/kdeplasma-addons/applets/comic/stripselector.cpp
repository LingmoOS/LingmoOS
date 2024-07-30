/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stripselector.h"
#include "comicdata.h"
#include "stripselector_p.h"

#include <KDatePicker>
#include <KLocalizedString>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QSpinBox>

#include <QLabel>
#include <QScopedPointer>
#include <QTimer>
#include <QVBoxLayout>

// NOTE based on GotoPageDialog KDE/kdegraphics/okular/part.cpp
// BEGIN choose a strip dialog
class ChooseStripNumDialog : public QDialog
{
public:
    ChooseStripNumDialog(QWidget *parent, int current, int min, int max)
        : QDialog(parent)
    {
        setWindowTitle(i18nc("@title:window", "Go to Strip"));

        QVBoxLayout *topLayout = new QVBoxLayout(this);
        topLayout->setContentsMargins(0, 0, 0, 0);
        numInput = new QSpinBox(this);
        numInput->setRange(min, max);
        numInput->setValue(current);

        QLabel *label = new QLabel(i18nc("@label:spinbox", "&Strip number:"), this);
        label->setBuddy(numInput);
        topLayout->addWidget(label);
        topLayout->addWidget(numInput);
        // A little bit extra space
        topLayout->addStretch(10);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &ChooseStripNumDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &ChooseStripNumDialog::reject);
        topLayout->addWidget(buttonBox);

        numInput->setFocus();
    }

    int getStripNumber() const
    {
        return numInput->value();
    }

protected:
    QSpinBox *numInput;
};
// END choose a strip dialog

StripSelector::StripSelector(QObject *parent)
    : QObject(parent)
{
}

StripSelector::~StripSelector()
{
}

StripSelector *StripSelectorFactory::create(IdentifierType type)
{
    switch (type) {
    case IdentifierType::NumberIdentifier:
        return new NumberStripSelector();
    case IdentifierType::DateIdentifier:
        return new DateStripSelector();
    case IdentifierType::StringIdentifier:
        return new StringStripSelector();
    }

    return nullptr;
}

StringStripSelector::StringStripSelector(QObject *parent)
    : StripSelector(parent)
{
}

StringStripSelector::~StringStripSelector()
{
}

void StringStripSelector::select(const ComicData &currentStrip)
{
    bool ok;
    const QString strip = QInputDialog::getText(nullptr,
                                                i18nc("@title:window", "Go to Strip"),
                                                i18nc("@label:textbox", "Strip identifier:"),
                                                QLineEdit::Normal,
                                                currentStrip.current(),
                                                &ok);
    if (ok) {
        Q_EMIT stripChosen(strip);
    }
    deleteLater();
}

NumberStripSelector::NumberStripSelector(QObject *parent)
    : StripSelector(parent)
{
}

NumberStripSelector::~NumberStripSelector()
{
}

void NumberStripSelector::select(const ComicData &currentStrip)
{
    QScopedPointer<ChooseStripNumDialog> pageDialog(
        new ChooseStripNumDialog(nullptr, currentStrip.current().toInt(), currentStrip.firstStripNum(), currentStrip.maxStripNum()));
    if (pageDialog->exec() == QDialog::Accepted) {
        Q_EMIT stripChosen(QString::number(pageDialog->getStripNumber()));
    }
    deleteLater();
}

DateStripSelector::DateStripSelector(QObject *parent)
    : StripSelector(parent)
{
}

DateStripSelector::~DateStripSelector()
{
}

void DateStripSelector::select(const ComicData &currentStrip)
{
    mFirstIdentifierSuffix = currentStrip.first();

    KDatePicker *calendar = new KDatePicker;
    calendar->setAttribute(Qt::WA_DeleteOnClose); // to have destroyed emitted upon closing
    calendar->setMinimumSize(calendar->sizeHint());
    calendar->setDate(QDate::fromString(currentStrip.current(), QStringLiteral("yyyy-MM-dd")));

    connect(calendar, &KDatePicker::dateSelected, this, &DateStripSelector::slotChosenDay);
    connect(calendar, &KDatePicker::dateEntered, this, &DateStripSelector::slotChosenDay);

    // only delete this if the dialog got closed
    connect(calendar, &KDatePicker::destroyed, this, &DateStripSelector::deleteLater);
    calendar->show();
}

void DateStripSelector::slotChosenDay(const QDate &date)
{
    if (date <= QDate::currentDate()) {
        QDate temp = QDate::fromString(mFirstIdentifierSuffix, QStringLiteral("yyyy-MM-dd"));
        // only update if date >= first strip date, or if there is no first
        // strip date
        if (temp.isValid() || date >= temp) {
            Q_EMIT stripChosen(date.toString(QStringLiteral("yyyy-MM-dd")));
        }
    }
}
