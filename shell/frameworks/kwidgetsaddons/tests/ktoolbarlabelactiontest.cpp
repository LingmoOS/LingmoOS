/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Benjamin Port <benjamin.port@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QApplication>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include <QToolBar>
#include <QVBoxLayout>

#include <ktoolbarlabelaction.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT
private Q_SLOTS:
    void updateLabel1()
    {
        label1->setText(labelText->text());
    }

public:
    MainWindow()
    {
        QWidget *mainWidget = new QWidget(this);
        setCentralWidget(mainWidget);

        QVBoxLayout *layout = new QVBoxLayout(mainWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        QToolBar *toolBar = addToolBar(QStringLiteral("Toolbar"));

        labelText = new QLineEdit();
        QPushButton *validateButton = new QPushButton(QStringLiteral("Update label"));

        layout->addWidget(labelText);
        layout->addWidget(validateButton);
        connect(validateButton, &QAbstractButton::clicked, this, &MainWindow::updateLabel1);

        QWidgetAction *lineEditAction2 = new QWidgetAction(toolBar);
        QLineEdit *lineEdit2 = new QLineEdit;
        lineEditAction2->setDefaultWidget(lineEdit2);
        label1 = new KToolBarLabelAction(QStringLiteral("&Label 1"), toolBar);
        label2 = new KToolBarLabelAction(lineEditAction2, QStringLiteral("&Second label"), toolBar);
        QWidgetAction *lineEditAction = new QWidgetAction(toolBar);
        QLineEdit *lineEdit = new QLineEdit;
        lineEditAction->setDefaultWidget(lineEdit);
        // set buddy for label1
        label1->setBuddy(lineEditAction);
        toolBar->addAction(label1);
        toolBar->addAction(lineEditAction);
        toolBar->addAction(label2);
        toolBar->addAction(lineEditAction2);
    }

    QLineEdit *labelText;
    KToolBarLabelAction *label1;
    KToolBarLabelAction *label2;
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("Test KToolBarLabelAction"));
    QApplication app(argc, argv);

    MainWindow *window = new MainWindow;
    window->show();

    return app.exec();
}

#include "ktoolbarlabelactiontest.moc"
