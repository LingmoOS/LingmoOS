/*
 * Copyright 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "appinputareamanager.h"
#include <KWindowSystem>

AppInputAreaManager::AppInputAreaManager(QObject *parent)
    : QObject(parent), dummyWidget_(nullptr), oneshotTimer_(nullptr) {
    KWindowSystem::setType(dummyWidget_.winId(), NET::Dock);
    oneshotTimer_.setSingleShot(true);
    connectSignal();
}

void AppInputAreaManager::connectSignal() {
    QObject::connect(&oneshotTimer_, &QTimer::timeout, this, [this]() {
        dummyWidget_.setGeometry(virtualKeyboardRect_);
        dummyWidget_.show();
        // 使用KWin接口调整工作区域，仅在X11下有效
        // 该接口对全屏应用无效
        // 该接口需在winId对象显示前后调用，否则可能不生效
        KWindowSystem::setExtendedStrut(dummyWidget_.winId(), 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, virtualKeyboardRect_.height(),
                                        virtualKeyboardRect_.x(),
                                        virtualKeyboardRect_.width() - 1);
    });
}

void AppInputAreaManager::raiseInputArea(const QRect &virtualKeyboardRect) {
    virtualKeyboardRect_ = virtualKeyboardRect;

    oneshotTimer_.start(SHOW_DELAY_TIME);
}

void AppInputAreaManager::fallInputArea() {
    KWindowSystem::setExtendedStrut(dummyWidget_.winId(), 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0);
    dummyWidget_.hide();
    oneshotTimer_.stop();
}
