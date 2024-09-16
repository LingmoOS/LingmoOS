// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "publisherfunc.h"
#include <QDir>
#include <QFile>
#include <QTime>
#include <QImage>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>

namespace publisherFunc {

bool createFile(const QString &filePath, const QString &text, QIODevice::OpenMode model)
{
    QFile file(filePath);
    if (file.open(model)) {
        file.write(text.toLocal8Bit());
        file.close();
        return true;
    }
    return false;
}

QString readFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QString str = file.readAll();
        file.close();
        return str;
    }
    return QString();
}

QStringList getDirFileNames(const QString &dirPath, const QDir::Filters &flag, const QStringList &suffix)
{
    QDir dir(dirPath); //获取文件夹内所有文件名
    if (dir.exists()) {
        dir.setFilter(flag); //设置文件类型
        dir.setNameFilters(suffix); //设置文件后缀
        return dir.entryList();
    }
    return QStringList();
}

QString getImagePixel(const QString &imagePath)
{
    QImage image(imagePath);
    return QString("%1x%2").arg(image.width()).arg(image.height());
}

void pause(const unsigned int &msec)
{
    if (msec != 0) {
        QTime dieTime = QTime::currentTime().addMSecs(static_cast<int>(msec));
        while (QTime::currentTime() < dieTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
    }
}

QPair<int, QString> startPopen(const QString &cmd, const QString &model)
{
    QString res;
    char buf[1024];
    FILE *p_file = popen(cmd.toLocal8Bit(), model.toLocal8Bit());
    if (p_file) {
        while (fgets(buf, 1024, p_file) != nullptr) {
            res += QString(buf);
        }
    }
    int i = pclose(p_file);
    return QPair<int, QString>(i, res);
}

void moveToWindowCenter(QWidget *widget)
{
    //获取主屏幕分辨率
    QRect screenRect = QApplication::desktop()->screenGeometry();
    widget->move(screenRect.width() / 2 - widget->width() / 2, screenRect.height() / 2 - widget->height() / 2 - 100);
}

QJsonDocument readJson(const QString &filePath)
{
    //创建json对象，读取json文件
    QByteArray allData = publisherFunc::readFile(filePath).toLocal8Bit();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        return QJsonDocument();
    }
    return jsonDoc;
}

bool createDir(const QString &dir)
{
    QDir _dir(dir);
    if (_dir.exists(dir)) {
        return true;
    }
    return _dir.mkpath(dir);
}

bool removeDir(const QString &dir)
{
    QDir _dir(dir);
    if (_dir.exists(dir)) {
        _dir.removeRecursively();
        return true;
    }
    return false;
}

int fontSize(const QString &str, const QFont &font)
{
    if (str.isEmpty())
        return 0;
    QFontMetrics fontMetrics(font);
    return fontMetrics.width(str); //获取之前设置的字符串的像素大小
}

QString getKeyValue(const QKeyEvent *event)
{
    if (!event)
        return QString();

    return getKeyValue(event->key());
}

QString getKeyValue(const int &key)
{
    int keyValue = key; //数值表示意义
    QString keyValue_Hex = QString("0x%1").arg(keyValue, 2, 16, QLatin1Char('0')); //k为int型或char型都可
    QString keyValue_QT_KEY = QString(keyValue);

    //对于特殊意义的键值[event->key()的值无法用ASCII码展示]
    switch (keyValue) {
    case Qt::Key_Escape:
        keyValue_QT_KEY = QString("Esc");
        break;

    case Qt::Key_Tab:
        keyValue_QT_KEY = QString("Tab");
        break;

    case Qt::Key_CapsLock:
        keyValue_QT_KEY = QString("CapsLock");
        break;
    case Qt::Key_Shift:
        keyValue_QT_KEY = QString("Shift");
        break;
    case Qt::Key_Control:
        keyValue_QT_KEY = QString("Ctrl");
        break;
    case Qt::Key_Alt:
        keyValue_QT_KEY = QString("Alt");
        break;
    case Qt::Key_Backspace:
        keyValue_QT_KEY = QString("Backspace");
        break;
    case Qt::Key_Meta:
        keyValue_QT_KEY = QString("Super");
        break;
    case Qt::Key_Return:
        keyValue_QT_KEY = QString("Enter");
        break;
    case Qt::Key_Enter:
        keyValue_QT_KEY = QString("Enter");
        break;
    case Qt::Key_Home:
        keyValue_QT_KEY = QString("Home");
        break;
    case Qt::Key_End:
        keyValue_QT_KEY = QString("End");
        break;
    case Qt::Key_PageUp:
        keyValue_QT_KEY = QString("PageUp");
        break;
    case Qt::Key_PageDown:
        keyValue_QT_KEY = QString("PageDown");
        break;
    case Qt::Key_Insert:
        keyValue_QT_KEY = QString("Insert");
        break;
    case Qt::Key_Up:
        keyValue_QT_KEY = QString::fromLocal8Bit("↑");
        break;
    case Qt::Key_Right:
        keyValue_QT_KEY = QString::fromLocal8Bit("→");
        break;
    case Qt::Key_Left:
        keyValue_QT_KEY = QString::fromLocal8Bit("←");
        break;
    case Qt::Key_Down:
        keyValue_QT_KEY = QString::fromLocal8Bit("↓");
        break;
    case Qt::Key_Delete:
        keyValue_QT_KEY = QString("Del");
        break;
    case Qt::Key_Space:
        keyValue_QT_KEY = QString("Space");
        break;
    case Qt::Key_F1:
        keyValue_QT_KEY = QString("F1");
        break;
    case Qt::Key_F2:
        keyValue_QT_KEY = QString("F2");
        break;
    case Qt::Key_F3:
        keyValue_QT_KEY = QString("F3");
        break;
    case Qt::Key_F4:
        keyValue_QT_KEY = QString("F4");
        break;
    case Qt::Key_F5:
        keyValue_QT_KEY = QString("F5");
        break;
    case Qt::Key_F6:
        keyValue_QT_KEY = QString("F6");
        break;
    case Qt::Key_F7:
        keyValue_QT_KEY = QString("F7");
        break;
    case Qt::Key_F8:
        keyValue_QT_KEY = QString("F8");
        break;
    case Qt::Key_F9:
        keyValue_QT_KEY = QString("F9");
        break;
    case Qt::Key_F10:
        keyValue_QT_KEY = QString("F10");
        break;
    case Qt::Key_F11:
        keyValue_QT_KEY = QString("F11");
        break;
    case Qt::Key_F12:
        keyValue_QT_KEY = QString("F12");
        break;
    case Qt::Key_NumLock:
        keyValue_QT_KEY = QString("NumLock");
        break;
    case Qt::Key_ScrollLock:
        keyValue_QT_KEY = QString("ScrollLock");
        break;
    case Qt::Key_Pause:
        keyValue_QT_KEY = QString("Pause");
        break;
    case Qt::Key_Super_L:
        keyValue_QT_KEY = QString("Super");
        break;
    case Qt::Key_Super_R:
        keyValue_QT_KEY = QString("Super");
        break;
    }

    return keyValue_QT_KEY;
}

} // namespace publisherFunc
