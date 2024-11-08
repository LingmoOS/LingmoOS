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

#ifndef PATHEDIT_H
#define PATHEDIT_H

#include <QLineEdit>
#include "explorer-core_global.h"

namespace Peony {

class PathBarModel;
class PathCompleter;

class PEONYCORESHARED_EXPORT PathEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PathEdit(QWidget *parent = nullptr);
    void cancelList();

Q_SIGNALS:
    void uriChangeRequest(const QString &uri);
    void editCancelled();

public Q_SLOTS:
    void setUri(const QString &uri);


protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    QString m_last_uri;

    PathBarModel *m_model;
    PathCompleter *m_completer;

    bool m_right_click = false;
};

}

#endif // PATHEDIT_H
