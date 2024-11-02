/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELECTWIDGET_H
#define SELECTWIDGET_H

#include <QDialog>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QGSettings>
#include "uslider.h"
#include "../controls/klabel.h"

#include <kpushbutton.h>
using namespace kdk;

class QLabel;
class QSlider;
class QPushButton;

class ReniceDialog : public QDialog
{
    Q_OBJECT
public:
    ReniceDialog(const QString &procName = "", const QString &procId = "", QWidget *parent = 0);
    ~ReniceDialog();

    void loadData(int nice);

public slots:
    void onClose();
    void onThemeFontChange(qreal lfFontSize);

signals:
    void resetReniceValue(int value);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    void initThemeStyle();

private:
    QVBoxLayout *m_mainLayout = nullptr;

    QHBoxLayout *h_layout = nullptr;
    QHBoxLayout *tip_layout = nullptr;
    QHBoxLayout *btn_layout = nullptr;
    QLabel *m_titleLabel = nullptr;
    USlider *m_slider = nullptr;
    QLabel *m_valueLabel = nullptr;
    QLabel *m_valueStrLabel = nullptr;
    KLabel *m_tipLabel = nullptr;
    QString m_strProcName;
    QString m_strProcId;
    qreal fontSize;
    QGSettings *styleSettings;

    KPushButton *m_cancelbtn = nullptr;
    KPushButton *m_changeBtn = nullptr;

    QPoint m_dragPosition; //移动的距离
    bool m_mousePressed; //按下鼠标左键
};

#endif // SELECTWIDGET_H
