/*
    SPDX-FileCopyrightText: 2014 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KSPLITTERCOLLAPSERBUTTONGUI_TEST_H
#define KSPLITTERCOLLAPSERBUTTONGUI_TEST_H

#include <QWidget>

class KSplitterCollapserButtonGui_test : public QWidget
{
    Q_OBJECT
public:
    explicit KSplitterCollapserButtonGui_test(int indexOfWidgetAssociateToSplitterCollapser, Qt::Orientation orientation, QWidget *parent = nullptr);
    ~KSplitterCollapserButtonGui_test() override;
};

#endif // KSPLITTERCOLLAPSERBUTTONGUI_TEST_H
