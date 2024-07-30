/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 Aurélien Gâteau <agateau@kde.org>
    Based on test program by Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QWidget>

#include <kmessagewidget.h>

class Receiver : public QObject
{
    Q_OBJECT
public:
    Receiver(QObject *parent)
        : QObject(parent)
    {
    }
    ~Receiver() override
    {
    }

public Q_SLOTS:
    void showWhatsThis(const QString &text)
    {
        QWhatsThis::showText(QCursor::pos(), text);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *mainWindow = new QWidget();

    QVBoxLayout *l = new QVBoxLayout(mainWindow);

    KMessageWidget *mw = new KMessageWidget(mainWindow);
    mw->setMessageType(KMessageWidget::Positive);
    mw->setWordWrap(true);
    mw->setText(QStringLiteral("Test KMessageWidget is properly sized when <a href=\"this is the contents\">word-wrap</a> is enabled by default."));
    mw->setIcon(QIcon::fromTheme(QStringLiteral("kde")));
    KMessageWidget *mw1 = new KMessageWidget(mainWindow);
    mw1->setMessageType(KMessageWidget::Positive);
    mw1->setWordWrap(true);
    mw1->setText(QStringLiteral("Test KMessageWidget icon alignemnt<br />with pre-wrapped<br />content<br />like<br />this.<br />Please test with word wrap enabled and disabled."));
    mw1->setIcon(QIcon::fromTheme(QStringLiteral("kde")));
    KMessageWidget *mw2 = new KMessageWidget(mainWindow);
    mw2->setWordWrap(true);
    mw2->setText(QStringLiteral("A KMessageWidget with an icon and two additional buttons"));
    mw2->setIcon(QIcon::fromTheme(QStringLiteral("kde")));
    mw2->addAction(new QAction(QStringLiteral("Foo"), mw2));
    mw2->addAction(new QAction(QStringLiteral("Bar"), mw2));
    KMessageWidget *mw3 = new KMessageWidget(mainWindow);
    mw3->setMessageType(KMessageWidget::Warning);
    mw3->setWordWrap(true);
    mw3->setText(QStringLiteral("A KMessageWidget of Warning type."));
    KMessageWidget *mw4 = new KMessageWidget(mainWindow);
    mw4->setMessageType(KMessageWidget::Error);
    mw4->setWordWrap(true);
    mw4->setText(QStringLiteral("A KMessageWidget of Error type."));

    // A frame to materialize the end of the KMessageWidget
    QFrame *frame = new QFrame(mainWindow);
    frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QCheckBox *wordWrapCb = new QCheckBox(QStringLiteral("wordWrap"), mainWindow);
    wordWrapCb->setChecked(true);
    QObject::connect(wordWrapCb, &QAbstractButton::toggled, mw, &KMessageWidget::setWordWrap);
    QObject::connect(wordWrapCb, &QAbstractButton::toggled, mw1, &KMessageWidget::setWordWrap);
    QObject::connect(wordWrapCb, &QAbstractButton::toggled, mw2, &KMessageWidget::setWordWrap);
    QObject::connect(wordWrapCb, &QAbstractButton::toggled, mw3, &KMessageWidget::setWordWrap);
    QObject::connect(wordWrapCb, &QAbstractButton::toggled, mw4, &KMessageWidget::setWordWrap);

    QCheckBox *closeButtonCb = new QCheckBox(QStringLiteral("closeButton"), mainWindow);
    closeButtonCb->setChecked(true);
    QObject::connect(closeButtonCb, &QAbstractButton::toggled, mw, &KMessageWidget::setCloseButtonVisible);
    QObject::connect(closeButtonCb, &QAbstractButton::toggled, mw1, &KMessageWidget::setCloseButtonVisible);
    QObject::connect(closeButtonCb, &QAbstractButton::toggled, mw2, &KMessageWidget::setCloseButtonVisible);
    QObject::connect(closeButtonCb, &QAbstractButton::toggled, mw3, &KMessageWidget::setCloseButtonVisible);
    QObject::connect(closeButtonCb, &QAbstractButton::toggled, mw4, &KMessageWidget::setCloseButtonVisible);

    l->addWidget(wordWrapCb);
    l->addWidget(closeButtonCb);
    l->addWidget(mw);
    l->addWidget(mw1);
    l->addWidget(mw2);
    l->addWidget(mw3);
    l->addWidget(mw4);
    l->addWidget(frame);

    mainWindow->resize(400, 300);
    mainWindow->show();

    // demonstrate linkActivated
    Receiver *info = new Receiver(mw);
    QObject::connect(mw, &KMessageWidget::linkActivated, info, &Receiver::showWhatsThis);

    return app.exec();
    delete mainWindow;
}

#include "kmessagewidgettest.moc"

// kate: replace-tabs on;
