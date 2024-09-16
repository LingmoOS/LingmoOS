#include "transferutil.h"

#include "settinghepler.h"
#include "optionsmanager.h"
#include "common/commonutils.h"
#include "net/helper/transferhepler.h"

#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QApplication>
#include <QJsonDocument>
#include <QStorageInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>

TransferUtil::TransferUtil()
    : QObject()
{
    initOnlineState();
}

TransferUtil::~TransferUtil() {}

TransferUtil *TransferUtil::instance()
{
    static TransferUtil ins;
    return &ins;
}

void TransferUtil::initOnlineState()
{
    //初始化网络监控
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, [this]() {
        // 网络状态检测
        bool isConnected = deepin_cross::CommonUitls::getFirstIp().size() > 0;
        if (isConnected != online) {
            LOG << "Network is" << isConnected;
            online = isConnected;
            Q_EMIT TransferHelper::instance()->onlineStateChanged(isConnected);
        }
    });

    timer->start(1000);
}

QString TransferUtil::generateRandomNumber()
{
    QString randomString;
    for (int i = 0; i < 6; i++) {
        int randomNumber = QRandomGenerator::global()->bounded(10);   // 生成范围在 0 到 9 之间的随机数
        randomString.append(QString::number(randomNumber));   // 将随机数转换成字符串并拼接到结果字符串
    }
    return randomString;
}

QString TransferUtil::tempCacheDir()
{
    QString savePath =
            QString("%1/%2/%3/")
                    .arg(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation))
                    .arg(qApp->organizationName())
                    .arg(qApp->applicationName());   //~/.cache/deepin/xx

    QDir cacheDir(savePath);
    if (!cacheDir.exists())
        QDir().mkpath(savePath);

    return savePath;
}

QString TransferUtil::getJsonfile(const QJsonObject &jsonData, const QString &save)
{
    QString savePath = save;
    QJsonDocument jsonDoc(jsonData);

    if (savePath.isEmpty()) {
        savePath = QString("./transfer.json");
    } else {
        savePath += "/transfer.json";
    }

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson());
        file.close();
        DLOG << "JSON data exported to transfer.json";
        return savePath;
    } else {
        DLOG << "Failed to open file for writing.";
        return QString();
    }
}

QString TransferUtil::DownLoadDir(bool isComplete)
{
    if (isComplete)
        return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) +"/deepin-data-transfer-temp/";
    else
        return "deepin-data-transfer-temp";
}

int TransferUtil::getRemainSize()
{
    QStorageInfo storage("/home");
    auto remainSize = storage.bytesAvailable() / 1024 / 1024 / 1024;
    return static_cast<int>(remainSize);
}

bool TransferUtil::isUnfinishedJob(QString &content, const QString &connectIP)
{
    QString transtempPath(tempCacheDir() + connectIP + "transfer-temp.json");
    QFile f(transtempPath);
    if (!f.exists())
        return false;
    LOG << "has UnfinishedJob: " << transtempPath.toStdString();
    if (!f.open(QIODevice::ReadOnly)) {
        WLOG << "could not open file";
        return false;
    }
    QByteArray bytes = f.readAll();
    content = QString(bytes.data());
    f.close();

    return true;
}

bool TransferUtil::checkSize(const QString &filepath)
{
#ifdef __linux__
    QJsonObject jsonObj = SettingHelper::ParseJson(filepath);
    if (jsonObj.isEmpty())
        return false;
    auto sizestr = jsonObj["user_data"].toString();
    auto size = static_cast<int>(QVariant(sizestr).toLongLong() / 1024 / 1024 / 1024) * 2;
    LOG << "The actual size is " << sizestr.toStdString() << "B "
        << "Two times the space needs to be reserved" << size << "G";
    int remainSize = getRemainSize();
    if (size >= remainSize) {
        LOG << "outOfStorage" << size;
        emit TransferHelper::instance()->outOfStorage(size);
        TransferHelper::instance()->cancelTransferJob();
        TransferHelper::instance()->disconnectRemote();
        return false;
    }
#endif
    return true;
}

QString TransferUtil::getTransferJson(QStringList appList, QStringList fileList,
                                      QStringList browserList, QString bookmarksName,
                                      QString wallPaperName, QString tempSavePath)
{
    // add app
    QJsonArray appArray;
    for (auto app : appList) {
        appArray.append(app);
    }
    // add file
    QJsonArray fileArray;
    LOG << "home_path:" << QDir::homePath().toStdString();
    for (QString file : fileList) {
        if (file.contains(QDir::homePath()))
            file.replace(QDir::homePath() + "/", "");
        fileArray.append(file);
    }
    // add browser
    QJsonArray browserArray;
    for (QString browser : browserList) {
        browserArray.append(browser);
    }

    QJsonObject jsonObject;
    QString userData = OptionsManager::instance()->getUserOption(Options::KSelectFileSize)[0];
    jsonObject["user_data"] = userData;
    jsonObject["user_file"] = fileArray;
    if (!appArray.isEmpty())
        jsonObject["app"] = appArray;
    if (!wallPaperName.isEmpty())
        jsonObject["wallpapers"] = wallPaperName;
    if (!bookmarksName.isEmpty())
        jsonObject["browserbookmark"] = bookmarksName;
    if (!browserList.isEmpty())
        jsonObject["browsersName"] = browserArray;

    QString jsonfilePath = getJsonfile(jsonObject, QString(tempSavePath));
    LOG << "transfer.json save path:" << jsonfilePath.toStdString();
    return jsonfilePath;
}
