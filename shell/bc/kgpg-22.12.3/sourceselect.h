/*
    SPDX-FileCopyrightText: 2007 Lukas Kropatschek <lukas.krop@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SOURCESELECT_H
#define SOURCESELECT_H

#include "ui_sourceselect.h"

class SrcSelect : public QWidget, public Ui::SrcSelect
{
	Q_OBJECT

public:
	explicit SrcSelect(QWidget *parent = nullptr);

private Q_SLOTS:
	void checkFile_toggled(bool isOn);
	void checkServer_toggled(bool isOn);
};

#endif  // SOURCESELECT_H
