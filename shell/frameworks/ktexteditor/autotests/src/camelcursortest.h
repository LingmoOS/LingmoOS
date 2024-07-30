/*
    This file is part of the Kate project.

    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CAMELCURSORTEST_H
#define CAMELCURSORTEST_H

#include <QObject>

namespace KTextEditor
{
class ViewPrivate;
class DocumentPrivate;
}

class CamelCursorTest : public QObject
{
    Q_OBJECT
public:
    CamelCursorTest(QObject *parent = nullptr);
    ~CamelCursorTest() override;

private Q_SLOTS:
    void testWordMovementSingleRow_data();
    void testWordMovementSingleRow();
    void testRtlWordMovement();
    void testWordMovementMultipleRow_data();
    void testWordMovementMultipleRow();
    void testDeletionRight();
    void testDeletionLeft();
    void testSelectionRight();
    void testSelectionLeft();

private:
    KTextEditor::ViewPrivate *view;
    KTextEditor::DocumentPrivate *doc;
};

#endif
