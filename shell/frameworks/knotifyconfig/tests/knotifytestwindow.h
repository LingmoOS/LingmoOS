/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
*/

#ifndef __KNotifyTestWINDOW_H__
#define __KNotifyTestWINDOW_H__

#include "ui_knotifytestview.h"
#include <QPointer>
#include <kxmlguiwindow.h>

class KNotification;

class KNotifyTestWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    KNotifyTestWindow(QWidget *parent = nullptr);

private:
    Ui::KNotifyTestView view;
    QPointer<KNotification> m_readNotif;
    int m_nbNewMessage;

public Q_SLOTS:
    void slotSendOnlineEvent();
    void slotSendMessageEvent();
    void slotMessageRead();

    void slotConfigureG();
};

#endif
