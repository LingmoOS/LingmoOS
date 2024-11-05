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


#ifndef RENAMEEDITOR_H
#define RENAMEEDITOR_H

#include <QTextEdit>

class RenameEditor: public QTextEdit
{
    Q_OBJECT
public:
    explicit RenameEditor(QWidget *parent = nullptr);

Q_SIGNALS:
    void returnPressed();

protected:

    void keyPressEvent(QKeyEvent *e) override;

};

#endif // RENAMEEDITOR_H
