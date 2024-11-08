/*
 * Copyright (C) 2021 CuteOS.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CALCENGINE_H
#define CALCENGINE_H

#include <QObject>
#include "engine/evaluator.h"

class CalcEngine : public QObject
{
    Q_OBJECT

public:
    explicit CalcEngine(QObject *parent = nullptr);

    Q_INVOKABLE QString eval(const QString &expr);

signals:
    void failed(const QString &errorString);

private:
    Evaluator *m_evaluator;
};

#endif // CALCENGINE_H
