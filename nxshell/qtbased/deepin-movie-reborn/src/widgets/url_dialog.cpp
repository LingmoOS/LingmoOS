// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "url_dialog.h"
#include "dmr_lineedit.h"
#include "dmr_settings.h"


DWIDGET_USE_NAMESPACE

namespace dmr {
    UrlDialog::UrlDialog(QWidget *parent)
        : DDialog(parent)
    {
        addButtons(QStringList() << QApplication::translate("UrlDialog", "Cancel")
                   << QApplication::translate("UrlDialog", "OK"));
        setOnButtonClickedClose(false);
        setDefaultButton(1);
        setIcon(QIcon::fromTheme("deepin-movie"));
        setMessage(QApplication::translate("UrlDialog", "Please enter the URL:"));
        setFixedSize(380, 190);

        m_lineEdit = new LineEdit(this);
        addContent(m_lineEdit);
        m_lineEdit->setFocusPolicy(Qt::StrongFocus);
        this->setFocusProxy(m_lineEdit);

        if (m_lineEdit->text().isEmpty()) {
            getButton(1)->setEnabled(false);
        }

        connect(getButton(0), &QAbstractButton::clicked, this, [ = ] {
            done(QDialog::Rejected);
        });
        connect(getButton(1), &QAbstractButton::clicked, this, [ = ] {
            done(QDialog::Accepted);
        });
        connect(m_lineEdit, &QLineEdit::textChanged, this, &UrlDialog::slotTextchanged);
    }

    QUrl UrlDialog::url() const
    {
        auto u = QUrl(m_lineEdit->text(), QUrl::StrictMode);
        if (u.isLocalFile() || u.scheme().isEmpty())
            return QUrl();

        if (!Settings::get().iscommonPlayableProtocol(u.scheme()))
            return QUrl();

        return u;
    }

    void UrlDialog::showEvent(QShowEvent *se)
    {
        m_lineEdit->setFocus();

        DDialog::showEvent(se);
    }

    void UrlDialog::slotTextchanged()
    {
        if (m_lineEdit->text().isEmpty()) {
            getButton(1)->setEnabled(false);
        } else {
            getButton(1)->setEnabled(true);
        }
    }
}

