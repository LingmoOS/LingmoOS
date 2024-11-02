/*
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */
#ifndef ACTIONLABEL_H
#define ACTIONLABEL_H

#include <QObject>
#include <QLabel>
#include <QEvent>
#include <kborderlessbutton.h>
namespace LingmoUISearch {
class ActionLabel : public kdk::KBorderlessButton
{
    Q_OBJECT
public:
    ActionLabel(const QString &action, const QString &key, QWidget *parent = nullptr);
    ~ActionLabel() = default;

Q_SIGNALS:
    void actionTriggered(QString &action);

protected:
    bool eventFilter(QObject *, QEvent *);

private:
    void initUi();
    QString m_action;
    QString m_key;


};
}

#endif // ACTIONLABEL_H
