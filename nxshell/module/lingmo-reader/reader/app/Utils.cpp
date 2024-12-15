// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Utils.h"
#include "Application.h"

#include <QMimeDatabase>
#include <QPainter>
#include <QStandardPaths>
#include <QPixmap>
#include <QClipboard>
#include <QUuid>
#include <QFileInfo>
#include <QImageReader>
#include <QWidget>
#include <QProcessEnvironment>
#include <QPainterPath>

#include <unistd.h>
extern "C" {
#include "load_libs.h"
}
QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

int Utils::m_colorIndex = 0;
QString Utils::m_currenFilePath = "";
QString Utils::getKeyshortcut(QKeyEvent *keyEvent)
{
    QStringList keys;
    Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
    if (modifiers != Qt::NoModifier) {
        if (modifiers.testFlag(Qt::ControlModifier)) {
            keys.append("Ctrl");
        }

        if (modifiers.testFlag(Qt::AltModifier)) {
            keys.append("Alt");
        }

        if (modifiers.testFlag(Qt::MetaModifier)) {
            keys.append("Meta");
        }

        if (modifiers.testFlag(Qt::ShiftModifier)) {
            keys.append("Shift");
        }
    }

    if (keyEvent->key() != 0 && keyEvent->key() != Qt::Key_unknown) {
        keys.append(QKeySequence(keyEvent->key()).toString());
    }

    return keys.join("+");
}

QString Utils::getInputDataSize(const qint64 &dSize)
{
    if (dSize < 1024) {
        return QString("%1 B").arg(dSize);
    }
    if (dSize < 1024 * 1024) {
        double d = dSize / 1024.0;
        return QString::number(d, 'f', 1) + " KB";
    }
    if (dSize < 1024 * 1024 * 1024) {
        double d = dSize / 1024.0 / 1024.0;
        return QString::number(d, 'f', 1) + " MB";
    }
    double d = dSize / 1024.0 / 1024.0 / 1024.0;
    return QString::number(d, 'f', 1) + " GB";
}

QPixmap Utils::roundQPixmap(const QPixmap &img_in, int radius)
{
    if (img_in.isNull()) {
        return QPixmap();
    }
    const QSize &imgSize = img_in.size();
    QPixmap pixmap(imgSize);
    pixmap.setDevicePixelRatio(dApp->devicePixelRatio());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath clippath;
    clippath.addRoundedRect(0, 0, imgSize.width() / dApp->devicePixelRatio(), imgSize.height() / dApp->devicePixelRatio(), radius, radius);
    painter.setClipPath(clippath);
    painter.drawPixmap(0, 0, img_in);
    return pixmap;
}

void  Utils::copyText(const QString &sText)
{
#if _ZPD_
    int intercept = 0;
    if (getLoadLibsInstance()->m_document_clip_copy) {
        qInfo() << "当前文档: *** "/* <<m_currenFilePath*/;
        getLoadLibsInstance()->m_document_clip_copy(m_currenFilePath.toLocal8Bit().data(), &intercept);
        qInfo() << "是否拦截不允许复制(1:拦截 0:不拦截): " << intercept;
    }
    if (intercept > 0)
        return;
#endif
    QClipboard *clipboard = DApplication::clipboard();
    QString sOldText = clipboard->text(QClipboard::Clipboard);
    if (sOldText != sText) {
        clipboard->setText(sText);
    }
}

QString Utils::getElidedText(const QFontMetrics &fontMetrics, const QSize &size, const QString &text, Qt::Alignment alignment)
{
    int textLength = text.length();
    QString tmptext;
    for (int index = 0; index < textLength; index++) {
        const QString &ltext = text.left(index + 1);
        if (fontMetrics.boundingRect(0, 0, size.width(), size.height(), static_cast<int>(alignment | Qt::TextWrapAnywhere), ltext + QString("...")).height() > size.height()) {
            break;
        }
        tmptext = ltext;
    }
    if (tmptext.length() < textLength)
        tmptext.append("...");
    return tmptext;
}

bool Utils::copyFile(const QString &sourcePath, const QString &destinationPath)
{
    if (sourcePath == destinationPath)
        return true;

    QFile sourceFile(sourcePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QFile destinationFile(destinationPath);
    if (!destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        sourceFile.close();
        return false;
    }

    const qint64 maxSize = 4096;
    qint64 size = -1;

    QScopedArrayPointer< char > buffer(new char[maxSize]);

    int ret = true;
    do {
        if ((size = sourceFile.read(buffer.data(), maxSize)) < 0) {
            ret = false;
            break;
        }

        if (destinationFile.write(buffer.data(), size) < 0) {
            ret = false;
            break;
        }
    } while (size > 0);

    sourceFile.close();

    destinationFile.flush();//函数将用户缓存中的内容写入内核缓冲区
    fsync(destinationFile.handle());//将内核缓冲写入文件(磁盘)
    destinationFile.close();
    return ret;
}

QImage Utils::copyImage(const QImage &srcimg, int x, int y, int w, int h)
{
    int dx = 0;
    int dy = 0;
    if (w <= 0 || h <= 0)
        return QImage();

    QImage::Format format = srcimg.format();
    QImage image(w, h, format);
    if (image.isNull())
        return image;

    int srcw = srcimg.width();
    int srch = srcimg.height();
    if (x < 0 || y < 0 || x + w > srcw || y + h > srch) {
        // bitBlt will not cover entire image - clear it.
        image.fill(Qt::white);
        if (x < 0) {
            dx = -x;
            x = 0;
        }
        if (y < 0) {
            dy = -y;
            y = 0;
        }
    }

    image.setColorTable(srcimg.colorTable());

    int pixels_to_copy = qMax(w - dx, 0);
    if (x > srcw)
        pixels_to_copy = 0;
    else if (pixels_to_copy > srcw - x)
        pixels_to_copy = srcw - x;
    int lines_to_copy = qMax(h - dy, 0);
    if (y > srch)
        lines_to_copy = 0;
    else if (lines_to_copy > srch - y)
        lines_to_copy = srch - y;

    bool byteAligned = true;
    if (format == QImage::Format_Mono || format == QImage::Format_MonoLSB)
        byteAligned = !(dx & 7) && !(x & 7) && !(pixels_to_copy & 7);

    if (byteAligned) {
        const uchar *src = srcimg.bits() + ((x * srcimg.depth()) >> 3) + y * srcimg.bytesPerLine();
        uchar *dest = image.bits() + ((dx * srcimg.depth()) >> 3) + dy * image.bytesPerLine();
        const int bytes_to_copy = (pixels_to_copy * srcimg.depth()) >> 3;
        for (int i = 0; i < lines_to_copy; ++i) {
            memcpy(dest, src, static_cast<size_t>(bytes_to_copy));
            src += srcimg.bytesPerLine();
            dest += image.bytesPerLine();
        }
    } else if (format == QImage::Format_Mono) {
        const uchar *src = srcimg.bits() + y * srcimg.bytesPerLine();
        uchar *dest = image.bits() + dy * image.bytesPerLine();
        for (int i = 0; i < lines_to_copy; ++i) {
            for (int j = 0; j < pixels_to_copy; ++j) {
                if (src[(x + j) >> 3] & (0x80 >> ((x + j) & 7)))
                    dest[(dx + j) >> 3] |= (0x80 >> ((dx + j) & 7));
                else
                    dest[(dx + j) >> 3] &= ~(0x80 >> ((dx + j) & 7));
            }
            src += srcimg.bytesPerLine();
            dest += image.bytesPerLine();
        }
    } else { // Format_MonoLSB
        Q_ASSERT(format == QImage::Format_MonoLSB);
        const uchar *src = srcimg.bits() + y * srcimg.bytesPerLine();
        uchar *dest = image.bits() + dy * image.bytesPerLine();
        for (int i = 0; i < lines_to_copy; ++i) {
            for (int j = 0; j < pixels_to_copy; ++j) {
                if (src[(x + j) >> 3] & (0x1 << ((x + j) & 7)))
                    dest[(dx + j) >> 3] |= (0x1 << ((dx + j) & 7));
                else
                    dest[(dx + j) >> 3] &= ~(0x1 << ((dx + j) & 7));
            }
            src += srcimg.bytesPerLine();
            dest += image.bytesPerLine();
        }
    }

    image.setDotsPerMeterX(srcimg.dotsPerMeterX());
    image.setDotsPerMeterY(srcimg.dotsPerMeterY());
    image.setDevicePixelRatio(srcimg.devicePixelRatio());
    image.setOffset(srcimg.offset());
    return image;
}

QList<QColor> Utils::getHiglightColorList()
{
    QList<QColor> listColor;
    listColor.append(QColor("#FFA503"));
    listColor.append(QColor("#FF1C49"));
    listColor.append(QColor("#9023FC"));
    listColor.append(QColor("#3468FF"));
    listColor.append(QColor("#00C7E1"));
    listColor.append(QColor("#05EA6B"));
    listColor.append(QColor("#FEF144"));
    listColor.append(QColor("#D5D5D1"));
    return listColor;
}

void Utils::setHiglightColorIndex(int index)
{
    if (m_colorIndex < 0 || m_colorIndex >= getHiglightColorList().size())
        return;
    m_colorIndex = index;
}

QColor Utils::getCurHiglightColor()
{
    const QList<QColor> &colorlst =  getHiglightColorList();
    int index = qBound(0, m_colorIndex, colorlst.size() - 1);
    return colorlst.at(index);
}

void Utils::setObjectNoFocusPolicy(QObject *obj)
{
    QList<QWidget *> list = obj->findChildren<QWidget *>();
    foreach (QWidget *w, list) {
        if (w->objectName().isEmpty())
            w->setFocusPolicy(Qt::NoFocus);
    }
}

bool Utils::isWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    } else {
        return false;
    }
}

void Utils::setCurrentFilePath(QString currentFilePath)
{
    m_currenFilePath = currentFilePath;
}
