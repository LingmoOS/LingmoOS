/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
*/

#include "knotifytestwindow.h"
#include "knotification.h"
#include "knotifyconfigwidget.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardActions>
#include <KXMLGUIFactory>

#include <QDialog>
#include <QDialogButtonBox>
#include <QStatusBar>
#include <QTest>
#include <QVBoxLayout>

// ------------------------------------------------------------------------

KNotifyTestWindow::KNotifyTestWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
    , m_nbNewMessage(0)
{
    QWidget *w = new QWidget(this);
    view.setupUi(w);
    //  statusBar()->message(i18n("Test program for KNotify"));
    setCaption(i18n("Test program for KNotify"));

    setCentralWidget(w);

    // set up the actions
    actionCollection()->addAction(KStandardActions::Quit, this, &KNotifyTestWindow::close);

    KStandardActions::keyBindings(guiFactory(), &KXMLGUIFactory::showConfigureShortcutsDialog, actionCollection());

    createGUI(QFINDTESTDATA("knotifytestui.rc"));

    connect(view.b_online, SIGNAL(clicked()), this, SLOT(slotSendOnlineEvent()));
    connect(view.b_message, SIGNAL(clicked()), this, SLOT(slotSendMessageEvent()));
    connect(view.b_read, SIGNAL(clicked()), this, SLOT(slotMessageRead()));
    connect(view.b_confG, SIGNAL(clicked()), this, SLOT(slotConfigureG()));
}

void KNotifyTestWindow::slotSendOnlineEvent()
{
    KNotification *n = new KNotification("online");
    n->setText(i18n("the contact %1 is now online", view.c_name->text()));
    n->sendEvent();
}

void KNotifyTestWindow::slotSendMessageEvent()
{
    m_nbNewMessage++;
    if (!m_readNotif) {
        KNotification *n = new KNotification("message", KNotification::Persistent);
        n->setText(i18n("new message : %1", view.c_text->toPlainText()));
        auto action = n->addAction(i18n("Read"));
        connect(action, &KNotificationAction::activated, this, &KNotifyTestWindow::slotMessageRead);

        m_readNotif = n;
    } else {
        m_readNotif->setText(i18n("%1 new messages", m_nbNewMessage));
    }

    m_readNotif->sendEvent();
}

void KNotifyTestWindow::slotMessageRead()
{
    m_nbNewMessage = 0;
    if (m_readNotif) {
        m_readNotif->close();
    }
    KMessageBox::information(this, view.c_text->toPlainText(), i18n("reading message"));
}

void KNotifyTestWindow::slotConfigureG()
{
    KNotifyConfigWidget::configure(this);
}

#include "moc_knotifytestwindow.cpp"
