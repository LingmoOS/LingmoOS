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

#ifndef APPINPUTAREAMANAGER_H
#define APPINPUTAREAMANAGER_H

#include <QObject>
#include <QRect>
#include <QTimer>
#include <QWidget>

class AppInputAreaManager : public QObject {
    Q_OBJECT

public:
    explicit AppInputAreaManager(QObject *parent = nullptr);
    ~AppInputAreaManager() = default;
    // TODO(linyuxuan): 使用kdk重新实现该函数
    void raiseInputArea(const QRect &virtualKeyboardRect);
    void fallInputArea();

private:
    void connectSignal();

private:
    QRect virtualKeyboardRect_;
    QWidget dummyWidget_;
    QTimer oneshotTimer_;
    // 经过指定时间之后顶起应用程序，避免桌面在虚拟键盘显示之前可见
    // 根据使用经验，选定200毫秒作为经验值
    static const int SHOW_DELAY_TIME = 200;
};

#endif // APPINPUTAREAMANAGER_H
