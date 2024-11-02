/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "noteExitWindow.h"
#include "ui_noteExitWindow.h"
#include "widget.h"
#include "ui_widget.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

noteExitWindow::noteExitWindow(Widget* page, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::noteExitWindow)
{
    ui->setupUi(this);
    pNotebook = page;

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    QPainterPath blurPath;
    blurPath.addRoundedRect(rect().adjusted(6, 6, -6, -6), 6, 6);      //增加圆角
    setProperty("useSystemStyleBlur", true);
    setProperty("blurRegion", QRegion(blurPath.toFillPolygon().toPolygon()));//使用QPainterPath的api生成多边形Region

    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(bmp.rect(),6,6);
    setMask(bmp);
}

noteExitWindow::~noteExitWindow()
{
    delete ui;
}

void noteExitWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;

    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(6, 6, -6, -6), 6, 6);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    //p.setOpacity(0.7);
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();
}

void noteExitWindow::on_pushButton_clicked()
{
    this->close();
}

void noteExitWindow::on_pushButton_2_clicked()
{
    this->close();
    pNotebook->close();
    if(ui->checkBox->isChecked()==true){
        for (auto it = pNotebook->m_editors.begin(); it!= pNotebook->m_editors.end();it++) {
            (*it)->close();
            //qDebug() << "before swap" << pNotebook->m_editors.size();
            //std::vector<Edit_page*> tmp;
            //pNotebook->m_editors.swap(tmp);
            //qDebug() << "after swap" << pNotebook->m_editors.size();
        }
    }
}

void noteExitWindow::exitImmediate()
{
    this->close();
    pNotebook->close();
//    if(ui->checkBox->isChecked()==true){
//        for (auto it = pNotebook->m_editors.begin(); it!= pNotebook->m_editors.end();it++) {
//            (*it)->close();
//        }
//    }
}
