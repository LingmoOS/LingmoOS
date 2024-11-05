/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef X11WINDOWMANAGER_H
#define X11WINDOWMANAGER_H

#include <QObject>
#include <QPoint>

class X11WindowManager : public QObject
{
    Q_OBJECT
public:
    static X11WindowManager *getInstance();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void registerWidget(QWidget *widget);

private:
    explicit X11WindowManager(QObject *parent = nullptr);

    bool m_is_draging = false;
    ulong m_prepare_drag_time = 0;

    QWidget *m_current_widget = nullptr;

    QPoint m_press_pos;
    QPoint m_toplevel_offset;
};

struct UnityCorners {
    ulong topLeft = 0;
    ulong topRight = 0;
    ulong bottomLeft = 0;
    ulong bottomRight = 0;
};

typedef struct {
    ulong flags = 0;
    ulong functions = 0;
    ulong decorations = 0;
    long input_mode = 0;
    ulong status = 0;
} MotifWmHints, MwmHints;

#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_HINTS_INPUT_MODE    (1L << 2)
#define MWM_HINTS_STATUS        (1L << 3)

#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)

#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)

#define MWM_INPUT_MODELESS 0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL 1
#define MWM_INPUT_SYSTEM_MODAL 2
#define MWM_INPUT_FULL_APPLICATION_MODAL 3
#define MWM_INPUT_APPLICATION_MODAL MWM_INPUT_PRIMARY_APPLICATION_MODAL

#define MWM_TEAROFF_WINDOW	(1L<<0)

namespace LINGMO {
class Decoration;
}

class XAtomHelper : public QObject
{
    friend class LINGMO::Decoration;
    Q_OBJECT
public:
    static XAtomHelper *getInstance();

    static bool isFrameLessWindow(int winId);

    bool isWindowDecorateBorderOnly(int winId);
    bool isWindowMotifHintDecorateBorderOnly(const MotifWmHints &hint);
    bool isLINGMOCsdSupported();
    bool isLINGMODecorationWindow(int winId);

    UnityCorners getWindowBorderRadius(int winId);
    void setWindowBorderRadius(int winId, const UnityCorners &data);
    void setWindowBorderRadius(int winId, int topLeft, int topRight, int bottomLeft, int bottomRight);
    void setLINGMODecoraiontHint(int winId, bool set = true);

    void setWindowMotifHint(int winId, const MotifWmHints &hints);
    MotifWmHints getWindowMotifHint(int winId);

private:
    explicit XAtomHelper(QObject *parent = nullptr);

    unsigned long registerLINGMOCsdNetWmSupportAtom();
    void unregisterLINGMOCsdNetWmSupportAtom();

    unsigned long m_motifWMHintsAtom = 0l;
    unsigned long m_unityBorderRadiusAtom = 0l;
    unsigned long m_lingmoDecorationAtion = 0l;
};

#endif // X11WINDOWMANAGER_H
