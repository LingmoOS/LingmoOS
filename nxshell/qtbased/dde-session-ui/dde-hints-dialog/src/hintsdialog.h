// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HINTSDIALOG_H
#define HINTSDIALOG_H

#include <DAbstractDialog>
#include <DTipLabel>
#include <DDialogCloseButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class QLabel;

class HintsDialog : public DAbstractDialog
{
    Q_OBJECT

public:
    explicit HintsDialog(QWidget *parent = nullptr);
    ~HintsDialog() override;
    void setHintTitle(const QString &title);
    void setHintContent(const QString &message);

protected:
    void showEvent(QShowEvent *event) override;

private:
    const int windowFixedWidth = 400;
    QLabel *m_title;
    DTipLabel *m_content;
    DDialogCloseButton *m_closeButton;
};

#endif // HINTSDIALOG_H
