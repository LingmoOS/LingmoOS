// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <DDialog>
#include <QHBoxLayout>
#include <DLabel>
#include <QTimer>

DWIDGET_USE_NAMESPACE

class ClientDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ClientDialog(QWidget *parent = nullptr);
    ~ClientDialog();

    int exec();

    int addButton(const QString &text, bool isDefault, ButtonType type);
    void setTitle(const QString &title);
    void setMessage(const QString& message);
    void setButtonText(int index, const QString &text);

    int buttonCount();

    void startBottomCloseTimer();
    QString closeTimerText(int num);

private:
    QHBoxLayout *m_buttonLayout;
    DLabel *m_titleLabel;
    DLabel *m_messageLabel;
    QList<QAbstractButton*> m_buttonList;

    int m_clickedButtonIndex;
    QTimer *m_closeTimer;
    QAbstractButton *m_defaultButton;
    QString m_defaultButtonText;
    int m_defaultButtonCount;

    DLabel *m_CloseLabel;
};

#endif // CLIENTDIALOG_H
