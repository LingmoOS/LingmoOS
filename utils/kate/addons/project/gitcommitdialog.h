/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

class QFont;

class GitCommitDialog : public QDialog
{
public:
    explicit GitCommitDialog(const QString &lastCommit, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    QString subject() const;
    QString description() const;
    bool signoff() const;
    bool amendingLastCommit() const;
    void setAmendingCommit();

private:
    void updateLineSizeLabel();
    void loadCommitMessage(const QString &lastCommit);

    QLineEdit m_le;
    QPlainTextEdit m_pe;
    QPushButton ok;
    QPushButton cancel;
    QLabel m_leLen;
    QLabel m_peLen;
    QCheckBox m_cbSignOff;
    QCheckBox m_cbAmend;
};
