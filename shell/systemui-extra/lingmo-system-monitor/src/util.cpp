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

#include "util.h"

#include <QApplication>
#include <QIcon>
#include <QDirIterator>
#include <QDebug>

#include <glibtop/procstate.h>
#include <fstream>
#include <sstream>
#include <QSvgRenderer>
#include <QProcess>
#include <lingmosdk/lingmosdk-base/kyutils.h>

bool loadSvg(const QString &fileName, const int size, QPixmap& pixmap)
{
    pixmap = pixmap.scaled(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    return true;
}

QPixmap drawSymbolicColoredPixmap(const QPixmap &source)
{
    QColor gray(128,128,128);
    QColor standard (31,32,34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else if(qAbs(color.red()-standard.red())<20 && qAbs(color.green()-standard.green())<20 && qAbs(color.blue()-standard.blue())<20)
                {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else
                {
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap drawSymbolicBlackColoredPixmap(const QPixmap &source)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red())>=200 && qAbs(color.green())>=200 && qAbs(color.blue())>=200) {
                    color.setRed(56);
                    color.setGreen(56);
                    color.setBlue(56);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

std::string make_string(char *c_str)
{
    if (!c_str) {
        return string();
    }
    string s(c_str);
    g_free(c_str);
    return s;
}

static inline unsigned divide(unsigned *q, unsigned *r, unsigned d)
{
    *q = *r / d;
    *r = *r % d;
    return *q != 0;
}

QString getElidedText(QFont font, QString str, int MaxWidth)
{
    if (str.isEmpty())
    {
        return "";
    }

    QFontMetrics fontWidth(font);

    //计算字符串宽度
    //calculat the width of the string
    int width = fontWidth.width(str);

    //当字符串宽度大于最大宽度时进行转换
    //Convert when string width is greater than maximum width
    if (width >= MaxWidth)
    {
        //右部显示省略号
        //show by ellipsis in right
        str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
    }
    //返回处理后的字符串
    //return the string that is been handled
    return str;
}

QString getMiddleElidedText(QFont font, QString str, int MaxWidth)
{
    if (str.isEmpty())
    {
        return "";
    }

    QFontMetrics fontWidth(font);

    //计算字符串宽度
    int width = fontWidth.width(str);

    //当字符串宽度大于最大宽度时进行转换
    if (width >= MaxWidth)
    {
        //右部显示省略号
        str = fontWidth.elidedText(str, Qt::ElideMiddle, MaxWidth);
    }
    //返回处理后的字符串
    return str;
}

QString formatDurationForDisplay(unsigned centiseconds)
{
    unsigned weeks = 0, days = 0, hours = 0, minutes = 0, seconds = 0;

    (void)(divide(&seconds, &centiseconds, 100)
           && divide(&minutes, &seconds, 60)
           && divide(&hours, &minutes, 60)
           && divide(&days, &hours, 24)
           && divide(&weeks, &days, 7));

    QString formatTime;
    gchar *duration = NULL;

    if (weeks) {
        duration = g_strdup_printf("%uw%ud", weeks, days);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    if (days) {
        duration = g_strdup_printf("%ud%02uh", days, hours);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    if (hours) {
        duration = g_strdup_printf("%u:%02u:%02u", hours, minutes, seconds);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    duration = g_strdup_printf("%u:%02u.%02u", minutes, seconds, centiseconds);
    formatTime = QString(QLatin1String(duration));
    if (duration) {
        g_free(duration);
        duration = NULL;
    }
    return formatTime;
}

std::string getDesktopFileAccordProcName(QString procName, QString cmdline)
{
    Q_UNUSED(cmdline);
    QDirIterator dir("/etc/xdg/autostart", QDirIterator::Subdirectories);
    std::string desktopFile;
    QString procname = procName.toLower();
    procname.replace("_", "-");
    QString processFilename = procname + ".desktop";

    while(dir.hasNext()) {
        if (dir.fileInfo().suffix() == "desktop") {
            if (dir.fileName().toLower().contains(processFilename)) {
                desktopFile = dir.filePath().toStdString();
//                qDebug()<<"---desktopFile---"<<desktopFile<<std::endl;
                break;
            }
        }
        dir.next();
    }
    return desktopFile;
}

std::string getDesktopFileAccordProcNameApp(QString procName, QString cmdline)
{
    Q_UNUSED(cmdline);
    QDirIterator dir("/usr/share/applications", QDirIterator::Subdirectories);
    std::string desktopFile;
    // 去掉程序名称后缀
    int nIndex = procName.lastIndexOf(".");
    if (nIndex != -1) {
        procName.truncate(nIndex);
    }
    QString procname = procName.toLower();
    procname.replace("_", "-");
    QString processFilename = procname + ".desktop";

    while(dir.hasNext()) {
        if (dir.fileInfo().suffix() == "desktop") {
            if (dir.fileName().toLower() == processFilename
                || dir.fileName().toLower().contains("-"+processFilename)) {
                desktopFile = dir.filePath().toStdString();
                break;
            }
        }
        dir.next();
    }
    return desktopFile;
}

QPixmap getAppIconFromDesktopFile(std::string desktopFile, int iconSize)
{
    QIcon defaultExecutableIcon = QIcon::fromTheme("application-x-executable");//gnome-mine-application-x-executable
    if (defaultExecutableIcon.isNull()) {
        defaultExecutableIcon = QIcon("/usr/share/icons/lingmo-icon-theme-default/48x48/mimetypes/application-x-executable.png");
        if (defaultExecutableIcon.isNull())
            defaultExecutableIcon = QIcon(":/res/autostart-default.png");
    }

    QIcon icon;
    QString iconName;
    QString strDesktopFile = QString::fromStdString(desktopFile);
    if (!strDesktopFile.isEmpty()) {
        QFile file(strDesktopFile);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            while(!stream.atEnd())
            {
                iconName = stream.readLine();

                if (iconName.startsWith("Icon=")) {
                    iconName.remove(0,5);
                }
                else {
                    continue;
                }

                if (iconName.contains("/")) {
                    QFileInfo fileInfo(iconName);
                    if (fileInfo.exists()) {
                        icon = QIcon(iconName);
                        break;
                    }
                }
                else {
                    icon = QIcon::fromTheme(iconName, defaultExecutableIcon);
                    break;
                }
            }
            file.close();
        }
    }

    qreal devicePixelRatio = 1;//qApp->devicePixelRatio();
    QPixmap pixmap = icon.pixmap(iconSize * devicePixelRatio, iconSize * devicePixelRatio);
    // pixmap.setDevicePixelRatio(devicePixelRatio);

    return pixmap;
}

QString getAppIconPathFromDesktopFile(std::string desktopFile)
{
    QString strIconPath = "";
    QString iconName;
    QString strDesktopFile = QString::fromStdString(desktopFile);
    if (!strDesktopFile.isEmpty()) {
        QFile file(strDesktopFile);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            while(!stream.atEnd())
            {
                iconName = stream.readLine();

                if (iconName.startsWith("Icon=")) {
                    iconName.remove(0,5);
                    strIconPath = iconName;
                    break;
                }
                else {
                    continue;
                }
            }
            file.close();
        }
    }
    return strIconPath;
}

QString getDisplayNameAccordProcName(QString procName, std::string desktopFile)
{
    if (desktopFile.size() == 0) {
        return procName;
    }

    QString iconName;
    QFile file(QString::fromStdString(desktopFile));
    QString displayName = procName;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            QString lineContent = stream.readLine();
            QString localNameFlag = QString("Name[%1]=").arg(QLocale::system().name());
            QString nameFlag = "Name=";
            QString genericNameFlag = QString("GenericName[%1]=").arg(QLocale::system().name());

            if (lineContent.startsWith(localNameFlag)) {
                displayName = lineContent.remove(0, localNameFlag.size());
                break;
            }
            else if (lineContent.startsWith(genericNameFlag)) {
                displayName = lineContent.remove(0, genericNameFlag.size());
                break;
            }
            else if (lineContent.startsWith(nameFlag)) {
                displayName = lineContent.remove(0, nameFlag.size());
                continue;
            }
            else {
                continue;
            }
        }
        file.close();
    }

    return displayName;
}

QString formatProcessState(guint state)
{
    QString status;
    switch (state)
    {
        case GLIBTOP_PROCESS_RUNNING:
            status = QString(QObject::tr("Running"));//运行中
            break;

        case GLIBTOP_PROCESS_STOPPED:
            status = QString(QObject::tr("Stopped"));//已停止
            break;

        case GLIBTOP_PROCESS_ZOMBIE:
            status = QString(QObject::tr("Zombie"));//僵死
            break;

        case GLIBTOP_PROCESS_UNINTERRUPTIBLE:
            status = QString(QObject::tr("Uninterruptible"));//不可中断
            break;

        default:
            status = QString(QObject::tr("Sleeping"));//睡眠中
            break;
    }

    return status;
}

QString getNiceLevel(int nice)
{
    if (nice < -7)
        return QObject::tr("Very High");
    else if (nice < -2)
        return QObject::tr("High");
    else if (nice < 3)
        return QObject::tr("Normal");
    else if (nice < 7)
        return QObject::tr("Low");
    else
        return QObject::tr("Very Low");
}

QString getNiceLevelWithPriority(int nice)
{
    if (nice < -7)
        return QObject::tr("Very High Priority");
    else if (nice < -2)
        return QObject::tr("High Priority");
    else if (nice < 3)
        return QObject::tr("Normal Priority");
    else if (nice < 7)
        return QObject::tr("Low Priority");
    else
        return QObject::tr("Very Low Priority");
}

void setFontSize(QPainter &painter, int textSize)
{
    QFont font = painter.font() ;
    font.setPixelSize(textSize);
//    font.setPointSize(textSize);
    painter.setFont(font);
}

QString formatUnitSize(double v, const char** orders, int nb_orders)
{
    int order = 0;
    guint64 m_size = guint64(v);
    char data[128] = {0};
    sprintf(data,"%lu",m_size);
    char result_data[128] = {0};
    double m_speed = 0;
    if (kdkVolumeBaseCharacterConvert(data, KDK_EXABYTE, result_data) == KDK_NOERR) {
        QString result_data_str = QString(result_data);
        result_data_str.replace("KB", "K");
        result_data_str.replace("MB", "M");
        result_data_str.replace("GB", "G");
        result_data_str.replace("TB", "T");
        result_data_str.replace("PB", "P");
        result_data_str.replace("EB", "E");

        result_data_str.replace("B", " B");
        result_data_str.replace("K", " KB");
        result_data_str.replace("M", " MB");
        result_data_str.replace("G", " GB");
        result_data_str.replace("T", " TB");
        result_data_str.replace("P", " PB");
        result_data_str.replace("E", " EB");
        sscanf(result_data, "%lf", &m_speed);
        return result_data_str;
    } else {
        while (v >= 1024 && order + 1 < nb_orders) {
            order++;
            v  = v/1024;
        }
        char buffer1[30] = {0};
        snprintf(buffer1, sizeof(buffer1), "%.1lf %s", v, orders[order]);
        return QString(buffer1);
    }
}

QString formatByteCount(double v)
{
    static const char* orders[] = { "B", "KB", "MB", "GB", "TB" };
    return formatUnitSize(v, orders, sizeof(orders)/sizeof(orders[0]));
}

QString getDeviceMountedPointPath(const QString &line)
{
    const QStringList items = line.split(" ");
    if (items.length() > 4) {
        return items.at(1);
    }
    else {
        return "";
    }
}

QString getFileContent(const QString &filePath)
{
    QFile fd(filePath);
    QString fileContent = "";
    if (fd.open(QFile::ReadOnly)) {
        fileContent = QLatin1String(fd.readAll());
        fd.close();
    }
    return fileContent;
}

QSet<QString> getFileContentsLineByLine(const QString &filePath)
{
    QString fileContent = getFileContent(filePath);
    return QSet<QString>::fromList(fileContent.split("\n"));
}

// 获取应用程序版本
QString getUsmVersion()
{
    QString versionText;
    QProcess proc;
    QStringList options;
    options << "-l" << "|" << "grep" << "lingmo-system-monitor";
    proc.start("dpkg", options);
    proc.waitForFinished();
    QString dpkgInfo = proc.readAll();
    QStringList infoList = dpkgInfo.split("\n");
    for (int n = 0; n < infoList.size(); n++) {
        QString strInfoLine = infoList[n];
        if (strInfoLine.contains("lingmo-system-monitor")) {
            QStringList lineInfoList = strInfoLine.split(QRegExp("[\\s]+"));
            if (lineInfoList.size() >= 3) {
                versionText = lineInfoList[2];
            }
            break;
        }
    }
    return versionText;
}
