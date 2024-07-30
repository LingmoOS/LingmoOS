/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef TESTAPPLICATION_H
#define TESTAPPLICATION_H

#include <QMainWindow>

class DynamicTreeModel;

//@cond PRIVATE

/**
 * @internal
 * Test Application for proxy models.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() override;

private:
    DynamicTreeModel *m_rootModel;
    //   ContactsWidget* cw;
};

//@endcond

#endif
