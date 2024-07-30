/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KFONTACTION_H
#define KFONTACTION_H

#include <kselectaction.h>

class KFontActionPrivate;

/**
 * @class KFontAction kfontaction.h KFontAction
 *
 * An action to select a font family.
 * On a toolbar this will show a combobox with all the fonts on the system.
 */
class KWIDGETSADDONS_EXPORT KFontAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY(QString font READ font WRITE setFont)

public:
    KFontAction(uint fontListCriteria, QObject *parent);
    explicit KFontAction(QObject *parent);
    KFontAction(const QString &text, QObject *parent);
    KFontAction(const QIcon &icon, const QString &text, QObject *parent);
    ~KFontAction() override;

    QString font() const;

    void setFont(const QString &family);

    QWidget *createWidget(QWidget *parent) override;

private:
    Q_DECLARE_PRIVATE(KFontAction)
};

#endif
