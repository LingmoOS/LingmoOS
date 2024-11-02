/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef ACTIVECONNECTIONWIDGET_H
#define ACTIVECONNECTIONWIDGET_H

#include <QIcon>
#include <QLabel>
#include <QTimer>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QCheckBox>
#include <QGSettings>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"

class ActiveConnectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActiveConnectionWidget(QString address, QString name, QString type, int rssi);
    ~ActiveConnectionWidget();
private:
    double scale;

    int devRSSI;
    int pressCnt = 0;
    double tran =1;

    bool mutex = false;

    QString devAddr;
    QString devName;
    QString devType;

    QGSettings  *settings     = nullptr;
    QGSettings *transparency  = nullptr;

    QLabel      *window_icon  = nullptr;
    QLabel      *window_title = nullptr;
    QLabel      *tip_txt      = nullptr;
    QLabel      *checkBox_txt = nullptr;

    QPushButton *window_close = nullptr;
    QPushButton *connect_btn  = nullptr;
    QPushButton *cancel_btn   = nullptr;
    QPushButton *selectedBtn  = nullptr;

    QCheckBox   *actCnt_box   = nullptr;

    QPushButton *getButton();
    void defaultStyleInit();
    void getTransparency();

private slots:
    void onClick_close_btn();
    void onClick_connect_btn();
    void onClick_cancel_btn();
    void GSettingsChanges(const QString &key);

public slots:
    void autoConnChanged(bool activate);

signals:
    void replyActiveConnection(QString,bool);

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void focusInEvent(QFocusEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
};

#endif // ACTIVECONNECTIONWIDGET_H
