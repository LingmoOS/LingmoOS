/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KFINDDIALOG_P_H
#define KFINDDIALOG_P_H

#include "kfind.h"
#include "kfinddialog.h"

#include <QStringList>

class KHistoryComboBox;

class QCheckBox;
class QDialogButtonBox;
class QGridLayout;
class QGroupBox;
class QMenu;
class QPushButton;

class KFindDialogPrivate
{
    Q_DECLARE_PUBLIC(KFindDialog)

public:
    KFindDialogPrivate(KFindDialog *qq)
        : q_ptr(qq)
        , regexpDialog(nullptr)
        , regexpDialogQueryDone(false)
        , initialShowDone(false)
        , enabled(KFind::WholeWordsOnly | KFind::FromCursor | KFind::SelectedText | KFind::CaseSensitive | KFind::FindBackwards | KFind::RegularExpression)
        , findExtension(nullptr)
        , buttonBox(nullptr)
    {
    }
    virtual ~KFindDialogPrivate() = default;

    void init(bool forReplace, const QStringList &findStrings, bool hasSelection);

    void slotPlaceholdersAboutToShow();
    void slotOk();
    void slotReject();
    void slotSelectedTextToggled(bool);
    void showPatterns();
    void showPlaceholders();
    void textSearchChanged(const QString &);

    KFindDialog *const q_ptr = nullptr;
    QDialog *regexpDialog = nullptr;
    bool regexpDialogQueryDone : 1;
    bool initialShowDone : 1;
    long enabled; // uses Options to define which search options are enabled
    QStringList findStrings;
    QString pattern;
    mutable QWidget *findExtension = nullptr;
    QDialogButtonBox *buttonBox = nullptr;

    QGroupBox *findGrp = nullptr;
    KHistoryComboBox *find = nullptr;
    QCheckBox *regExp = nullptr;
    QPushButton *regExpItem = nullptr;
    QGridLayout *findLayout = nullptr;

    QCheckBox *wholeWordsOnly = nullptr;
    QCheckBox *fromCursor = nullptr;
    QCheckBox *selectedText = nullptr;
    QCheckBox *caseSensitive = nullptr;
    QCheckBox *findBackwards = nullptr;

    QMenu *patterns = nullptr;

    // for the replace dialog

    QGroupBox *replaceGrp = nullptr;
    KHistoryComboBox *replace = nullptr;
    QCheckBox *backRef = nullptr;
    QPushButton *backRefItem = nullptr;
    QGridLayout *replaceLayout = nullptr;

    QCheckBox *promptOnReplace = nullptr;

    QMenu *placeholders = nullptr;
};

#endif // KFINDDIALOG_P_H
