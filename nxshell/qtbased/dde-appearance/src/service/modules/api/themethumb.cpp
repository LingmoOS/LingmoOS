// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QImageReader>

#include "themethumb.h"
#include "../api/dfile.h"
#include "../api/utils.h"
#include "compatibleengine.h"
#include "keyfile.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

#include <DGuiApplicationHelper>

#include <X11/Xcursor/Xcursor.h>

#undef Bool
#undef Status
#undef Unsorted
#undef True
#undef False

using Dtk::Gui::DGuiApplicationHelper;

const int width = 320;
const int height = 70;
const int baseCursorSize = 24;
const int baseCursorPadding = 12;
const int baseIconPadding  = 4;
const int baseIconSize = 48;

static QVector<QStringList> presentCursors {
   {"left_ptr"},
   {"left_ptr_watch"},
   {"x-cursor", "X_cursor"},
   {"hand2", "hand1"},
   {"grab", "grabbing", "closedhand"},
   {"fleur", "move"},
   {"sb_v_double_arrow"},
   {"sb_h_double_arrow"},
   {"watch", "wait"}
};

static QVector<QStringList> presentIcons {
    // file manager:
    {"dde-file-manager", "system-file-manager"},
    // music player:
    {"deepin-music", "banshee", "amarok", "deadbeef", "clementine", "rhythmbox"},
    // image viewer:
    {"deepin-image-viewer", "eog", "gthumb", "gwenview", "gpicview", "showfoto", "phototonic"},
    // media/video player:
    {"deepin-movie", "media-player", "totem", "smplayer", "vlc", "dragonplayer", "kmplayer"},
    // web browser:
    {"google-chrome", "firefox", "chromium", "opera", "internet-web-browser", "browser"},
    // system settings:
    {"preferences-system"},
    // text editor:
//    {"accessories-text-editor", "text-editor", "gedit", "kedit", "xfce-edit"},
    // terminal:
//    {"deepin-terminal", "utilities-terminal", "terminal", "gnome-terminal", "xfce-terminal", "terminator", "openterm"},
};
// ScaleFactor 多次用到，且修改有信号，改为静态变量做缓存
static double g_ScaleFactor = 0.0;

QString getScaleDir()
{
    double scaleFactor = getScaleFactor();

    return QString("X%1").arg(scaleFactor);
}

QString getTypeDir(QString type0, int version)
{
    return QString("%1-v%2").arg(type0).arg(version);
}

void init()
{
    removeUnusedScaleDirs();
    removeAllTypesOldVersionDirs();
}

void removeUnusedScaleDirs()
{
    QString cacheDir = utils::GetUserCacheDir();
    cacheDir+="/deepin/dde-api/theme_thumb";
    removeUnusedDirs(cacheDir+"/X*", getScaleDir());
}

void removeAllTypesOldVersionDirs()
{
    QString scaleDir = getScaleDir();
    removeOldVersionDirs(scaleDir, "gtk", gtkVersion);
    removeOldVersionDirs(scaleDir, "cursor", cursorVersion);
    removeOldVersionDirs(scaleDir, "icon", iconVersion);
}

void removeOldVersionDirs(QString scaleDir, QString type0, int version)
{
    QString cacheDir = utils::GetUserCacheDir();
    cacheDir+="/deepin/dde-api/theme_thumb";
    QString pattern = cacheDir + "/" + scaleDir + "/" + type0 + "/-v*";
    QString usedDir = getTypeDir(type0, version);
    removeUnusedDirs(pattern, usedDir);
}

void removeUnusedDirs(QString pattern, QString userDir)
{
    std::vector<QString> dirs = DFile::glob(pattern);
    if(dirs.empty()){
        return;
    }

    for(auto dir:dirs){
        if(DFile::base(dir)!=userDir){
            remove(dir.toStdString().c_str());
        }
    }
}

bool checkScaleFactor()
{
    double scaleFactor = getScaleFactor();
    if (scaleFactor <= 0)
        return false;
    return true;
}

double getScaleFactor()
{
    return g_ScaleFactor;
}

QString getCursor(QString id, QString descFile)
{
    if (!checkScaleFactor()) {
        qInfo() << "scaleFactor <= 0";
        return "";
    }

    QString out = prepareOutputPath("cursor", id, cursorVersion);
    if (!shouldGenerateNewCursor(descFile, out)) {
        return out;
    }

    double scaleFactor = getScaleFactor();

    if(!genCursor(descFile,width,height,scaleFactor, out))
    {
        return "";
    }

    return out;
}

bool genCursor(QString descFile,int width,int height,double scaleFactor,QString out)
{
    QString dirPath = descFile + "/cursors";

    int iconSize = static_cast<int>(baseCursorSize * scaleFactor);
    int padding  = static_cast<int>(baseCursorPadding * scaleFactor);
    width        = static_cast<int>(width * scaleFactor);
    height       = static_cast<int>(height * scaleFactor);

    qDebug()<<"dirPath : "<<dirPath;
    QVector<QImage*> images = getCursors(dirPath,iconSize);
    QImage image = CompositeImages(images,width,height,iconSize,padding);

    if(image.save(out))
    {

    }

    return true;
}

bool genIcon(QString theme,int width,int height,double scaleFactor,QString out)
{
    int iconSize = static_cast<int>(baseIconSize * scaleFactor);
    int padding  = static_cast<int>(baseIconPadding * scaleFactor);
    width        = static_cast<int>(width * scaleFactor);
    height       = static_cast<int>(height * scaleFactor);

    QList<QIcon> images = getIcons(theme, iconSize);

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);

    int spaceW = width - iconSize * images.size();
    int x = (spaceW - (images.size() - 1) * padding) / 2;
    int y = (height - iconSize) / 2;
    int i = 0;

    for (auto iter : images) {
        iter.paint(&painter, QRect(x, y, iconSize, iconSize));
        x += iconSize + padding;
    }
    painter.end();
    return pixmap.save(out);
}

bool genGtk(QString name,int width,int height,double scaleFactor,QString dest)
{
    double gdkWinScalingFactor = 1.0;

    if (scaleFactor > 1.7) {
        // 根据 startdde 的逻辑，此种条件下 gtk 窗口放大为 2 倍
        gdkWinScalingFactor = 2.0;
    }

    width = static_cast<int>(width* scaleFactor / gdkWinScalingFactor);
    height = static_cast<int>(height* scaleFactor / gdkWinScalingFactor);

    QString cmd = "/usr/lib/deepin-api/gtk-thumbnailer";
    QStringList args{"-theme", name,
                "-dest", dest,
                "-width", QString::number(width),
                "-height", QString::number(height),
                "-force"};

    QProcess process;
    process.setProgram(cmd);
    process.setArguments(args);
    process.start();

    return true;
}

QVector<QImage*> getCursors(QString dir, int size)
{
    qDebug()<<"dir :"<<dir;
    QVector<QImage*> images;
    for(auto cursors : presentCursors)
    {
        for(auto iter : cursors)
        {
            bool bAdd = true;
            QFile file(dir + "/" + iter);
            QImage* image = loadXCursor(dir + "/" + iter,size);
            if(!image)
            {
                continue;
            }

            for(auto tempImage: images)
            {
                if(tempImage->cacheKey() == image->cacheKey())
                {
                    bAdd = false;
                }
            }

            if(bAdd)
            {
                if(image->width() != size)
                {
                    *image = image->scaledToWidth(size);
                }
                images.push_back(image);
                break;
            }
        }

    }

    return images;
}

QList<QIcon> getIcons(QString theme, int size)
{
    QList<QIcon> images;
    QIcon::setThemeName(theme);
    for(const auto &icons : qAsConst(presentIcons)) {
        for (auto &&iconName:icons) {
            QIcon icon(new CompatibleEngine(iconName)); // QIcon::fromTheme(iconName); DTK中DCI支持不完整，暂用CompatibleEngine处理兼容
            if (!icon.isNull()) {
                images.append(icon);
                break;
            }
        }
    }

    return images;
}

QImage CompositeImages(QVector<QImage*> images, int width, int height, int iconSize,int padding)
{
    QImage image(width,height,QImage::Format_RGBA8888);
    if(images.isEmpty())
    {
        return image;
    }

    while (images.size()>9) {
        images.pop_back();
    }
    QColor bmpBackA(254,254,254,0);
    for(int i=0;i< image.width();++i)
    {
        for(int j=0;j<image.height();++j)
        {
            image.setPixelColor(i,j,bmpBackA);
        }

    }
    int spaceW = width - iconSize*images.size();
    int x = (spaceW-(images.size() -1)*padding) / 2;
    int y = (height - iconSize) / 2;

    QPainter painter;
    painter.begin(&image);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);

    int i=0;

    for(auto iter : images)
    {
        qDebug()<<i<<" :"<<(*iter).bits();
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(x,y,*iter);
        x+=iconSize +padding;
    }
    painter.end();

    return image;
}

static QImage* fromXCurorImageToQImage(XcursorImage* image)
{
    // golang 原代码 pixels := (*[1 << 12]C.XcursorPixel)(unsafe.Pointer(img.pixels))[:n:n]
    XcursorPixel* tempPixel = &image->pixels[0];
    QImage* qImage =new QImage(static_cast<int>(image->width),static_cast<int>(image->height),QImage::Format_RGBA8888);

    int i=0;
    for(int y=0; y<static_cast<int>(image->height); y++)
    {
        for(int x=0; x<static_cast<int>(image->width); x++)
        {
            int alpha = tempPixel[i] >> 24;
            int red = (tempPixel[i] >> 16) & 0xff;
            int green = (tempPixel[i] >>8) & 0xff;
            int blue = tempPixel[i] & 0xff;
            QColor color(red,green,blue,alpha);
            qImage->setPixelColor(x,y,color);
            i++;
        }
    }

    return qImage;
}

QImage* loadXCursor(QString fileName, int size)
{
    XcursorImage* xcursorImage = XcursorFilenameLoadImage(fileName.toStdString().c_str(),size);
    if(xcursorImage == nullptr)
    {
        return nullptr;
    }
    QImage* image =fromXCurorImageToQImage(xcursorImage);
    delete xcursorImage;

    return image;
}

QString getGlobal(QString id, QString descFile)
{
    if (!checkScaleFactor()) {
        qInfo() << "scaleFactor <= 0";
        return "";
    }
    KeyFile keyFile(',');
    keyFile.loadFile(descFile);
    QStringList example = keyFile.getStrList("Deepin Theme", "Example");
    if (!example.isEmpty()) {
        bool isdark = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType;
        QString path = isdark ? example.last() : example.first();
        QFileInfo file(path);
        if (file.isRelative()) {
            QFileInfo themefile(descFile);
            file.setFile(themefile.absoluteDir(), path);
            path = file.absoluteFilePath();
        }
        return path;
    }
    return QString();
}

QString getGtk(QString id, QString descFile)
{
    if (!checkScaleFactor()) {
        qInfo() << "scaleFactor <= 0";
        return "";
    }

    QString out = prepareOutputPath("gtk", id, cursorVersion);
    if (!shouldGenerateNewCursor(descFile, out)) {
        return out;
    }

    double scaleFactor = getScaleFactor();

    if(!genGtk(id,width,height,scaleFactor,out))
    {
        return "";
    }

    return out;
}

QString getIcon(QString id, QString descFile)
{
    if (!checkScaleFactor()) {
        qInfo() << "scaleFactor <= 0";
        return "";
    }

    QString out = prepareOutputPath("icon", id, iconVersion);
    if (!shouldGenerateNew(descFile, out)) {
        return out;
    }

    double scaleFactor = getScaleFactor();

    if(!genIcon(id, width, height, scaleFactor, out)) {
        return "";
    }

    return out;
}

bool shouldGenerateNew(QString descFile, QString out)
{
    QFileInfo outFileInfo(out);
    if (!outFileInfo.exists()) {
        return true;
    }
    QFileInfo descFileInfo(descFile);
    if (descFileInfo.isFile()) {
        descFileInfo = QFileInfo(descFileInfo.dir().absolutePath());
    }

    return descFileInfo.lastModified() > outFileInfo.lastModified();
}

bool shouldGenerateNewCursor(QString descFile, QString out)
{
    QString dir = DFile::dir(descFile);
    descFile = dir;

    return shouldGenerateNew(descFile,out);
}

QString prepareOutputPath(QString type0, QString id, int version)
{
    QString scaleDir = getScaleDir();
    QString typeDir = getTypeDir(type0,version);

    QString cacheDir = utils::GetUserCacheDir();
    cacheDir+="/deepin/dde-api/theme_thumb";
    QString dirPath = cacheDir +"/"+scaleDir+"/"+typeDir;

    QDir dir;
    if(!dir.mkpath(dirPath))
    {
        return "";
    }

    return dirPath+"/"+id+".png";
}

void UpdateScaleFactor(double scaleFactor)
{
    g_ScaleFactor = scaleFactor;
}
