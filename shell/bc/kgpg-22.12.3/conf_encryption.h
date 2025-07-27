/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONF_ENCRYPTION_H
#define CONF_ENCRYPTION_H

#include "ui_conf_encryption.h"

class Encryption : public QWidget, public Ui_Encryption
{

public:
    explicit Encryption(QWidget* parent = nullptr);
    
public:
    virtual void encrypt_to_always_toggled( bool );
    virtual void encrypt_files_to_toggled( bool );
    virtual void allow_custom_option_toggled( bool );

private:
   
};

#endif
