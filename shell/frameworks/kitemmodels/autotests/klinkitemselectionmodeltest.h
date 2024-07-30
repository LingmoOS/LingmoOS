/*
    SPDX-FileCopyrightText: 2013 Aurélien Gateau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLINKITEMSELECTIONMODELTEST_H
#define KLINKITEMSELECTIONMODELTEST_H

#include <QObject>

class QItemSelectionModel;
class QStandardItemModel;
class QSortFilterProxyModel;

class KLinkItemSelectionModel;

class KLinkItemSelectionModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void testToggle();
    void testMainSetCurrent();
    void testSubSetCurrent();
    void testChangeModel();
    void testChangeModelOfExternal();
    void testChangeLinkedSelectionModel();
    void testAdditionalLink();
    void testClearSelection();

private:
    QStandardItemModel *m_mainModel;
    QItemSelectionModel *m_mainSelectionModel;
    QSortFilterProxyModel *m_subModel;
    KLinkItemSelectionModel *m_subSelectionModel;
};

#endif /* KLINKITEMSELECTIONMODELTEST_H */
