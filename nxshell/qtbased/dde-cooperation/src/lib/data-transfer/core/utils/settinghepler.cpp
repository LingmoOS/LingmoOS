#include "settinghepler.h"

#include "common/log.h"

#include <net/helper/transferhepler.h>

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QGuiApplication>
#include <QScreen>
#include <QDir>
#include <QJsonDocument>

SettingHelper::SettingHelper()
    : QObject()
{
    initAppList();
}

SettingHelper::~SettingHelper() {}

SettingHelper *SettingHelper::instance()
{
    static SettingHelper ins;
    return &ins;
}

QJsonObject SettingHelper::ParseJson(const QString &filepath)
{
    QJsonObject jsonObj;
    QFile file(filepath);
    LOG << "Parsing the configuration file for transmission" << file.fileName().toStdString();
    if (!file.open(QIODevice::ReadOnly)) {
        WLOG << "could not open datajson file";
        return jsonObj;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        WLOG << "Parsing JSON data failed";
        return jsonObj;
    }
    jsonObj = jsonDoc.object();

    if (jsonObj.isEmpty())
        WLOG << "this job none file";

    return jsonObj;
}

bool SettingHelper::handleDataConfiguration(const QString &path)
{
    addTaskcounter(1);

    QDir pdir(path);
    QString filepath = pdir.absolutePath() + "/";
    QJsonObject jsonObj = ParseJson(filepath + "transfer.json");
    if (jsonObj.isEmpty()) {
        addTaskcounter(-1);
        WLOG << "transfer.json is invaild";
        emit TransferHelper::instance()->addResult(tr("Profiles"), false, tr("Wrong or missing profile"));
        return false;
    }

    //Configure file
    setFile(jsonObj, filepath);

    // Configure desktop wallpaper
    QString image = filepath + jsonObj["wallpapers"].toString();
    if (!jsonObj["wallpapers"].isNull())
        setWallpaper(image);

    //setBrowserBookMark
    if (!jsonObj["browserbookmark"].toString().isEmpty())
        setBrowserBookMark(filepath + jsonObj["browserbookmark"].toString());

    //installApps
    QJsonValue userFileValue = jsonObj["app"];
    if (userFileValue.isArray()) {
        const QJsonArray &userFileArray = userFileValue.toArray();
        for (const auto &value : userFileArray) {
            installApps(value.toString());
        }
    }
    addTaskcounter(-1);
    //remove dir
    pdir.removeRecursively();
    return true;
}

bool SettingHelper::setWallpaper(const QString &filepath)
{
    LOG << "Setting picture as wallpaper" << filepath.toStdString();

    QString service = "com.deepin.daemon.Appearance";
    QString path = "/com/deepin/daemon/Appearance";
    QString interfaceName = "com.deepin.daemon.Appearance";

    QDBusInterface interface(service, path, interfaceName);

    QString func = "SetMonitorBackground";
    QString screenName = QGuiApplication::screens().first()->name();
    QVariant monitorName = QVariant::fromValue(screenName);
    QVariant imageFile = QVariant::fromValue(filepath);

    QDBusMessage reply = interface.call(func, monitorName, imageFile);
    if (reply.type() == QDBusMessage::ReplyMessage) {
        DLOG << "SetMonitorBackground method called successfully";
        emit TransferHelper::instance()->addResult(tr("My Wallpaper"), true, tr("Transfer completed"));
        QFileInfo info(filepath);
        QString destination = QDir::homePath() + "/Pictures/ConvertedWallpaper.png";
        moveFile(filepath, destination);
        return true;
    } else {
        DLOG << "Failed to call SetMonitorBackground method";
        return false;
    }
}

bool SettingHelper::setBrowserBookMark(const QString &filepath)
{
    if (filepath.isEmpty())
        return true;
    QString targetDir = QDir::homePath() + "/.config/browser/Default/book/";
    QDir dir(targetDir);
    if (!dir.exists())
        dir.mkpath(".");

    QFileInfo info(filepath);
    if (info.suffix() != "json") {
        emit TransferHelper::instance()->addResult(tr("Browser Bookmarks"), false, tr("Format error"));
        return false;
    }

    QString targetfile = targetDir + info.fileName();
    LOG << "Set browser bookmarks" << filepath.toStdString() << targetfile.toStdString();

    bool success = moveFile(filepath, targetfile);
    LOG << "Set browser bookmarks" << targetfile.toStdString() << success;
    if (!success) {
        emit TransferHelper::instance()->addResult(tr("Browser Bookmarks"), false, tr("Setup failed, configuration can be imported manually"));
        return false;
    }
    emit TransferHelper::instance()->addResult(tr("BrowserBookMark"), true, tr("Transfer completed"));
    return true;
}

bool SettingHelper::installApps(const QString &app)
{
    if (app.isEmpty())
        return true;

    QString &package = applist[app];
    if (package.isEmpty()) {
        emit TransferHelper::instance()->addResult(app, false, tr("Installation failed, please go to the app store to install"));
        return false;
    }

    LOG << "Installing " << app.toStdString() << package.toStdString();

    QString service = "com.deepin.lastore";
    QString path = "/com/deepin/lastore";
    QString interfaceName = "com.deepin.lastore.Manager";

    QDBusInterface interface(service, path, interfaceName, QDBusConnection::systemBus());

    //Check if installed
    QString existfunc = "PackageExists";
    QDBusMessage existReply = interface.call(existfunc, package);
    if (existReply.type() == QDBusMessage::ReplyMessage) {
        bool isExist = existReply.arguments().at(0).toBool();
        if (isExist) {
            WLOG << app.toStdString() << "is installed";
            emit TransferHelper::instance()->addResult(app, true, tr("is installed"));
            return true;
        }
    }

    //installed
    QString func = "InstallPackage";

    QDBusMessage reply = interface.call(func, QString(), package);

    if (reply.type() != QDBusMessage::ReplyMessage) {
        WLOG << "Installing " << app.toStdString() << "false" << reply.errorMessage().toStdString();
        emit TransferHelper::instance()->addResult(app, false, tr("Installation failed, please go to the app store to install"));
        return false;
    }

    QString jobPath = reply.arguments().at(0).value<QDBusObjectPath>().path();
    LOG << "Installing " << app.toStdString() << "true" << jobPath.toStdString();

    bool success = QDBusConnection::systemBus().connect(service, jobPath, "org.freedesktop.DBus.Properties",
                                                        "PropertiesChanged", this, SLOT(onPropertiesChanged(QDBusMessage)));
    if (!success)
        WLOG << "Failed to connect to signal";

    emit TransferHelper::instance()->transferContent(tr("Installing"), app, 99, -2);

    addTaskcounter(1);
    return true;
}

void SettingHelper::onPropertiesChanged(const QDBusMessage &message)
{
    if (message.arguments().count() != 3)
        return;
    QVariantMap changedProps = qdbus_cast<QVariantMap>(message.arguments().at(1).value<QDBusArgument>());
    foreach (const QString &key, changedProps.keys()) {
        QVariant value = changedProps.value(key);
        QDBusInterface interface("com.deepin.lastore",
                                 message.path(),
                                 "com.deepin.lastore.Job",
                                 QDBusConnection::systemBus());
        auto packages = interface.property("Packages").toStringList();
        QString package;
        if (!packages.isEmpty())
            package = packages.first();
        QString app = applist.key(package);
        QString content = applist.key(package) + "  Key:" + key + "   Value:" + value.toString();

        //        if (key == "Progress") {
        //            float floatValue = value.toString().trimmed().toFloat();
        //            int percentageValue = qRound(floatValue * 100);
        //            QString progress = QString(tr("Installing Progress %1 %").arg(percentageValue));
        //            emit TransferHelper::instance()->transferContent(progress, app, 99, -2);
        //        }

        LOG << "Installing " << content.toStdString();

        if (key == "Status" && value == "succeed") {
            emit TransferHelper::instance()->addResult(app, true, tr("Transfer completed"));
            addTaskcounter(-1);
        }
        if (key == "Status" && value == "failed") {
            emit TransferHelper::instance()->addResult(app, false, tr("Installation failed, please go to the app store to install"));
            addTaskcounter(-1);
        }
    }
}

void SettingHelper::addTaskcounter(int value)
{
    taskcounter += value;

    if (taskcounter == 0)
        emit TransferHelper::instance()->transferFinished();
}

bool SettingHelper::setFile(QJsonObject jsonObj, QString filepath)
{
    QJsonValue userFileValue = jsonObj["user_file"];
    if (userFileValue.isArray()) {
        const QJsonArray &userFileArray = userFileValue.toArray();
        for (const auto &value : userFileArray) {
            QString filename = value.toString();
            QString targetFile = QDir::homePath() + "/" + filename;
            QString file = filepath + filename.mid(filename.indexOf('/') + 1);
            QFileInfo info = QFileInfo(targetFile);
            auto dir = info.dir();
            if (!dir.exists())
                dir.mkpath(".");
            bool res = moveFile(file, targetFile);
            QString des = res ? tr("Transfer completed") : tr("Transfer failed");
            emit TransferHelper::instance()->addResult(info.fileName(), res, des);
        }
    }
    LOG << jsonObj["user_file"].toString().toStdString();
    return true;
}

bool SettingHelper::moveFile(const QString &src, QString &dst)
{
    if (QFile::exists(dst)) {
        int i = 1;
        QString fileName = dst.split("/").last();
        QString dstDir = dst.remove(fileName);
        QStringList filenamelist = fileName.split(".");
        QString suffix;
        if (!QFileInfo(src).isDir() && filenamelist.size() >= 2) {
            suffix = filenamelist.last();
            suffix = "." + suffix;
        }
        QString baseName = fileName;
        baseName = fileName.remove(suffix);

        while (QFile::exists(dst)) {
            dst = dstDir + baseName + "(" + QString::number(i) + ")" + suffix;
            i++;
        }
    }
    QFile f(src);
    LOG << "moveFile dst: " << src.toStdString() << "   " << dst.toStdString();
    if (f.rename(dst))
        return true;

    WLOG << "moveFile error: " << f.errorString().toStdString();
    return false;
}

void SettingHelper::initAppList()
{
    QJsonObject jsonObj = ParseJson(":/fileResource/apps.json");
    if (jsonObj.isEmpty())
        return;
    for (const QString &app : jsonObj.keys()) {
        applist[app] = jsonObj.value(app).toObject().value("packageName").toString();
    }
    LOG << "SettingHelper::initAppList() finished";
}
