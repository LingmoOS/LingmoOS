/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "connect-server-dialog.h"
#include "ui_connect-server-dialog.h"
#include "global-settings.h"
#include <QPushButton>

ConnectServerDialog::ConnectServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectServerDialog)
{
    ui->setupUi(this);
    ui->passwd_edit->setEchoMode(QLineEdit::Password);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(ui->anonymous_checkbox, &QCheckBox::toggled, [=](bool checked) {
        if (checked) {
            this->ui->usr_edit->clear();
            this->ui->usr_edit->setEnabled(false);
            this->ui->passwd_edit->clear();
            this->ui->passwd_edit->setEnabled(false);
            if (COMMERCIAL_VERSION)
            {
                this->ui->domain_edit->clear();
                this->ui->domain_edit->setEnabled(false);
            }

            this->ui->save_passwd_checkbox->setChecked(false);
            this->ui->save_passwd_checkbox->setEnabled(true);
        } else {
            this->ui->usr_edit->clear();
            this->ui->usr_edit->setEnabled(true);
            this->ui->passwd_edit->clear();
            this->ui->passwd_edit->setEnabled(true);
            if (COMMERCIAL_VERSION)
            {
                this->ui->domain_edit->clear();
                this->ui->domain_edit->setEnabled(true);
            }
            this->ui->save_passwd_checkbox->setChecked(false);
            this->ui->save_passwd_checkbox->setEnabled(true);
        }
    });

    ui->passwd_edit->setEnabled(false);
    ui->anonymous_checkbox->setChecked(true);

    if (COMMERCIAL_VERSION)
    {
        this->ui->domain_edit->setEnabled(false);
        this->ui->domain_edit->setVisible(false);
        this->ui->label_3->setVisible(false);
        ui->passwd_edit->setEnabled(true);
        ui->anonymous_checkbox->setChecked(false);
    }
}

ConnectServerDialog::~ConnectServerDialog()
{
    disconnect();
    delete ui;
}

QString ConnectServerDialog::user()
{
    return ui->usr_edit->text();
}

QString ConnectServerDialog::password()
{
    return ui->passwd_edit->text();
}

QString ConnectServerDialog::domain()
{
    return ui->domain_edit->text();
}

bool ConnectServerDialog::savePassword()
{
    return ui->save_passwd_checkbox->isChecked();
}

bool ConnectServerDialog::anonymous()
{
    return ui->anonymous_checkbox->isChecked();
}
