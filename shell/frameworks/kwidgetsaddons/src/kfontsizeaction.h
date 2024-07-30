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

#ifndef KFONTSIZEACTION_H
#define KFONTSIZEACTION_H

#include <kselectaction.h>

class KFontSizeActionPrivate;

/**
 * @class KFontSizeAction kfontsizeaction.h KFontSizeAction
 *
 * An action to allow changing of the font size.
 * This action will be shown as a combobox on a toolbar with a proper set of font sizes.
 */
class KWIDGETSADDONS_EXPORT KFontSizeAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize)

public:
    explicit KFontSizeAction(QObject *parent);
    KFontSizeAction(const QString &text, QObject *parent);
    KFontSizeAction(const QIcon &icon, const QString &text, QObject *parent);

    ~KFontSizeAction() override;

    int fontSize() const;

    void setFontSize(int size);

Q_SIGNALS:
    void fontSizeChanged(int);

protected Q_SLOTS:
    /**
     * This function is called whenever an action from the selections is triggered.
     */
    void slotActionTriggered(QAction *action) override;

private:
    Q_DECLARE_PRIVATE(KFontSizeAction)
};

#endif
