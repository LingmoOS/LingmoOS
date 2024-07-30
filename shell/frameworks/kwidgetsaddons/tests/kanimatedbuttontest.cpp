/*
    SPDX-FileCopyrightText: 2008 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kanimatedbuttontest.h"

#include "kanimatedbutton.h"

#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <qlayout.h>

AnimationGroup::AnimationGroup(const QString &path, int size, QWidget *parent)
    : QGroupBox(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    m_animButton = new KAnimatedButton(this);
    lay->addWidget(m_animButton);
    QPushButton *start = new QPushButton(QStringLiteral("Start"), this);
    lay->addWidget(start);
    QPushButton *stop = new QPushButton(QStringLiteral("Stop"), this);
    lay->addWidget(stop);

    setTitle(QStringLiteral("%1 (%2)").arg(path).arg(size));
    m_animButton->setIconSize(QSize(size, size));
    m_animButton->setAnimationPath(path);

    connect(start, &QAbstractButton::clicked, m_animButton, &KAnimatedButton::start);
    connect(stop, &QAbstractButton::clicked, m_animButton, &KAnimatedButton::stop);
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    QWidget *top = new QWidget(this);
    lay->addWidget(top);
    QHBoxLayout *lay2 = new QHBoxLayout(top);
    m_path = new QLineEdit(top);
    lay2->addWidget(m_path);
    m_size = new QSpinBox(top);
    lay2->addWidget(m_size);

    m_size->setValue(22);

    connect(m_path, &QLineEdit::returnPressed, this, &MainWindow::slotAddNew);
}

void MainWindow::slotAddNew()
{
    AnimationGroup *group = new AnimationGroup(m_path->text(), m_size->value(), this);
    layout()->addWidget(group);
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kanimatedbuttontest"));
    QApplication app(argc, argv);

    MainWindow *window = new MainWindow();
    window->show();

    return app.exec();
}

#include "moc_kanimatedbuttontest.cpp"
