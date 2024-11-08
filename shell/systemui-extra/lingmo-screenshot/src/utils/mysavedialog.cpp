/*
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
#include "mysavedialog.h"
#include <QLineEdit>
#include <QComboBox>
#include <QDebug>
#include <QStandardPaths>
#include "src/utils/filenamehandler.h"

MySaveDialog::MySaveDialog(QWidget *parent) :
    QFileDialog(parent)
{
    // setWindowIcon(QIcon::fromTheme("lingmo-screenshot"));
    setNameFilter(QLatin1String(" PNG(*.png);;BMP(*.bmp);;JPEG(*.jpg)"));
    setDirectory(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0));
    setAcceptMode(QFileDialog::AcceptSave);
    setDefaultSuffix("png");
    setLabelText(QFileDialog::Reject, tr("Cancel"));
    QString a = FileNameHandler().parsedPattern()+ ".png";
    //    this->findChildren<QLineEdit *>("fileNameEdit").at(0)->setText(a);
    //修改1
    QString path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
    if(path.endsWith("/"))
        path += a;
    else
        path += "/" + a;
    this->selectFile(path);

    //filterSelected   修改2 可以改为响应这个信号
    connect(this->findChildren<QComboBox *>("fileTypeCombo").at(0),
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index)
    {
        switch (index) {
        case 0:
            qDebug()<<"change the type is png";
            setDefaultSuffix("png");
            break;
        case 1:
            setDefaultSuffix("bmp");
            qDebug()<<"change the type is bmp";
            break;
        case 2:
            setDefaultSuffix("jpg");
            qDebug()<<"change the type is jpg";
            break;
        default:
            setDefaultSuffix("png");
            break;
        }
        //        this->findChildren<QLineEdit *>("fileNameEdit").at(0)->setText(a);
        //修改3
        this->selectFile(path);
    });
}

QString MySaveDialog::filename()
{
    //    return this->findChildren<QLineEdit *>("fileNameEdit").at(0)->text();
    //修改4
    return this->selectedFiles()[0];
}
