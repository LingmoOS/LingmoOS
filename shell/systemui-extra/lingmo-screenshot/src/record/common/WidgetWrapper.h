/*
 * Copyright (c) 2012-2020 Maarten Baert <maarten-baert@hotmail.com>

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
#ifndef WIDGETWRAPPER_H
#define WIDGETWRAPPER_H


#include "Global.h"
#include "my_qt.h"


class QComboBoxWithSignal : public QComboBox {
    Q_OBJECT

public:
    QComboBoxWithSignal(QWidget* parent);

    virtual void showPopup() override;
    virtual void hidePopup() override;

signals:
    void popupShown();
    void popupHidden();

};
class QSpinBoxWithSignal : public QSpinBox {
    Q_OBJECT
public:
    QSpinBoxWithSignal(QWidget* parent);
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
signals:
    void focusIn();
    void focusOut();

};
class ScreenLabelWindow : public QWidget {
    Q_OBJECT
private:
    QString m_text;
    QFont m_font;

public:
    ScreenLabelWindow(QWidget* parent, const QString& text);

protected:
    virtual void paintEvent(QPaintEvent* event) override;

};

class RecordingFrameWindow : public QWidget {
    Q_OBJECT

private:
    QPixmap m_texture;

public:
    RecordingFrameWindow(QWidget* parent);

private:
    void UpdateMask();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

};



class WidgetWrapper
{
public:
    WidgetWrapper();
};

#endif // WIDGETWRAPPER_H
