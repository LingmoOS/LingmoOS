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

#ifndef PINCODEWIDGET_H
#define PINCODEWIDGET_H

#include <QWidget>
#include <QGuiApplication>
#include <QList>
#include <QLabel>
#include <QIcon>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QDialog>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPalette>
#include <QGSettings>
#include <QScreen>
#include <QPainter>
#include <QFile>
#include <QProcess>
#include <QKeyEvent>

#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"

class PinCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PinCodeWidget(QString name = "", QString pin = "", bool flag = true, QWidget* parent = nullptr);
    ~PinCodeWidget();
    void pairFailureShow();
    void updateUIInfo(const QString &name,const QString &pin);
    bool canceled = false;
public slots:

private slots:
    void onClick_close_btn(bool);
    void onClick_accept_btn(bool);
    void onClick_refuse_btn(bool);
    void GSettingsChanges(const QString &key);
signals:
    void accepted();
    void rejected();
    void msgShowedOver();
private:
    QGSettings  *settings    = nullptr;
    QGSettings *transparency_gsettings = nullptr;

    QLabel      *PIN_label   = nullptr;
    QLabel      *tip_label   = nullptr;
    QLabel      *top_label   = nullptr;
    QLabel      *warn_icon   = nullptr;
    QLabel      *title_icon   = nullptr;
    QLabel      *title_text   = nullptr;
    double tran = 1;

    QVBoxLayout *main_layout = nullptr;

    QPushButton *close_btn   = nullptr;
    QPushButton *accept_btn  = nullptr;
    QPushButton *refuse_btn  = nullptr;

    QString dev_name;
    QString PINCode;
    bool    show_flag;
    bool    is_Intel = false;
    double scale = 1.0;
    int pressCnt = 0;
    QPushButton *selectedBtn = nullptr;

    QPushButton *getButton();
protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // PINCODEWIDGET_H
