// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseutils.h"
#include "imageutils.h"
#include <stdio.h>
#include <fcntl.h>
#include <fstream>
#include <linux/fs.h>

#include <QApplication>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopServices>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDir>
#include <QFontMetrics>
#include <QFileInfo>
#include <QImage>
#include <QMimeData>
#include <QProcess>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QTextStream>
#include <QtMath>
#include <QImageReader>
#include <QTranslator>
#include <QDirIterator>

#include <DApplication>
#include <DDesktopServices>


#ifdef USE_UNIONIMAGE
#include "unionimage.h"
#endif
DWIDGET_USE_NAMESPACE

// 当前动态库使用的翻译QM翻译文件路径
#define PLUGIN_QM_TRANS_PATH "/usr/share/libimageviewer/translations"

namespace Libutils {

namespace base {

const QString DATETIME_FORMAT_NORMAL = "yyyy.MM.dd";
const QString DATETIME_FORMAT_EXIF = "yyyy:MM:dd HH:mm:ss";

QPixmap renderSVG(const QString &filePath, const QSize &size)
{
    /*lmh0724使用USE_UNIONIMAGE*/
#ifdef USE_UNIONIMAGE
    QImage tImg(size, QImage::Format_ARGB32);
    QString errMsg;
    QSize realSize;
//    if (!UnionImage_NameSpace::loadStaticImageFromFile(filePath, tImg, realSize, errMsg)) {
//        qDebug() << errMsg;
//    }
    if (!LibUnionImage_NameSpace::loadStaticImageFromFile(filePath, tImg, errMsg)) {
        qDebug() << errMsg;
    }
    QPixmap pixmap;
    pixmap = QPixmap::fromImage(tImg);

    return pixmap;
#else
    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    return pixmap;
#endif
}

//QString sizeToHuman(const qlonglong bytes)
//{
//    qlonglong sb = 1024;
//    if (bytes < sb) {
//        return QString::number(bytes) + " B";
//    } else if (bytes < sb * sb) {
//        QString vs = QString::number((double)bytes / sb, 'f', 1);
//        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
//            return QString::number((int)vs.toDouble()) + " KB";
//        } else {
//            return vs + " KB";
//        }
//    } else if (bytes < sb * sb * sb) {
//        QString vs = QString::number((double)bytes / sb / sb, 'f', 1);
//        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
//            return QString::number((int)vs.toDouble()) + " MB";
//        } else {
//            return vs + " MB";
//        }
//    } else {
//        return QString::number(bytes);
//    }
//}

QString timeToString(const QDateTime &time, bool normalFormat)
{
    if (normalFormat)
        return time.toString(DATETIME_FORMAT_NORMAL);
    else
        return time.toString(DATETIME_FORMAT_EXIF);
}

int stringWidth(const QFont &f, const QString &str)
{
    QFontMetrics fm(f);
    return fm.boundingRect(str).width();
}

int stringHeight(const QFont &f, const QString &str)
{
    QFontMetrics fm(f);
    return fm.boundingRect(str).height();
}

QDateTime stringToDateTime(const QString &time)
{
    QDateTime dt = QDateTime::fromString(time, DATETIME_FORMAT_EXIF);
    if (! dt.isValid()) {
        dt = QDateTime::fromString(time, DATETIME_FORMAT_NORMAL);
    }
    return dt;
}

void showInFileManager(const QString &path)
{
    if (path.isEmpty() || !QFile::exists(path)) {
        return;
    }

#if 1
    QUrl url = QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath());
#else
    QUrl url = QUrl::fromLocalFile(path);
#endif
    Dtk::Widget::DDesktopServices::showFileItem(url);
//    QUrl url = QUrl::fromLocalFile(QFileInfo(path).dir().absolutePath());
//    QUrlQuery query;
//    query.addQueryItem("selectUrl", QUrl::fromLocalFile(path).toString());
//    url.setQuery(query);
//    qDebug() << "showInFileManager:" << url.toString();
//    // Try dde-file-manager
//    QProcess *fp = new QProcess();
//    QObject::connect(fp, SIGNAL(finished(int)), fp, SLOT(deleteLater()));
//    fp->start("dde-file-manager", QStringList(url.toString()));
//    fp->waitForStarted(3000);
//    if (fp->error() == QProcess::FailedToStart) {
//        // Start dde-file-manager failed, try nautilus
//        QDBusInterface iface("org.freedesktop.FileManager1",
//                             "/org/freedesktop/FileManager1",
//                             "org.freedesktop.FileManager1",
//                             QDBusConnection::sessionBus());
//        if (iface.isValid()) {
//            // Convert filepath to URI first.
//            const QStringList uris = { QUrl::fromLocalFile(path).toString() };
//            qDebug() << "freedesktop.FileManager";
//            // StartupId is empty here.
//            QDBusPendingCall call = iface.asyncCall("ShowItems", uris, "");
//            Q_UNUSED(call);
//        }
//        // Try to launch other file manager if nautilus is invalid
//        else {
//            qDebug() << "desktopService::openUrl";
//            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).dir().absolutePath()));
//        }
//        fp->deleteLater();
//    }
}

/**
   @brief 拷贝图片信息到剪贴板，若 `sourceImage` 不为空，则在 wayland 下设置缩略图数据
 */
void copyImageToClipboard(const QStringList &paths, const QImage &sourceImage)
{
    if (paths.isEmpty()) {
        return;
    }

    //  Get clipboard
    QClipboard *cb = qApp->clipboard();

    // Ownership of the new data is transferred to the clipboard.
    QMimeData *newMimeData = new QMimeData();

    // Copy old mimedata
//    const QMimeData* oldMimeData = cb->mimeData();
//    for ( const QString &f : oldMimeData->formats())
//        newMimeData->setData(f, oldMimeData->data(f));

    // Copy file (gnome)
    QByteArray gnomeFormat = QByteArray("copy\n");
    QString text;
    QList<QUrl> dataUrls;
    for (QString path : paths) {
        if (!path.isEmpty())
            text += path + '\n';
        dataUrls << QUrl::fromLocalFile(path);
        gnomeFormat.append(QUrl::fromLocalFile(path).toEncoded()).append("\n");
    }

    newMimeData->setText(text.endsWith('\n') ? text.left(text.length() - 1) : text);
    newMimeData->setUrls(dataUrls);
    gnomeFormat.remove(gnomeFormat.length() - 1, 1);
    newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

    // Warning: 会导致 1070 wayland 下拷贝无图片显示，此修改和 bug 191601 有关，仅在定制镜像保留。
    Q_UNUSED(sourceImage);
#if 0
    // Copy Image Data
    if (!sourceImage.isNull() && checkWayland()) {
        newMimeData->setImageData(sourceImage.scaled(200, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
#endif

    // Set the mimedata
    cb->setMimeData(newMimeData, QClipboard::Clipboard);
}

QString getFileContent(const QString &file)
{
    QFile f(file);
    QString fileContent = "";
    if (f.open(QFile::ReadOnly)) {
        fileContent = QLatin1String(f.readAll());
        f.close();
    }
    return fileContent;
}

//bool writeTextFile(QString filePath, QString content)
//{
//    QFile file(filePath);
//    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        QTextStream in(&file);
//        in << content << endl;
//        file.close();
//        return true;
//    }

//    return false;
//}

QString getNotExistsTrashFileName(const QString &fileName)
{
    QByteArray name = fileName.toUtf8();

    int index = name.lastIndexOf('/');

    if (index >= 0)
        name = name.mid(index + 1);

    index = name.lastIndexOf('.');
    QByteArray suffix;

    if (index >= 0)
        suffix = name.mid(index);

    if (suffix.size() > 200)
        suffix = suffix.left(200);

    name.chop(suffix.size());
    name = name.left(200 - suffix.size());

    QString trashpath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/Trash";

    while (true) {
        QFileInfo info(trashpath + name + suffix);
        // QFile::exists ==> If the file is a symlink that points to a non-existing file, false is returned.
        if (!info.isSymLink() && !info.exists()) {
            break;
        }

        name = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();
    }

    return QString::fromUtf8(name + suffix);
}

bool trashFile(const QString &file)
{
#ifdef QT_GUI_LIB
    QString trashPath;
    QString trashInfoPath;
    QString trashFilesPath;

    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    // There maby others location for trash like $HOME/.trash or
    // $XDG_DATA_HOME/Trash, but our stupid FileManager coder said we should
    // assume that the trash lcation is $HOME/.local/share/Trash,so...
    trashPath = home + "/.local/share/Trash";
    trashInfoPath = trashPath + "/info";
    trashFilesPath = trashPath + "/files";
    if (! QDir(trashFilesPath).exists()) {
        QDir().mkpath(trashFilesPath);
    }
    if (! QDir(trashInfoPath).exists()) {
        QDir().mkpath(trashInfoPath);
    }

    QFileInfo originalInfo(file);
    if (! originalInfo.exists()) {
        qWarning() << "File doesn't exists, can't move to trash";
        return false;
    }
    // Info for restore
    QString infoStr;
    infoStr += "[Trash Info]\nPath=";
    infoStr += QString(originalInfo.absoluteFilePath().toUtf8().toPercentEncoding("/"));
    infoStr += "\nDeletionDate=";
    infoStr += QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzzZ");
    infoStr += "\n";

    QString trashname = getNotExistsTrashFileName(originalInfo.fileName());
    QString infopath = trashInfoPath + "/" + trashname + ".trashinfo";
    QString filepath = trashFilesPath + "/" + trashname;
    int nr = 1;
    while (QFileInfo(infopath).exists() || QFileInfo(filepath).exists()) {
        nr++;
        trashname = originalInfo.baseName() + "." + QString::number(nr);
        if (!originalInfo.completeSuffix().isEmpty()) {
            trashname += QString(".") + originalInfo.completeSuffix();
        }
        infopath = trashInfoPath + "/" + trashname + ".trashinfo";
        filepath = trashFilesPath + "/" + trashname;
    }
    QFile infoFile(infopath);
    if (infoFile.open(QIODevice::WriteOnly)) {
        infoFile.write(infoStr.toUtf8());
        infoFile.close();

        if (!QDir().rename(originalInfo.absoluteFilePath(), filepath)) {
            qWarning() << "move to trash failed!";
            return false;
        }
    } else {
        qDebug() << "Move to trash failed! Could not write *.trashinfo!";
        return false;
    }
    // Remove thumbnail
    Libutils::image::removeThumbnail(file);
    return true;
#else
    Q_UNUSED(file);
    qWarning() << "Trash in server-mode not supported";
    return false;
#endif
}

//bool trashFiles(const QStringList &files)
//{
//    bool v = true;
//    for (QString file : files) {
//        if (! trashFile(file))
//            v = false;
//    }

//    return v;
//}

///*!
// * \brief wrapStr
// * Split info string by Space
// * \param str
// * \param font
// * \param maxWidth
// * \return
// */
//QString wrapStr(const QString &str, const QFont &font, int maxWidth)
//{
//    QFontMetrics fm(font);
//    QString ns;
//    QString ss;
//    for (int i = 0; i < str.length(); i ++) {
//        if (/*str.at(i).isSpace()||*/ fm.boundingRect(ss).width() > maxWidth) {
//            ss = QString();
//            ns += "\n";
//        }
//        ns += str.at(i);
//        ss += str.at(i);
//    }
//    return ns;
////    return str;
//}


QString SpliteText(const QString &text, const QFont &font, int nLabelSize, bool bReturn)
{
    QFontMetrics fm(font);
    int nTextSize = fm.width(text);
    if (nTextSize > nLabelSize) {
        int nPos = 0;
        long nOffset = 0;
        for (int i = 0; i < text.size(); i++) {
            nOffset += fm.width(text.at(i));
            if (nOffset >= nLabelSize) {
                nPos = i;
                break;
            }
        }

        nPos = (nPos - 1 < 0) ? 0 : nPos - 1;

        QString qstrLeftData = text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        if (bReturn) {
            qstrLeftData.replace(" ", "\n");
            qstrMidData.replace(" ", "\n");
            if (qstrLeftData != "")
                return qstrLeftData + SpliteText(qstrMidData, font, nLabelSize);
        } else {
            if (qstrLeftData != "")
                return qstrLeftData + "\n" + SpliteText(qstrMidData, font, nLabelSize);
        }
    }
    return text;
}


//QString symFilePath(const QString &path)
//{
//    QFileInfo fileInfo(path);
//    if (fileInfo.isSymLink()) {
//        return fileInfo.symLinkTarget();
//    } else {
//        return path;
//    }
//}

QString hash(const QString &str)
{
    return QString(QCryptographicHash::hash(str.toUtf8(),
                                            QCryptographicHash::Md5).toHex());
}

bool onMountDevice(const QString &path)
{
    return (path.startsWith("/media/") || path.startsWith("/run/media/"));
}

bool mountDeviceExist(const QString &path)
{
    QString mountPoint;
    if (path.startsWith("/media/")) {
        const int sp = path.indexOf("/", 7) + 1;
        const int ep = path.indexOf("/", sp) + 1;
        mountPoint = path.mid(0, ep);

    } else if (path.startsWith("/run/media/")) {
        const int sp = path.indexOf("/", 11) + 1;
        const int ep = path.indexOf("/", sp) + 1;
        mountPoint = path.mid(0, ep);
    }

    return QFileInfo(mountPoint).exists();
}

bool checkCommandExist(const QString &command)
{
    try {
        QString path = QStandardPaths::findExecutable(command);
        return !path.isEmpty();
    } catch (std::logic_error &e) {
        qWarning() << e.what();
        return false;
    }
}

static bool g_IsWaylandEnv = false;
/**
   @brief 初始化判断当前是否为 wayland 环境，并缓存状态
 */
bool initCheckWaylandEnv()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        g_IsWaylandEnv = true;
        return true;
    } else {
        return false;
    }
}

/**
   @return 返回当前是否为 wayland 环境
 */
bool checkWayland()
{
    return g_IsWaylandEnv;
}

// 是否已初始化翻译文件
static bool g_LoadTranslator = false;
/**
   @brief 加载动态库指向的翻译文件
   @note 非线程安全，仅在GUI线程调用
 */
bool loadLibTransaltor()
{
    if (g_LoadTranslator) {
        return true;
    }

    QDir dir(PLUGIN_QM_TRANS_PATH);
    if (dir.exists()) {
        QDirIterator qmIt(PLUGIN_QM_TRANS_PATH, QStringList() << QString("*%1.qm").arg(QLocale::system().name()), QDir::Files);

        while (qmIt.hasNext()) {
            qmIt.next();
            QFileInfo finfo = qmIt.fileInfo();
            QTranslator *translator = new QTranslator(qApp);
            if (translator->load(finfo.baseName(), finfo.absolutePath())) {
                qApp->installTranslator(translator);
            }
        }

        QStringList parseLocalNameList = QLocale::system().name().split("_", QString::SkipEmptyParts);
        if (parseLocalNameList.length() > 0) {
            QString translateFilename = QString("/libimageviewer_%2.qm").arg(parseLocalNameList.at(0));

            QString translatePath = PLUGIN_QM_TRANS_PATH + translateFilename;
            if (QFile::exists(translatePath)) {
                qDebug() << "translatePath after feedback:" << translatePath;
                auto translator = new QTranslator(qApp);
                translator->load(translatePath);
                qApp->installTranslator(translator);
            }
        }

        g_LoadTranslator = true;
        return true;
    }

    return false;
}

}  // namespace base

}  // namespace utils
