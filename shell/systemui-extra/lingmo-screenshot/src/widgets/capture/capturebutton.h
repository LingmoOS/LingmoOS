/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*             2020 KylinSoft Co., Ltd.
 * This file is part of Lingmo-Screenshot.
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

#pragma once

#include <QPushButton>
#include <QMap>
#include <QVector>
#include <QLabel>
#include <QHBoxLayout>
#include "src/tools/capturecontext.h"

class QWidget;
class QPropertyAnimation;
class CaptureTool;

class CaptureButton : public QPushButton {
    Q_OBJECT

public:
    // Don't forget to add the new types to CaptureButton::iterableButtonTypes
    // in the .cpp and the order value in the private array buttonTypeOrder
    enum ButtonType
    {
#ifdef ENABLE_RECORD
        TYPE_CUT,
        TYPE_LUPING,
#endif
        TYPE_RECT,
        TYPE_CIRCLE,
        TYPE_LINE,
        TYPE_ARROW,
        TYPE_PEN,
        TYPE_MARKER,
        TYPE_TEXT,
        TYPE_BLUR,
        TYPR_UNDO,
#ifndef SUPPORT_NEWUI
        TYPE_OPTION,
#endif
        TYPE_CLOSE,
        TYPE_COPY,
        TYPE_SAVE,
#ifndef SUPPORT_NEWUI
        TYPE_SAVEAS,
#endif
        TYPE_PIN,
#ifdef ENABLE_RECORD
        TYPE_RECORD_CURSOR,
        TYPE_RECORD_AUDIO,
        TYPE_RECORD_FOLLOW_MOUSE,
        TYPE_RECORD_OPTION,
//        TYPE_RECORD_CLOSE,
        TYPE_RECORD_START,
#endif
    };


#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    Q_ENUMS(ButtonType)
#else
    Q_ENUM(ButtonType)
#endif

    CaptureButton() = delete;
    explicit CaptureButton(const ButtonType, QWidget *parent = nullptr);

    static QString globalStyleSheet();
    static QVector<CaptureButton::ButtonType> getIterableButtonTypes();
    static int getPriorityByButton(CaptureButton::ButtonType);

    QString name() const;
    QString description() const;
    QIcon icon() const;
    QString styleSheet() const;
    CaptureTool* tool() const;

    void setColor(const QColor &c);
    void animatedShow();
    ButtonType m_buttonType;
protected:
    virtual void mousePressEvent(QMouseEvent *);
    static QVector<ButtonType> iterableButtonTypes;

    CaptureTool *m_tool;
    CaptureContext m_context;
signals:
    void pressedButton(CaptureButton *);

private:
    CaptureButton(QWidget *parent = nullptr);

    QPropertyAnimation *m_emergeAnimation;

    static QColor m_mainColor;

    void initButton();
    void updateIcon();
};
