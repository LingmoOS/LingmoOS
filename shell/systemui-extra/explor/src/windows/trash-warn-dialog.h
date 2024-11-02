/*
 * Peony-Qt
 *
 * Copyright (C) 2021, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#ifndef TRASHWARNDIALOG_H
#define TRASHWARNDIALOG_H

#include "QDialog"

namespace Peony {

class TrashWarnDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TrashWarnDialog(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

}


#endif //TRASHWARNDIALOG_H
