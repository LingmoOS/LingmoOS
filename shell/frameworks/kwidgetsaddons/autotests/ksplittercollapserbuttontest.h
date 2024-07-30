/*
    SPDX-FileCopyrightText: 2014 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KSPLITTERCOLLAPSERBUTTONTEST_H
#define KSPLITTERCOLLAPSERBUTTONTEST_H

#include <QWidget>
class QSplitter;
class QTextEdit;

class TestSplitter : public QWidget
{
    Q_OBJECT
public:
    explicit TestSplitter(QWidget *parent = nullptr);
    QSplitter *splitter;
    QTextEdit *edit1;
    QTextEdit *edit2;
};

class KSplitterCollapserButtonTest : public QObject
{
    Q_OBJECT
public:
    explicit KSplitterCollapserButtonTest(QObject *parent = nullptr);
    ~KSplitterCollapserButtonTest() override;

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldCollapseWhenClickOnButton();
    void shouldRestoreCorrectPosition();
    void shouldRestoreCorrectPositionForFirstWidget();
    void shouldTestVerticalSplitterFirstWidget();
    void shouldTestVerticalSplitterSecondWidget();
    void shouldBeVisible_data();
    void shouldBeVisible();
    void shouldBeVisibleWhenMovingHandle_data();
    void shouldBeVisibleWhenMovingHandle();
};

#endif // KSPLITTERCOLLAPSERBUTTONTEST_H
