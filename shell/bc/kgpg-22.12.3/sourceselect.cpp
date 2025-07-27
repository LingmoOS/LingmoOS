/*
    SPDX-FileCopyrightText: 2007 Lukas Kropatschek <lukas.krop@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sourceselect.h"

SrcSelect::SrcSelect(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	connect(checkFile, &QRadioButton::toggled, this, &SrcSelect::checkFile_toggled);
	connect(checkServer, &QRadioButton::toggled, this, &SrcSelect::checkServer_toggled);
}

void SrcSelect::checkFile_toggled(bool isOn)
{
	newFilename->setEnabled(isOn);
}

void SrcSelect::checkServer_toggled(bool isOn)
{
	keyIds->setEnabled(isOn);
}
