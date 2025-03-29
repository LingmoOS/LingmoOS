/***************************************************************************
 *   Copyright 2021 Reion Wong <reion@lingmoos.com>                      *
 *   Copyright Ken <https://stackoverflow.com/users/1568857/ken>           *
 *   Copyright 2016 Leslie Zhai <xiangzhai83@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>

class ShortCut : public QObject
{
    Q_OBJECT

public:
    explicit ShortCut(QObject *parent = nullptr);

    Q_INVOKABLE void install(QObject *target = nullptr);

signals:
    void open();
    void copy();
    void cut();
    void paste();
    void rename();
    void refresh();
    void openPathEditor();
    void selectAll();
    void backspace();
    void deleteFile();
    void showHidden();
    void keyPressed(const QString &text);
    void close();
    void undo();
    void deleteFileForever();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    QObject *m_object;
};

#endif // SHORTCUT_H
