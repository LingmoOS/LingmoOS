// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINEBAR_H
#define LINEBAR_H

#include "dlineedit.h"

#include <QTimer>

DWIDGET_USE_NAMESPACE

class LineBar : public DLineEdit
{
    Q_OBJECT

public:
    explicit LineBar(DLineEdit *parent = 0);

public slots:
    void handleTextChangeTimer();
    void handleTextChanged(const QString &str="");
    void sendText(QString t);

signals:
    void contentChanged();
    void focusOut();
    void pressAltEnter();
    void pressCtrlEnter();
    void pressEnter();
    void pressMetaEnter();
    void signal_sentText(QString t);

protected:
    virtual void focusOutEvent(QFocusEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);

private:
    QTimer *m_autoSaveTimer;
    int m_autoSaveInternal;
};

#endif
