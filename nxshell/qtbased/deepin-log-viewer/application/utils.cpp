// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "logsettings.h"
#include "dbusmanager.h"
#include "dbusproxy/dldbushandler.h"

#include <math.h>
#include <pwd.h>

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QProcessEnvironment>
#include <QTime>
#include <QLoggingCategory>
#include <QDBusInterface>

#include <polkit-qt5-1/PolkitQt1/Authority>
using namespace PolkitQt1;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logUtils, "org.deepin.log.viewer.utils")
#else
Q_LOGGING_CATEGORY(logUtils, "org.deepin.log.viewer.utils", QtInfoMsg)
#endif
const QString DCONFIG_APPID = "org.deepin.log.viewer";

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;
QMap<QString, QStringList> Utils::m_mapAuditType2EventType;
int Utils::specialComType = -1;
// 1.sudo权限运行deepin-log-viewer，QDir::homePath返回的是/root
// 此种情况下，使用freedesktop dbus接口获取当前登录用户家目录，以便能正确导出日志
// 2.systemd服务启动deepin-log-viewer，QDir::homePath返回的是/，因该方式下freedesktop dbus接口获取为空，只能将/root作为homePath
QString Utils::homePath = ((QDir::homePath() != "/root" && QDir::homePath() != "/") ? QDir::homePath() : (QDir::homePath() == "/" ? "/root" : DBusManager::getHomePathByFreeDesktop()));
bool Utils::runInCmd = false;

// 高频重复崩溃记录exe路径名单
//const QString COREDUMP_REPEAT_CONFIG_PATH = "/var/cache/deepin/deepin-log-viewer/repeatCoredumpApp.list";
const QString COREDUMP_REPEAT_CONFIG_PATH = Utils::homePath + "/.cache/deepin/deepin-log-viewer/repeatCoredumpApp.list";
const float COREDUMP_HIGH_REPETITION = 0.8f;
const int COREDUMP_TIME_THRESHOLD = 3;


Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils()
{
}

QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
    }

    return qss;
}

QString Utils::getConfigPath()
{
    QDir dir(QDir(Utils::homePath + "/.config")
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

QString Utils::getAppDataPath()
{
    QDir dir(QDir(Utils::homePath + "/.local/share")
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

bool Utils::isFontMimeType(const QString &filePath)
{
    const QString mimeName = QMimeDatabase().mimeTypeForFile(filePath).name();
    ;

    if (mimeName.startsWith("font/") || mimeName.startsWith("application/x-font")) {
        return true;
    }

    return false;
}

bool Utils::isTextFileType(const QString &filePath)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath);
    //标准文本文件和空白无后缀文件均计入文本文件范围
    if (mime.inherits("text/plain") || mime.inherits("application/x-zerosize")) {
        return true;
    }
    return false;
}

QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}

QPixmap Utils::renderSVG(const QString &filePath, const QSize &size)
{
    if (m_imgCacheHash.contains(filePath)) {
        return m_imgCacheHash.value(filePath);
    }

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

    m_imgCacheHash.insert(filePath, pixmap);

    return pixmap;
}

QString Utils::loadFontFamilyFromFiles(const QString &fontFileName)
{
    if (m_fontNameCache.contains(fontFileName)) {
        return m_fontNameCache.value(fontFileName);
    }

    QString fontFamilyName = "";

    QFile fontFile(fontFileName);
    if (!fontFile.open(QIODevice::ReadOnly)) {
        return fontFamilyName;
    }

    int loadedFontID = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if (!loadedFontFamilies.empty()) {
        fontFamilyName = loadedFontFamilies.at(0);
    }
    fontFile.close();

    m_fontNameCache.insert(fontFileName, fontFamilyName);
    return fontFamilyName;
}

QString Utils::getReplaceColorStr(const char *d)
{
    QByteArray byteChar(d);
    byteChar = replaceEmptyByteArray(byteChar);
    QString d_str = QString(byteChar);
    d_str.replace(QRegExp("\\x1B\\[\\d+(;\\d+){0,2}m"), "");
    d_str.replace(QRegExp("\\002"), "");
    return  d_str;
}

/**
 * @brief Utils::replaceEmptyByteArray 替换从qproccess获取的日志信息的字符中的空字符串 替换 \u0000,不然QByteArray会忽略这以后的内容
 * @param iReplaceStr要替换的字符串
 * @return 替换过的字符串
 */
QByteArray Utils::replaceEmptyByteArray(const QByteArray &iReplaceStr)
{
    QByteArray byteOutput = iReplaceStr;
    //\u0000是空字符，\x01是标题开始，出现于系统日志部分进程进程名称最后一个字符，不替换英文情况下显示错误
    return byteOutput.replace('\u0000', "").replace("\x01", "");
}
/**
 * @brief Utils::isErroCommand 判断qproccess获取日志的返回值是否为报错
 * @param str 要判断的字符串结果
 * @return 见头文件中CommandErrorType的定义
 */
Utils::CommandErrorType Utils::isErroCommand(const QString &str)
{
    if (str.contains("权限") || str.contains("permission", Qt::CaseInsensitive)) {
        return PermissionError;
    }
    if (str.contains("请重试") || str.contains("retry", Qt::CaseInsensitive)) {
        return RetryError;
    }
    return NoError;
}

bool Utils::checkAndDeleteDir(const QString &iFilePath)
{
    QFileInfo tempFileInfo(iFilePath);

    if (tempFileInfo.isDir()) {
        deleteDir(iFilePath);
        return true;
    } else if (tempFileInfo.isFile()) {
        QFile deleteFile(iFilePath);
        return deleteFile.remove();
    }
    return false;
}

bool Utils::deleteDir(const QString &iFilePath)
{
    QDir directory(iFilePath);
    if (!directory.exists()) {
        return true;
    }

    QString srcPath = QDir::toNativeSeparators(iFilePath);
    if (!srcPath.endsWith(QDir::separator()))
        srcPath += QDir::separator();

    QStringList fileNames = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    bool error = false;
    for (QStringList::size_type i = 0; i != fileNames.size(); ++i) {
        QString filePath = srcPath + fileNames.at(i);
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            QFile::setPermissions(filePath, QFile::WriteOwner);
            if (!QFile::remove(filePath)) {
                error = true;
            }
        } else if (fileInfo.isDir()) {
            if (!deleteDir(filePath)) {
                error = true;
            }
        }
    }

    if (!directory.rmdir(QDir::toNativeSeparators(directory.path()))) {
        error = true;
    }

    return !error;
}

void Utils::replaceColorfulFont(QString *iStr)
{
    iStr->replace(QRegExp("[[0-9]{1,2}m"), "");
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

bool Utils::isTreeland()
{
    return qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") == QStringLiteral("TreeLand");
}

bool Utils::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(static_cast<int>(msec));

    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    return true;
}

QString Utils::mkMutiDir(const QString &path)
{
    QDir dir(path);
    if (path.isEmpty() || dir.exists()) {
        return path;
    }
    QString parentDir = mkMutiDir(path.mid(0, path.lastIndexOf('/')));
    QString dirname = path.mid(path.lastIndexOf('/') + 1);
    QDir parentPath(parentDir);
    if (!dirname.isEmpty()) {
        bool bRet = parentPath.mkpath(dirname);
        if (!bRet)
            qCWarning(logUtils) << QString("mkpath: unable to create directory '%1' in path: '%2/', please confirm if the file exists.").arg(dirname).arg(parentDir);
    }
    return parentDir + "/" + dirname;
}

bool Utils::checkAuthorization(const QString &actionId, qint64 applicationPid)
{
    Authority::Result result;

    result = Authority::instance()->checkAuthorizationSync(actionId, UnixProcessSubject(applicationPid),
                                                           Authority::AllowUserInteraction);
    return result == Authority::Yes ? true : false;
}

QString Utils::osVersion()
{
    QProcess *unlock = new QProcess;
    unlock->start("lsb_release", {"-r"});
    unlock->waitForFinished();
    auto output = unlock->readAllStandardOutput();
    auto str = QString::fromUtf8(output);
    QRegExp re("\t.+\n");
    QString osVerStr;
    if (re.indexIn(str) > -1) {
        auto result = re.cap(0);
        osVerStr = result.remove(0, 1).remove(result.size() - 1, 1);
    }
    unlock->deleteLater();
    return osVerStr;
}

QString Utils::auditType(const QString &eventType)
{
    QMapIterator<QString, QStringList> it(m_mapAuditType2EventType);
    while (it.hasNext()) {
        it.next();
        if (it.value().indexOf(eventType) != -1)
            return it.key();
    }

    return "";
}

double Utils::convertToMB(quint64 cap, const int size/* = 1024*/)
{
    static QString type[] = {" B", " KB", " MB"};

    double dc = cap;
    double ds = size;

    for (size_t p = 0; p < sizeof(type); ++p) {
        if (cap < pow(size, p + 1) || p == sizeof(type) - 1)
            return dc / pow(ds, p);
    }

    return 0.0;
}

QString Utils::getUserNamebyUID(uint uid)
{
    struct passwd * pwd;
    pwd = getpwuid(uid);
    if (nullptr == pwd) {
        qCWarning(logUtils) << QString("unknown uid:%1").arg(uid);
        return "";
    }

    return pwd->pw_name;
}

QString Utils::getUserHomePathByUID(uint uid)
{
    struct passwd * pwd;
    pwd = getpwuid(uid);
    if (nullptr == pwd) {
        qCWarning(logUtils) << QString("unknown uid:%1").arg(uid);
        return "";
    }

    return pwd->pw_dir;
}

QString Utils::getCurrentUserName()
{
    // 获取当前系统用户名
    struct passwd* pwd = getpwuid(getuid());
    return pwd->pw_name;
}

bool Utils::isValidUserName(const QString &userName)
{
    bool bValidUserName = false;
    QDBusInterface interface("com.deepin.daemon.Accounts", "/com/deepin/daemon/Accounts", "com.deepin.daemon.Accounts", QDBusConnection::systemBus());
    QStringList userList = qvariant_cast< QStringList >(interface.property("UserList"));
    for (auto strUser : userList) {
        uint uid = strUser.mid(strUser.lastIndexOf("User") + 4).toUInt();
        QString tempUserName = getUserNamebyUID(uid);
        if(tempUserName == userName) {
            bValidUserName = true;
            break;
        }
    }

    return bValidUserName;
}

bool Utils::isCoredumpctlExist()
{
    bool isCoredumpctlExist = false;
    QDir dir("/usr/bin");
    QStringList list = dir.entryList(QStringList() << (QString("coredumpctl") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    for (int i = 0; i < list.count(); i++) {
        if("coredumpctl" == list[i]) {
            isCoredumpctlExist = true;
            break;
        }
    }
    return isCoredumpctlExist;
}

QString Utils::getHomePath(const QString &userName)
{
    QString uName("");
    if (!userName.isEmpty())
        uName = userName;
    else
        uName = getCurrentUserName();

    QString homePath = "";
    if (isValidUserName(uName)) {
        QProcess *unlock = new QProcess;
        unlock->start("sh", QStringList() << "-c" << QString("cat /etc/passwd | grep %1").arg(uName));
        unlock->waitForFinished();
        auto output = unlock->readAllStandardOutput();
        auto str = QString::fromUtf8(output);
        homePath = str.mid(str.indexOf("::") + 2).split(":").first();
    }

    // 根据用户名获取家目录失败，默认采用QDir::homePath()作为homePath
    QDir dir(homePath);
    if (!dir.exists() || homePath.isEmpty())
        homePath = QDir::homePath();

    qCDebug(logUtils) << "userName: " << uName << "homePath:" << homePath;

    return homePath;
}

QString Utils::appName(const QString &filePath)
{
    QString ret;
    if (filePath.isEmpty())
        return ret;

    QStringList strList = filePath.split("/");
    if (strList.count() < 2) {
        if (filePath.contains("."))
            ret = filePath.section(".", 0, 0);
        else {
            ret = filePath;
        }
        return ret;
    }

    QString tmpPath = filePath;
    if (tmpPath.endsWith('/'))
        tmpPath = tmpPath.remove(tmpPath.size() - 1, 1);
    QString desStr = tmpPath.section("/", -1);
    ret = desStr.mid(0, desStr.lastIndexOf("."));
    return ret;
}

void Utils::resetToNormalAuth(const QString &path)
{
    QFileInfo fi(path);
    if (!path.isEmpty() && fi.exists()) {
        qInfo() << "resetToNormalAuth: " << path;
        QProcess procss;
        if (fi.isDir())
            procss.setWorkingDirectory(path);
        else
            procss.setWorkingDirectory(fi.absolutePath());

        QStringList arg = {"-c"};
        arg.append(QString("chmod -R 777 '%1'").arg(path));
        procss.start("/bin/bash", arg);
        procss.waitForFinished(-1);
    }
}

QList<LOG_REPEAT_COREDUMP_INFO> Utils::countRepeatCoredumps(qint64 timeBegin, qint64 timeEnd)
{
    QList<LOG_REPEAT_COREDUMP_INFO> result;
    QString data = DLDBusHandler::instance()->executeCmd("coredumpctl-list");
    QStringList strList = data.split('\n', QString::SkipEmptyParts);
    int total = 0;
    for (int i = strList.size() - 1; i >= 0; --i) {
        QString str = strList.at(i);
        if (str.trimmed().isEmpty())
            continue;
        QStringList tmpList = str.split(" ", QString::SkipEmptyParts);
        if (tmpList.count() < 10)
            continue;

        QDateTime dt = QDateTime::fromString(tmpList[1] + " " + tmpList[2], "yyyy-MM-dd hh:mm:ss");
        if (timeBegin > 0 && timeEnd > 0) {
            if (dt.toMSecsSinceEpoch() < timeBegin || dt.toMSecsSinceEpoch() > timeEnd)
                continue;
        }

        total++;
        QString exePath = tmpList[9];

        auto it = std::find_if(result.begin(), result.end(), [&exePath](const LOG_REPEAT_COREDUMP_INFO& item){
            return exePath == item.exePath;
        });
        if (it == result.end()) {
            LOG_REPEAT_COREDUMP_INFO info;
            info.exePath = exePath;
            info.times = 1;
            info.repetitionRate = 0.0;
            result.push_back(info);
        } else {
            it->times++;
        }
    }

    // 计算重复率
    for (auto &info : result) {
        if (info.times > 1) {
            info.repetitionRate = static_cast<float>(info.times) / total;
        }
    }

    return result;
}

QStringList Utils::getRepeatCoredumpExePaths()
{
    QFile file(COREDUMP_REPEAT_CONFIG_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringList();

    QString data = file.readAll();

    file.close();

    return data.split(' ');
}

void Utils::updateRepeatCoredumpExePaths(const QList<LOG_REPEAT_COREDUMP_INFO> &infos)
{
    // 每隔24小时强制清空一次名单内容，以便重复的崩溃exe路径是最新的
    QFileInfo fi(COREDUMP_REPEAT_CONFIG_PATH);
    if (fi.birthTime().daysTo(QDateTime::currentDateTime()) >= 1) {
        if (QFile::exists(COREDUMP_REPEAT_CONFIG_PATH))
                QFile::remove(COREDUMP_REPEAT_CONFIG_PATH);
    }

    QDateTime bt = fi.birthTime();
    // 目录不存在，则创建目录
    if (!QFileInfo::exists(fi.absolutePath())) {
        QDir dir;
        dir.mkpath(fi.absolutePath());
    }

    // 确定当前时间范围的崩溃数据高频重复路径
    QStringList newRepatExePaths;
    for (auto info : infos) {
        if (info.repetitionRate > COREDUMP_HIGH_REPETITION || info.times >= COREDUMP_TIME_THRESHOLD)
            newRepatExePaths.push_back(info.exePath);
    }

    if (newRepatExePaths.size() == 0)
        return;

    QStringList curReaptExePaths = getRepeatCoredumpExePaths();
    for (auto exePath : newRepatExePaths) {
        if (curReaptExePaths.indexOf(exePath) == -1)
            curReaptExePaths.push_back(exePath);
    }

    QFile file(COREDUMP_REPEAT_CONFIG_PATH);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(logUtils) << "failed to open coredump repeat config file:" << COREDUMP_REPEAT_CONFIG_PATH;
        return;
    }

    QTextStream out(&file);
    out << curReaptExePaths.join(' ');

    file.close();
}

#ifdef DTKCORE_CLASS_DConfigFile
LoggerRules::LoggerRules(QObject *parent)
    : QObject(parent), m_rules(""), m_config(nullptr) {
}

LoggerRules::~LoggerRules() { m_config->deleteLater(); }

void LoggerRules::initLoggerRules()
{
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    qunsetenv("QT_LOGGING_RULES");

    // set env
    m_rules = logRules;

    // set dconfig
    m_config = Dtk::Core::DConfig::create(DCONFIG_APPID, DCONFIG_APPID);
    logRules = m_config->value("log_rules").toByteArray();
    appendRules(logRules);
    setRules(m_rules);

    // watch dconfig
    connect(m_config, &Dtk::Core::DConfig::valueChanged, this, [this](const QString &key) {
      if (key == "log_rules") {
          setRules(m_config->value(key).toByteArray());
          qCDebug(logUtils) << "value changed:" << key;
      }
    });
}

void LoggerRules::setRules(const QString &rules) {
  auto tmpRules = rules;
  m_rules = tmpRules.replace(";", "\n");
  QLoggingCategory::setFilterRules(m_rules);
}

void LoggerRules::appendRules(const QString &rules) {
  QString tmpRules = rules;
  tmpRules = tmpRules.replace(";", "\n");
  auto tmplist = tmpRules.split('\n');
  for (int i = 0; i < tmplist.count(); i++)
    if (m_rules.contains(tmplist.at(i))) {
      tmplist.removeAt(i);
      i--;
    }
  if (tmplist.isEmpty())
    return;
  m_rules.isEmpty() ? m_rules = tmplist.join("\n")
                    : m_rules += "\n" + tmplist.join("\n");
}
#endif
