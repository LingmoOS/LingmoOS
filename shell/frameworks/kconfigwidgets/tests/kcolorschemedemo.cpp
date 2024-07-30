/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KActionMenu>
#include <kcolorschememanager.h>
#include <kcolorschememenu.h>

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolButton>
#include <QVBoxLayout>

class KColorSchemeDemo : public QMainWindow
{
    Q_OBJECT
public:
    KColorSchemeDemo()
        : QMainWindow(nullptr)
    {
        KColorSchemeManager *manager = new KColorSchemeManager(this);

        QListView *view = new QListView(this);
        view->setModel(manager->model());
        connect(view, &QListView::activated, manager, &KColorSchemeManager::activateScheme);
        manager->setAutosaveChanges(true);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Close, this);
        connect(box, &QDialogButtonBox::rejected, qApp, &QApplication::quit);

        QToolButton *button = new QToolButton(box);
        button->setIcon(QIcon::fromTheme(QStringLiteral("fill-color")));
        button->setMenu(KColorSchemeMenu::createMenu(manager, button)->menu());
        box->addButton(button, QDialogButtonBox::InvalidRole);

        QWidget *w = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(w);
        layout->addWidget(view);
        layout->addWidget(box);

        setCentralWidget(w);

        QMenu *menu = new QMenu("Menu", this);
        menu->addAction(KColorSchemeMenu::createMenu(manager, this));
        menuBar()->addMenu(menu);
    }
    ~KColorSchemeDemo() override
    {
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KColorSchemeDemo *d = new KColorSchemeDemo;
    d->show();
    return app.exec();
}

#include "kcolorschemedemo.moc"
