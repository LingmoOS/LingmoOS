/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "conf_encryption.h"

Encryption::Encryption( QWidget* parent )
    : QWidget( parent ), Ui_Encryption()
{
    setupUi( this );
    connect(kcfg_EncryptFilesTo, &QCheckBox::toggled, this, &Encryption::encrypt_files_to_toggled);
    connect(kcfg_AllowCustomEncryptionOptions, &QCheckBox::toggled, this, &Encryption::allow_custom_option_toggled);
    connect(encrypt_to_always, &QCheckBox::toggled, this, &Encryption::encrypt_to_always_toggled);
}

void Encryption::encrypt_to_always_toggled(bool isOn)
{
    always_key->setEnabled(isOn);
}


void Encryption::encrypt_files_to_toggled(bool isOn)
{
    file_key->setEnabled(isOn);
}


void Encryption::allow_custom_option_toggled(bool isOn)
{
    kcfg_CustomEncryptionOptions->setEnabled(isOn);
}
