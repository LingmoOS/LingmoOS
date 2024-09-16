// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DTextEdit>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QWidget>

#include "simplelabel.h"

class ErrorMessageWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ErrorMessageWidget(QWidget *parent = nullptr);
    QString title() {
        return m_titleLabel->text();
    }
    void setTitle(const QString title) {
        m_titleLabel->setText(title);
    }
    void setErrorLog(const QString msg) {
        m_errorTextEdit->setPlainText(msg);
    }

private:
    QVBoxLayout *m_mainLayout;
    SimpleLabel *m_iconLabel;
    SimpleLabel *m_titleLabel;
    DTextEdit   *m_errorTextEdit;

    void initUI();
};
