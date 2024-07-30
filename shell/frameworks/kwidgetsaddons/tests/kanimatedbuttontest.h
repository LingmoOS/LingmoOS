/*
    SPDX-FileCopyrightText: 2008 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANIMATEDBUTTONTEST_H
#define KANIMATEDBUTTONTEST_H

#include <QGroupBox>

class QLineEdit;
class QSpinBox;
class KAnimatedButton;

class AnimationGroup : public QGroupBox
{
    Q_OBJECT

public:
    AnimationGroup(const QString &name, int size, QWidget *parent = nullptr);

private:
    KAnimatedButton *m_animButton;
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private Q_SLOTS:
    void slotAddNew();

private:
    QLineEdit *m_path;
    QSpinBox *m_size;
};

#endif
