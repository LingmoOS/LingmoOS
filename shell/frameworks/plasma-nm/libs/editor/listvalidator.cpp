/*
    SPDX-FileCopyrightText: 2009 Andrey Batyiev <batyiev@gmail.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "listvalidator.h"

#include <QStringList>

ListValidator::ListValidator(QObject *parent)
    : QValidator(parent)
{
}

ListValidator::~ListValidator() = default;

QValidator::State ListValidator::validate(QString &text, int &pos) const
{
    Q_ASSERT(inner);
    Q_UNUSED(pos);

    QStringList strings = text.split(QLatin1Char(','));
    int unusedPos;
    QValidator::State state = Acceptable;
    for (QStringList::iterator i = strings.begin(); i != strings.end(); ++i) {
        QString string = i->trimmed();
        const int position = i->indexOf(string);
        const int size = string.size();
        const QValidator::State current = inner->validate(string, unusedPos);
        i->replace(position, size, string);
        if (current == Invalid) {
            state = Invalid;
            break;
        }
        if (current == Intermediate) {
            if (state == Intermediate) {
                state = Invalid;
                break;
            }
            state = Intermediate;
        }
    }
    text = strings.join(QLatin1Char(','));
    return state;
}

void ListValidator::setInnerValidator(QValidator *validator)
{
    inner = validator;
}

#include "moc_listvalidator.cpp"
