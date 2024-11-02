#include "variable.h"

//项目名称
const QString Variable::PROGRAM_NAME = QString("lingmo-photo-viewer");
//临时文件路径
const QString Variable::TEMP_PATH = Variable::creatTempPath();

// QGSettings服务名称
const QString Variable::PHOTO_VIEW_GSETTINGS_SERVICENAME = QString("org.lingmo-photo-viewer.settings");
// GSettings
QGSettings *Variable::m_settings = Variable::getSettings();

// DBUS服务名称
const QString Variable::PHOTO_VIEW_DBUS_SERVICENAME = QString("org.lingmo.lingmo_photo_viewer");
// DBUS路径
const QString Variable::PHOTO_VIEW_DBUS_PARH = QString("/");
// DBUS接口
const QString Variable::PHOTO_VIEW_DBUS_INTERFACE = QString("lingmo_photo_viewer.commands");

//日志级别
QtMsgType Variable::g_logLevel = Variable::getLogLevel();
//日志路径
const QPair<QString, QString> Variable::LOG_PATH = Variable::getLogPath();
//日志是否写入文件
bool Variable::g_logToFile = Variable::getSettings("log-to-file").toBool();
//最大日志文件大小
const qint64 Variable::MAX_LOG_SIZE = 1024 * 1024; // 1MB

//支持的命令列表
const QMap<QString, QString> Variable::SUPPORT_CMD = Variable::getSupportCmd();

// opencv支持的格式列表
const QStringList Variable::opencvCanSupportFormats =
    { //"exr","EXR","tiff","tif",授权问题，不打开，opencv只支持其查看，不支持写入。此格式支持设置为壁纸
        /* "pbm", "pgm", "ppm"*/};
// opencv不支持的格式列表
const QStringList Variable::opencvCannotSupportFormats = { //"JP2"待添加,xbm不支持
    "tga", "svg", "gif", "apng", "ico", "xpm"};
// const QStringList Variable::opencvCannotSupportFormats={//"JP2"待添加,xbm不支持
//    "TGA","SVG","GIF","APNG","ICO",
//    "tga","svg","gif","apng","ico"};
const QStringList Variable::freeimageCanSupportFormats =
    { // 10种可读取可保存的格式"jp2"和"j2k",因涉及版权问题，暂时去掉，不打开
        "exr", "psd", "jfi", "jif", "j2k", "jp2", "jng", "wbmp", "xbm", "tiff", "tif", "webp",
        "pnm", "bmp", "dib", "sr",  "ras", "pbm", "pgm", "ppm",  "png", "jpg",  "jpe", "jpeg"};
const QString Variable::SUPPORT_FORMATS_CLASSIFY = "(*.jpg *.jpe *.jpeg);;"
                                                   "(*.pnm *.pbm);;"
                                                   "(*.pgm *.ppm);;"
                                                   "(*.png *.apng);;"
                                                   "(*.ico);;(*.sr);;"
                                                   "(*.ras);;(*.tga);;"
                                                   "(*.svg);;(*.gif);;"
                                                   "(*.webp);;(*.bmp *.dib);;"
                                                   "(*.exr);;(*.psd);;(*.jfi *.jif);;"
                                                   "(*.tif *.tiff);;"
                                                   "(*.j2k *.jp2 *.jng);;(*.wbmp);;"
                                                   "(*.xbm *.xpm)";
//另存为
const QString Variable::SUPPORT_FORMATS_SAVEAS = "PNG(*.png);;JPE(*.jpe);;JPEG(*.jpeg);;"
                                                 "JPG(*.jpg);;"
                                                 "ICO(*.ico);;"
                                                 "BMP(*.bmp);;"
                                                 "JNG(*.jng);;"
                                                 "XPM(*.xpm);;TIFF(*.tiff);;TIF(*.tif);;J2K(*.j2k);;JP2(*.jp2)";
//".gif",".apng",GIF(*.gif);;APNG(*.apng);;
const QStringList Variable::SUPPORT_FORMATS_END = {".tiff", ".tif", ".j2k",  ".jp2", ".jpg", ".jpe", ".jpeg", ".png",
                                                   ".ico",  ".svg", ".webp", ".bmp", ".dib", ".psd", ".jng",  ".xpm"};
//壁纸支持的格式列表
const QStringList Variable::BACKGROUND_SUPPORT_FORMATS =
    { //经测试
      //"jp2","tga","dib","pbm","ppm"，"sr","ras",这几种格式不支持设置为壁纸"tiff""tif"暂时因版权问题不打开，再这里也先去掉
        "jpg", "jpe", "jpeg", "pgm", "pnm", "png", "bmp", "svg", "gif", "apng", "tiff", "tif"};
//支持的格式列表
const QStringList Variable::SUPPORT_FORMATS = Variable::creatSupportFormats();
//不支持转存为此种格式
const QStringList Variable::SUPPORT_FORMATS_NOT = {".jif", ".svg", ".jfi", ".exr", ".wbmp", ".tga", ".xbm",
                                                   ".pgm", ".pbm", ".sr",  ".ras", ".pnm",  ".ppm"};
//不支持裁剪
// const QStringList Variable::SUPPORT_FORMATS_NOT={".exr",".tga",".svg"};

const QSize Variable::ALBUM_IMAGE_SIZE = QSize(94, 56) /*QSize(96, 96)*/; //相册缩略图尺寸
const QSize Variable::NAVIGATION_SIZE = QSize(200, 133);                  //导航器尺寸
const int Variable::PICTURE_DEEPEN_KEY = 50;                              //图片加深值
const int Variable::REFRESH_RATE = 15;                                    //刷新间隔
const int Variable::RESIZE_KEY = 10;                                      //每次放大缩小的值-5-150
const int Variable::RESIZE_KEY_SECOND = 25;                               //每次放大缩小的值-150-300
const int Variable::RESIZE_KEY_THIRD = 50;                                //每次放大缩小的值-300-500
const int Variable::RESIZE_KEY_FOURTH = 100;                              //每次放大缩小的值-500-1000
const int Variable::RESIZE_KEY_MAX = 10000;                               //能够放大的最大值
const int Variable::RESIZE_KEY_MIN = 5;                                   //能够缩小的最小值
const int Variable::RESIZE_SPEED = 15;                                    //缩放速度
const int Variable::DEFAULT_MOVIE_TIME_INTERVAL = 100;                    //默认动图时间间隔
const int Variable::DEFAULT_WHITE_COLOR = 255;                            //默认白色色值
const int Variable::DEFAULT_BLACK_COLOR = 80;                             //默认黑色色值
const QString Variable::API_TYPE = QString("$api$");                      //是否为外部作为API调用的标识
int Variable::LOADIMAGE_NUM = 8;                                          //相册一次加载的数量
QString Variable::g_themeStyle = "";                                      //主题
QStringList Variable::g_damagedImage;                                     //损坏图片路径
const QString Variable::DAMAGED_IMAGE_BLACK = QString(":/res/res/damaged_imgB.png");
const QString Variable::DAMAGED_IMAGE_WHITE = QString(":/res/res/damaged_img.png");
DisplayMode displayMode = DisplayMode::NormalMode;
OperayteMode operayteMode = OperayteMode::NoOperate;
MattingOperayteMode mattingOperayteMode = MattingOperayteMode::NoOperate;
ScannerOperayteMode scannerOperayteMode = ScannerOperayteMode::NoOperate;
const double Variable::PERCENTAGE = 0.01; //百分比转换
QString Variable::startAppName = "";
QStringList Variable::startAppNameList = {"lingmo-gallery"};
QString Variable::platForm = QString("normal");
const QString Variable::SIGN_APP_NAME = "kolourpaint ";
bool Variable::g_hasIllegalSuffix = false;
bool Variable::g_delOpenNext = false;
bool Variable::g_needSaveas = false;
double Variable::g_acturalWidHeight = 570;
double Variable::g_acturalWidWidth = 1080;
bool Variable::g_needChangeAlbumPos = false;
//标注
bool Variable::g_allowChangeImage = true;
bool Variable::g_needImageSaveTip = false;
bool Variable::g_needEnterSign = false;
bool Variable::g_hasRotation = false;
bool Variable::g_mainlineVersion = false;
const QStringList Variable::MOVIETYPE = {"gif", "apng"};
bool Variable::g_isClosing = false;
QSize Variable::g_widSize = QSize(1080, 720);
QString Variable::g_currentImageType = "";
QString Variable::g_currentPath = "";
QImage Variable::g_tempImage = QImage();
int Variable::g_currentScale = 100;
QMap<QString, QImage> Variable::g_loadedImageMap = Variable::initImageMap();
bool Variable::g_nextOrPreviousTiff = false;
QStringList Variable::g_changedImageList = {""};
QGSettings *Variable::getSettings()
{
    QGSettings *mysetting = nullptr;
    if (QGSettings::isSchemaInstalled(PHOTO_VIEW_GSETTINGS_SERVICENAME.toLocal8Bit())) {
        mysetting = new QGSettings(PHOTO_VIEW_GSETTINGS_SERVICENAME.toLocal8Bit());
        QObject::connect(mysetting, &QGSettings::changed, &Variable::onGsettingChange);
    }
    return mysetting;
}

QMap<QString, QString> Variable::getSupportCmd()
{
    QMap<QString, QString> cmds;
    cmds.insert("-next", "下一张图片");
    cmds.insert("-back", "上一张图片");
    cmds.insert("-big", "放大显示图片");
    cmds.insert("-small", "缩小显示图片");
    cmds.insert("-origin", "显示原图");
    cmds.insert("-auto", "自适应窗口大小显示图片");
    cmds.insert("-rotate", "旋转图片");
    cmds.insert("-api", "外部接口  -api -flip [path] [cmd] [saveway]\n"
                        "  ├-cmd: -v 垂直翻转 -h 水平翻转 -c 顺时针旋转90度 | 可重复使用，例如-vhcc \n"
                        "  └-saveway:  -b 备份保存 -r 覆盖保存 | 可不填，不填相当于-b");
    return cmds;
}

void Variable::onGsettingChange(const QString &key)
{
    if (key == "logLevel") {
        Variable::g_logLevel = getLogLevel();
        return;
    }
    if (key == "logToFile") {
        Variable::g_logToFile = m_settings->get("logToFile").toBool();
        return;
    }
}

QPair<QString, QString> Variable::getLogPath()
{
    QString logPath = "/run/user/";
    logPath += QString::number(getuid()) + "/";
    //创建目录
    QDir dir;
    if (!dir.exists(logPath)) {
        dir.mkdir(logPath);
    }
    QPair<QString, QString> pair;
    pair.first = logPath + PROGRAM_NAME + "_1" + ".log";
    pair.second = logPath + PROGRAM_NAME + "_2" + ".log";
    return pair;
}

QtMsgType Variable::getLogLevel()
{
    QString level = m_settings->get("logLevel").toString().toLower();
    if (level == "warning" || level == "1") {
        return QtWarningMsg;
    }
    if (level == "critical" || level == "2") {
        return QtCriticalMsg;
    }
    if (level == "fatal" || level == "3") {
        return QtFatalMsg;
    }
    if (level == "info" || level == "4") {
        return QtInfoMsg;
    }
    return QtDebugMsg;
}

QString Variable::platFormType()
{
    QString platformName;
    kdk::kabase::SystemInformation systemInformation;
    platformName = systemInformation.getProjectCodeName();
    return platformName;
}

QMap<QString, QImage> Variable::initImageMap()
{
    QMap<QString, QImage> tempMap;
    tempMap.insert(0, "", QImage());
    return tempMap;
}

const QString Variable::creatTempPath()
{
    const QString filePath = "/tmp/." + PROGRAM_NAME + "/";
    QDir dir;
    if (!dir.exists(filePath)) {
        dir.mkdir(filePath);
    }
    return filePath;
}

QStringList Variable::creatSupportFormats()
{
    QStringList list;
    list.append(Variable::opencvCanSupportFormats);
    list.append(Variable::opencvCannotSupportFormats);
    list.append(Variable::freeimageCanSupportFormats);
    return list;
}

//写入配置文件
void Variable::setSettings(const QString &key, const QVariant &vlue)
{
    m_settings->set(key, vlue);
}

//读取配置文件
QVariant Variable::getSettings(const QString &key)
{
    QVariant setting;
    setting = m_settings->get(key);
    if (key == "imagePath") {
        if (setting.toString().isEmpty()) {
            return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        }
    }
    return setting;
}
//存加载的图片
void Variable::setLoadedImageMap(QString pathId, QImage pathImg)
{
    if (g_loadedImageMap.contains(pathId)) {
        g_loadedImageMap.remove(pathId);
    }
    g_loadedImageMap.insert(pathId, pathImg);
}
//取某张图片
QImage Variable::getLoadedImage(QString pathId)
{
    //返回已经加载过的图片
    if (g_loadedImageMap.contains(pathId)) {
        return g_loadedImageMap.value(pathId);
    }
    return QImage();
}
//删除已经加载过得图片
void Variable::delLoadedImage(QString pathId)
{
    if (g_loadedImageMap.contains(pathId)) {
        g_loadedImageMap.remove(pathId);
    }
}
//更改某个图片的id
void Variable::renameLoadedImagePath(QString oldId, QString newId)
{
    if (g_loadedImageMap.contains(oldId)) {
        g_loadedImageMap.insert(newId, g_loadedImageMap.value(oldId));
        g_loadedImageMap.remove(oldId);
    }
}

bool Variable::imageHasLoaded(QString pathId)
{
    if (g_loadedImageMap.contains(pathId)) {
        return true;
    }
    return false;
}
