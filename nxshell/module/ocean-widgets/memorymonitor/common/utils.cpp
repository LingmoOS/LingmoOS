// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <QApplication>
#include <QDir>
#include <QFontMetrics>
#include <QIcon>
#include <QLayout>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QWidget>
#include <QtDBus>
#include <QtMath>
#include <QtX11Extras/QX11Info>
#include <QPainterPath>

#include <fstream>
#include <string>

//#include <X11/extensions/shape.h>

//#include <proc/readproc.h>


//DCORE_USE_NAMESPACE

namespace Utils {
static QMap<QString, QString> desktopfileMaps = getDesktopfileMap();

QMap<QString, QString> getDesktopfileMap()
{
    QMap<QString, QString> map;

    map["/opt/kingsoft/wps-office/office6/wps"] = "/usr/share/applications/wps-office-wps.desktop";
    map["/opt/kingsoft/wps-office/office6/wpp"] = "/usr/share/applications/wps-office-wpp.desktop";
    map["/opt/kingsoft/wps-office/office6/et"] = "/usr/share/applications/wps-office-et.desktop";

    return map;
}


int getStatusBarMaxWidth()
{
    // TODO: use more elegent way to calc bar width
    return 300;
}

bool startWithHanzi(const QString &text)
{
    if (text.isEmpty())
        return false;

    return text.at(0).script() == QChar::Script_Han;
}

QPixmap getDesktopFileIcon(const std::string &desktopFile, int iconSize)
{
    std::ifstream in;
    in.open(desktopFile);
    QIcon defaultExecutableIcon = QIcon::fromTheme("application-x-executable");
    QIcon icon;
    QString iconName;
    bool foundIconLine = false;

    while (!in.eof()) {
        std::string line;
        std::getline(in, line);
        iconName = QString::fromStdString(line);

        if (iconName.startsWith("Icon")) {
            iconName = iconName.split("=").last().simplified();
            foundIconLine = true;
        } else {
            continue;
        }

        if (iconName.contains("/")) {
            // this is probably a path to the file, use that instead of the theme icon name
            icon = QIcon(iconName);
        } else {
            icon = QIcon::fromTheme(iconName, defaultExecutableIcon);
            break;
        }
    }
    in.close();

    // Use default icon instead, if not found "Icon=" line in desktop file.
    if (!foundIconLine) {
        icon = defaultExecutableIcon;
    }

    return icon.pixmap(iconSize, iconSize);
}

QSize getRenderSize(int fontSize, const QString &string)
{
    QFont font;
    font.setPointSizeF(fontSize);
    QFontMetrics fm(font);

    int width = 0;
    int height = 0;
    foreach (auto line, string.split("\n")) {
        int lineWidth = fm.width(line);
        int lineHeight = fm.height();

        if (lineWidth > width) {
            width = lineWidth;
        }
        height += lineHeight;
    }

    return QSize(width, height);
}

QString getProcessEnvironmentVariable(pid_t pid, const QString &environmentName)
{
    std::string temp;
    try {
        std::fstream fs;
        fs.open("/proc/" + std::to_string(long(pid)) + "/environ", std::fstream::in);
        std::getline(fs, temp);
        fs.close();
    } catch (std::ifstream::failure &e) {
        Q_UNUSED(e)
        return "FAILED TO READ PROC";
    }

    // change \0 to ' '
    std::replace(temp.begin(), temp.end(), '\0', '\n');

    if (temp.size() < 1) {
        return "";
    }

    foreach (auto environmentVariable, QString::fromStdString(temp).trimmed().split("\n")) {
        if (environmentVariable.startsWith(QString("%1=").arg(environmentName))) {
            return environmentVariable.remove(0, QString("%1=").arg(environmentName).length());
        }
    }

    return "";
}

QString getProcessCmdline(pid_t pid)
{
    std::string temp;
    try {
        std::fstream fs;
        fs.open("/proc/" + std::to_string(long(pid)) + "/cmdline", std::fstream::in);
        std::getline(fs, temp);
        fs.close();
    } catch (std::ifstream::failure &e) {
        Q_UNUSED(e)
        return "FAILED TO READ PROC";
    }

    // change \0 to ' '
    std::replace(temp.begin(), temp.end(), '\0', ' ');

    if (temp.size() < 1) {
        return "";
    }

    return QString::fromStdString(temp).trimmed();
}

QString getQrcPath(QString imageName)
{
    return QString(":/image/%1").arg(imageName);
}

QDir getFlatpakAppPath(const QString &flatpakAppid)
{
    QProcess whichProcess;
    QString exec = "flatpak";
    QStringList params;
    params << "info";
    params << flatpakAppid;
    whichProcess.start(exec, params);
    whichProcess.waitForFinished();
    QString output(whichProcess.readAllStandardOutput());

    return QDir(output.split("Location:")[1].split("\n")[0].simplified());
}

QString getFlatpakAppIcon(const QString &flatpakAppid)
{
    QString exec = "flatpak";

    QStringList params;
    params << "info";
    params << flatpakAppid;

    // set the LANGUAGE env so that the output is fixed to be English,
    // or below code may break in other locales like pt_BR.
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANGUAGE", "en_US");

    QProcess whichProcess;
    whichProcess.setProcessEnvironment(env);
    whichProcess.start(exec, params);
    whichProcess.waitForFinished();
    QString output(whichProcess.readAllStandardOutput());

    const QString dirPath = output.split("Location:")[1].split("\n")[0].simplified();

    QDir flatpakDir = QDir(dirPath);
    flatpakDir.cd("export");
    flatpakDir.cd("share");
    flatpakDir.cd("icons");
    flatpakDir.cd("hicolor");
    flatpakDir.cd("scalable");
    flatpakDir.cd("apps");

    const QString appID = flatpakAppid.split("app/")[1].split("/")[0];
    return flatpakDir.filePath(QString("%1.svg").arg(appID));
}

bool fileExists(const QString &path)
{
    QFileInfo check_file(path);

    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}


void drawLoadingRing(QPainter &painter, int centerX, int centerY, int radius, int penWidth,
                     int loadingAngle, int rotationAngle, QColor foregroundColor,
                     double foregroundOpacity, QColor backgroundColor, double backgroundOpacity,
                     double percent)
{
    drawRing(painter, centerX, centerY, radius, penWidth, loadingAngle, rotationAngle,
             backgroundColor, backgroundOpacity);
    drawRing(painter, centerX, centerY, radius, penWidth, int(loadingAngle * percent), rotationAngle,
             foregroundColor, foregroundOpacity);
}

void drawRing(QPainter &painter, int centerX, int centerY, int radius, int penWidth,
              int loadingAngle, int rotationAngle, const QColor &color, double opacity)
{
    QRect drawingRect;

    drawingRect.setX(centerX - radius + penWidth);
    drawingRect.setY(centerY - radius + penWidth);
    drawingRect.setWidth(radius * 2 - penWidth * 2);
    drawingRect.setHeight(radius * 2 - penWidth * 2);

    painter.setOpacity(opacity);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(QBrush(QColor(color)), penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    int arcLengthApproximation = penWidth + penWidth / 3;
    painter.drawArc(drawingRect, 90 * 16 - arcLengthApproximation + rotationAngle * 16,
                    -loadingAngle * 16);
}

void drawTooltipBackground(QPainter &painter, QRect rect, qreal opacity)
{
    painter.setOpacity(opacity);
    QPainterPath path;
    path.addRoundedRect(QRectF(rect), RECTANGLE_RADIUS, RECTANGLE_RADIUS);
    painter.fillPath(path, QColor("#F5F5F5"));

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}

void drawTooltipText(QPainter &painter, const QString &text, const QString &textColor, int textSize, const QRectF &rect)
{
    setFontSize(painter, textSize);
    painter.setOpacity(1);
    painter.setPen(QPen(QColor(textColor)));
    painter.drawText(rect, Qt::AlignCenter, text);
}

void removeChildren(QWidget *widget)
{
    qDeleteAll(widget->children());
}

void removeLayoutChild(QLayout *layout, int index)
{
    QLayoutItem *item = layout->itemAt(index);
    if (item != nullptr) {
        QWidget *widget = item->widget();
        if (widget != nullptr) {
            widget->hide();
            widget->setParent(nullptr);
            layout->removeWidget(widget);
        }
    }
}

void setFontSize(QPainter &painter, int textSize)
{
    QFont font = painter.font();
    font.setPointSizeF(textSize);
    painter.setFont(font);
}
}  // namespace Utils
