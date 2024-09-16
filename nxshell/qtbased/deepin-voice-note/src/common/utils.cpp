// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "globaldef.h"
#include "vnoteitem.h"
#include "vnoteapplication.h"

#include <DGuiApplicationHelper>

#include <QImageReader>
#include <QIcon>
#include <QDebug>
#include <QTextBlock>
#include <QBuffer>
#include <QFileInfo>
#include <QProcess>

Utils::Utils()
{
}

/**
 * @brief Utils::convertDateTime
 * @param dateTime 时间
 * @return 格式化后的字符串
 */
QString Utils::convertDateTime(const QDateTime &dateTime)
{
    QDate currDateTime = QDate::currentDate();
    QDate folerDate = dateTime.date();
    QString disptime;
    qint64 offset = folerDate.daysTo(currDateTime);
    if (0 == offset) {
        qint64 offsetSec = dateTime.secsTo(QDateTime::currentDateTime());
        if (offsetSec < 3600) {
            offsetSec /= 60;
            if (offsetSec <= 1) {
                disptime = DApplication::translate("Utils", "1 min ago");
            } else {
                disptime = DApplication::translate("Utils", "%1 mins ago").arg(offsetSec);
            }
        } else {
            disptime = dateTime.toString("hh:mm");
        }
    } else if (1 == offset) {
        disptime = DApplication::translate("Utils", "Yesterday") + " " + dateTime.toString("hh:mm");
    } else if (folerDate.year() == currDateTime.year()) {
        //不跨年 其他时间：MM-DD（例如：9-27）；
        disptime = dateTime.toString("MM-dd");
    } else {
        //跨年:YYYY-MM-DD（例如2018-9-26）；
        disptime = dateTime.toString("yyyy-MM-dd");
    }

    return disptime;
}

/**
 * @brief Utils::renderSVG
 * @param filePath
 * @param size
 * @param pApp
 * @return 加载的图标
 */
QPixmap Utils::renderSVG(const QString &filePath, const QSize &size, DApplication *pApp)
{
    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        // const qreal ratio = qApp->devicePixelRatio();
        const qreal ratio = pApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    return pixmap;
}

/**
 * @brief Utils::loadSVG
 * @param fileName
 * @param fCommon 图标是否区分主题，true 不区分
 * @return 加载的图标
 */
QPixmap Utils::loadSVG(const QString &fileName, bool fCommon)
{
    DGuiApplicationHelper::ColorType theme =
        DGuiApplicationHelper::instance()->themeType();

    QString iconPath(STAND_ICON_PAHT);

    if (!fCommon) {
        if (DGuiApplicationHelper::ColorType::LightType == theme) {
            iconPath += QString("light/");
        } else {
            iconPath += QString("dark/");
        }
    }

    iconPath += fileName;

    qreal ratio = 1.0;

    const qreal devicePixelRatio = qGuiApp->devicePixelRatio();

    QPixmap pixmap;

    if (!qFuzzyCompare(ratio, devicePixelRatio)) {
        QImageReader reader;
        reader.setFileName(qt_findAtNxFile(iconPath, devicePixelRatio, &ratio));
        if (reader.canRead()) {
            reader.setScaledSize(reader.size() * (devicePixelRatio / ratio));
            pixmap = QPixmap::fromImage(reader.read());
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
    } else {
        pixmap.load(iconPath);
    }

    return pixmap;
}

/**
 * @brief Utils::highTextEdit
 * @param textDoc 编辑器的文档
 * @param searchKey 搜索关键字
 * @param highColor 高亮颜色
 * @param undo 是否取消文档上一步操作，true 取消
 * @return 搜索结果个数
 */
int Utils::highTextEdit(QTextDocument *textDoc, const QString &searchKey,
                        const QColor &highColor, bool undo)
{
    int findCount = 0;
    int len = searchKey.length();
    if (undo == true) {
        textDoc->undo();
    }
    if (len != 0) {
        QTextCursor highlightCursor(textDoc);
        QTextCursor cursor(textDoc);

        cursor.beginEditBlock();
        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setForeground(highColor);

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = textDoc->find(searchKey, highlightCursor,
                                            QTextDocument::FindFlags());

            if (!highlightCursor.isNull()) {
                int pos = highlightCursor.position();
                highlightCursor.setPosition(pos - len);
                highlightCursor.setPosition(pos, QTextCursor::KeepAnchor);
                highlightCursor.mergeCharFormat(colorFormat);
                findCount++;
            }
        }
        cursor.endEditBlock();
    }

    return findCount;
}

/**
 * @brief Utils::formatMillisecond
 * @param millisecond 毫秒
 * @param minValue 最小显示的时间
 * @return 格式化后的字符串
 */
QString Utils::formatMillisecond(qint64 millisecond, qint64 minValue)
{
    qint64 curSecond = millisecond / 1000;
    if (curSecond < minValue) {
        curSecond = minValue;
    }
    if (curSecond < 3600) {
        return QDateTime::fromTime_t(static_cast<uint>(curSecond)).toUTC().toString("mm:ss");
    }

    return QString("60:00");
}

/**
 * @brief Utils::documentToBlock
 * @param block 绑定的数据
 * @param doc 编辑器的文档
 */
void Utils::documentToBlock(VNoteBlock *block, const QTextDocument *doc)
{
    if (block != nullptr) {
        block->blockText = "";
    }

    if (doc != nullptr) {
        QTextBlock currentBlock = doc->begin();
        QTextBlock::iterator it;
        while (true) {
            for (it = currentBlock.begin(); !(it.atEnd());) {
                QTextFragment currentFragment = it.fragment();
                //                QTextImageFormat newImageFormat = currentFragment.charFormat().toImageFormat();
                //                if (newImageFormat.isValid()) {
                //                    int pos = currentFragment.position();
                //                    qDebug() << "image block:" << pos <<"url;" << newImageFormat.name();
                //                    ++it;
                //                    continue;
                //                }
                if (currentFragment.isValid()) {
                    ++it;
                    block->blockText.append(currentFragment.text());
                }
            }

            currentBlock = currentBlock.next();
            if (!currentBlock.isValid())
                break;

            block->blockText.append('\n');
        }
    }
}

/**
 * @brief Utils::blockToDocument
 * @param block 绑定的数据
 * @param doc 编辑器文档
 */
void Utils::blockToDocument(const VNoteBlock *block, QTextDocument *doc)
{
    if (block && doc) {
        doc->setPlainText(block->blockText);
    }
}

/**
 * @brief Utils::setDefaultColor
 * @param srcDoc 编辑器文档
 * @param color 字体颜色
 */
void Utils::setDefaultColor(QTextDocument *srcDoc, const QColor &color)
{
    QTextCursor cursor(srcDoc);
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QTextCharFormat newFormat = cursor.charFormat();
    newFormat.setForeground(color);
    cursor.mergeCharFormat(newFormat);
}

/**
 * @brief Utils::setTitleBarTabFocus
 * @param event
 */
void Utils::setTitleBarTabFocus(QKeyEvent *event)
{
    VNoteApplication *app = dynamic_cast<VNoteApplication *>(qGuiApp);
    if (app) {
        VNoteMainWindow *wnd = app->mainWindow();
        if (wnd) {
            wnd->setTitleBarTabFocus(event);
        }
    }
}

/**
 * @brief Utils::pictureToBase64
 * 图片转base64
 * @param imgPath 图片路径
 * @param base64 base64编码
 * @return  转换是否成功 true:成功， false:失败
 */
bool Utils::pictureToBase64(QString imgPath, QString &base64)
{
    //加载图片，目前的参考格式为（bmp, jpg, png, jpeg, gif, ico）
    QImage img(imgPath, "bmp, jpg, png, jpeg, gif, ico");
    //非本地文件返回空字符串
    if (img.isNull()) {
        return false;
    }

    QByteArray ba;
    QBuffer buf(&ba);
    QFileInfo fileInfo(imgPath);

    //设置图片最大宽度
    int width = qMin(img.width(), 712);
    int height = img.height();

    //等比例缩放图片
    img.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(&buf, qPrintable(fileInfo.suffix()));
    //图片转base64
    base64 = QString("data:image/%1;base64," + ba.toBase64()).arg(fileInfo.suffix());
    return true;
}

bool Utils::isLoongsonPlatform()
{
    static QString cpuModeName = "";
    if (cpuModeName.isEmpty()) {
        QProcess process;
        //获取CPU型号
        process.start("cat /proc/cpuinfo");
        if (process.waitForFinished()) {
            QString result = process.readAllStandardOutput();
            if (result.contains("Loongson")) {
                cpuModeName = "Loongson";
                return true;
            }
            cpuModeName = "other";
        }
    }
    return cpuModeName.contains("Loongson");
}

/**
 * @brief filteredFileName
 * 过滤不符合文件名规范的文件名内容，若过滤后结果为空则返回defaultName
 * @param fileName 待过滤的文件名
 * @return 过滤后的文件名
 */
QString Utils::filteredFileName(QString fileName, const QString &defaultName)
{
    //使用正则表达式除去不符合规范的字符并清理文件名两端的空白字符，中间的空白字符不做处理
    QString name = fileName.replace(QRegExp("[\"\'\\[\\]\\\\/:|<>+=;,?*]"), "").trimmed();
    if (name.isEmpty()) {
        return defaultName;
    }
    return name;
}

bool Utils::isWayland()
{
    return qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool();
}
