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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include "explor-core_global.h"

class QLabel;
class QToolBar;

namespace Peony {

class FMWindowIface;

class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(FMWindowIface *window, QWidget *parent = nullptr);
    ~StatusBar() override;

public Q_SLOTS:
    void update();
    void update(const QString &message);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    FMWindowIface *m_window = nullptr;

    QLabel *m_label = nullptr;
    QToolBar *m_styled_toolbar = nullptr;
};

}

#endif // STATUSBAR_H
