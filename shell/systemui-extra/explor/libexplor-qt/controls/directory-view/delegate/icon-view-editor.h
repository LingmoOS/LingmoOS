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

#ifndef ICONVIEWEDITOR_H
#define ICONVIEWEDITOR_H

#include <QTextEdit>

#include "explor-core_global.h"

class QLabel;
class QLineEdit;

namespace Peony {

namespace DirectoryView {

class PEONYCORESHARED_EXPORT IconViewEditor : public QTextEdit
{
    Q_OBJECT
public:
    explicit IconViewEditor(QWidget *parent = nullptr);
    ~IconViewEditor() override;

Q_SIGNALS:
    void returnPressed();

public Q_SLOTS:
    void minimalAdjust();
    void setMaxLengthLimit(int length);
    void setLimitBytes(bool limitBytes);

protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    QLineEdit *m_styled_edit;

    int m_max_length_limit = 0;
    bool m_limit_bytes = true;
};

}

}

#endif // ICONVIEWEDITOR_H
