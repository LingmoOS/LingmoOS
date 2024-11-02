/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */


#ifdef LINGMO_FILE_DIALOG

#ifndef KYFILEOPERATIONDIALOG_H
#define KYFILEOPERATIONDIALOG_H

#include <QWidget>
#include <lingmosdk/applications/kdialog.h>

class KyFileOperationDialog : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit KyFileOperationDialog(QWidget *parent = nullptr);
};

#endif // KYFILEOPERATIONDIALOG_H

#endif
