/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
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
 */
#ifndef CAPTURETOOLCAP_H
#define CAPTURETOOLCAP_H
#include "src/tools/capturecontext.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include <QIcon>
#include <QPainter>

class CaptureToolCap  : public QObject {

    Q_OBJECT
public:
    enum Request {
        REQ_SCREENCUT,
        REQ_SCRRENCAP,
        REQ_SCREENMOUSECURSOR,
        REQ_VOICE,
        REQ_MOUSE,
        REQ_SCREENCAPOPTION,
        REQ_SCREENCAPEXIT,
        REQ_SCREENSTAR,
     };
    explicit CaptureToolCap(QObject *parent = nullptr) : QObject(parent){}
    virtual bool isValid() const = 0;
    virtual QString name() const = 0;
    static QString nameID();
    virtual QString description() const = 0;
};
#endif // CAPTURETOOLCAP_H
