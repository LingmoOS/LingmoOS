/*
    KPeople - Duplicates
    SPDX-FileCopyrightText: 2013 Franck Arrecot <franck.arrecot@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MERGEDIALOG_H
#define MERGEDIALOG_H

#include <kpeople/widgets/kpeoplewidgets_export.h>

#include <QDialog>

class QStandardItem;
class KJob;
class MergeDialogPrivate;

namespace KPeople
{
class PersonsModel;
class Match;

/**
 * The merge dialog will be used to provide a GUI to attempt to
 * figure out what contacts should be merged.
 *
 * By properly mergeing contacts, the user will end up with having more
 * information on each person.
 *
 * @since 5.8
 */
class KPEOPLEWIDGETS_EXPORT MergeDialog : public QDialog
{
    Q_OBJECT

public:
    enum Role {
        NameRole = Qt::DisplayRole,
        UriRole = Qt::UserRole + 1,
        PixmapRole,
        MergeReasonRole,
    };

    explicit MergeDialog(QWidget *parent = nullptr);
    ~MergeDialog() override;

    /**
     * Specifies which PersonsModel will be used to look for duplicates.
     */
    void setPersonsModel(PersonsModel *model);

private Q_SLOTS:
    KPEOPLEWIDGETS_NO_EXPORT void searchForDuplicates();
    KPEOPLEWIDGETS_NO_EXPORT void searchForDuplicatesFinished(KJob *);
    KPEOPLEWIDGETS_NO_EXPORT void onMergeButtonClicked();

private:
    KPEOPLEWIDGETS_NO_EXPORT QStandardItem *itemMergeContactFromMatch(bool parent, const KPeople::Match &match);
    KPEOPLEWIDGETS_NO_EXPORT void feedDuplicateModelFromMatches(const QList<Match> &matches);

    MergeDialogPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(MergeDialog)
};

}

#endif // MERGEDIALOG_H
