// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zdrivermanager.h"
#include "cupsconnection.h"
#include "cupsattrnames.h"
#include "qtconvert.h"
#include "config.h"
#include "common.h"
#include "ztaskinterface.h"
#include "printerservice.h"
#include "zdrivermanager_p.h"
#include "cupsppd.h"
#include "cupsconnectionfactory.h"
#include "zsettings.h"

#include <QProcess>
#include <QTcpSocket>
#include <QString>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QStringList>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QVersionNumber>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <map>

#include <DConfig>
DCORE_USE_NAMESPACE

static QMutex g_mutex;
static QMap<QString, QMap<QString, QString>> g_ppds; //所有ppd文件的字典，以device_id(没有device_id则以make_and_model)作为key
static QMap<QString, QMap<QString, QString> *> g_ppdsDirct; //将厂商和型号格式化之后作为key生成的字典，键值为g_ppds的key
static QMap<QString, QMap<QString, QString> *> g_ppdsMakeModelNames; //厂商和型号的字典，用于显示厂商和型号列表
static QMap<QString, QString> g_textPPd; //没有找到驱动的情况，默认的驱动
static QSet<QString> g_offlineDriver;

static int g_iStatus = TStat_None;

static const QString g_dbpath = "/opt/deepin/dde-printer/printer-drivers/deb-repo";
static const QString g_ppddbname = "ppd.db";
static const QString g_dbversion = "0.1.2";

static bool isLastoreConfigVerAccord()
{
    QString name = "org.deepin.lastore";
    DConfig *config = DConfig::create(name, name);

    if (config == nullptr) {
        return false;
    }

    QString version = config->value("version").toByteArray();
    config->deleteLater();

    qCDebug(COMMONMOUDLE) << "config ver:" << version;
    if (!version.isEmpty() && QVersionNumber::fromString(version) > QVersionNumber::fromString("1.0")) {
        return true;
    }

    return false;
}

static QMap<QString, QVariant> stringToVariant(const QMap<QString, QString> &driver)
{
    QMap<QString, QVariant> info;
    QStringList keys = driver.keys();
    foreach (QString key, keys) {
        info.insert(key, driver.value(key));
    }
    return info;
}

/*计算两个字符串的编辑距离,并返回相似度*/
double CalculateSimilarity(const string &source, const string &target)
{
    //step 1

    size_t n = source.length();
    size_t m = target.length();
    if (m == 0) return n;
    if (n == 0) return m;
    //Construct a matrix
    typedef vector< vector<size_t> >  Tmatrix;
    Tmatrix matrix(n + 1);
    for (size_t i = 0; i <= n; i++)  matrix[i].resize(m + 1);

    //step 2 Initialize

    for (size_t i = 1; i <= n; i++) matrix[i][0] = i;
    for (size_t i = 1; i <= m; i++) matrix[0][i] = i;

    //step 3
    for (size_t i = 1; i <= n; i++) {
        const char si = source[i - 1];
        //step 4
        for (size_t j = 1; j <= m; j++) {

            const char dj = target[j - 1];
            //step 5
            size_t cost;
            if (si == dj) {
                cost = 0;
            } else {
                cost = 1;
            }
            //step 6
            const size_t above = matrix[i - 1][j] + 1;
            const size_t left = matrix[i][j - 1] + 1;
            const size_t diag = matrix[i - 1][j - 1] + cost;
            matrix[i][j] = min(above, min(left, diag));

        }
    }//step7
    size_t distance = matrix[n][m];
    double similarity = 1 - (static_cast<double>(distance) / qMax(source.length(), target.length()));
    return similarity;
}

/*判断model里面的关键数字是否相同*/
bool isMatchModelIDNumber(const QString &modelNumber, const QString &mdl)
{
    bool match = false;
    //如果没有匹配到，取字符串里面的数字进行匹配
    if (!modelNumber.isEmpty()) {
        QStringList letters = mdl.split(" ");
        if (letters.contains(modelNumber))
            match = true;
    }
    return match;
}

/*获取model里面的关键数字型号*/
bool getModelNumber(const QString &model, QString &modelNumber)
{
    if (!model.isEmpty()) {
        bool success = false;
        QStringList parts = model.split(" ");
        foreach (const QString &part, parts) {
            part.toInt(&success, 10);
            if (success) {
                modelNumber = part;
                return true;
            }
        }
    }
    return false;
}

/*获取两个字符串相同前缀的长度*/
int findPrefixLength(const QString source, const QString target)
{
    if (source.isEmpty() || target.isEmpty())
        return 0;
    int slength = source.length();
    for (int i = 1; i < slength; i++) {
        if (target.indexOf(source.left(i)) != 0) {
            return i - 1;
        }
    }
    return source.length();
}

static QStringList findBestMatchPPDs(QStringList &models, QString mdl)
{
    QStringList list;
    /*去掉无用信息*/
    mdl = mdl.toLower();
    if (mdl.endsWith(" series")) {
        mdl.chop(QString(" series").length());
    }
    /*获取modelnumber*/
    QString modelNumber;
    if (!getModelNumber(mdl, modelNumber)) {
        qCWarning(COMMONMOUDLE) << "Can not find modelnumber from model";
    }
    /*1.先使用插入排序规则匹配前缀相似的类型*/
    int index = 0, len = 0;
    QString left, right;
    int leftMatch = 0, rightMatch = 0;


    //先通过字符串排序找到和mdl最相近的项
    mdl = mdl.toLower();
    models.append(mdl);
    models.sort();
    index = models.indexOf(mdl);
    len = models.count();
    if (index > 0)
        left = models[index - 1];
    if (index < (len - 1))
        right = models[index + 1];

    qCDebug(COMMONMOUDLE) << QString("Between %1 left: %2 righ: %3").arg(mdl).arg(left).arg(right);

    //找出左右两边和mdl匹配的字符数目
    len = mdl.length();

    leftMatch = findPrefixLength(left, mdl);
    rightMatch = findPrefixLength(right, mdl);

    //取匹配字符数较多并且超过mdl一半的项
    if (rightMatch > leftMatch && rightMatch > len / 2)
        list.append(right);
    else if (leftMatch > rightMatch && leftMatch > len / 2)
        list.append(left);

    /*2.使用编辑距离根据相似度进行匹配*/

    /*降序排列*/
    std::multimap<double, QString, std::greater<double>> simModelMap;
    /*先通过计算相似度找到和mdl最相近的项*/
    foreach (const QString &source, models) {
        double sim = CalculateSimilarity(source.toStdString(), mdl.toStdString());
        if (sim > 0.50) {
            simModelMap.insert(std::make_pair(sim, source));
        }
    }
    /*保证最多返回三个结果*/
    int need = 3 - list.count();
    index = 0;
    for (std::multimap<double, QString>::iterator iter = simModelMap.begin();
            iter != simModelMap.end();
            ++iter) {
        qCDebug(COMMONMOUDLE) << iter->second << " " << iter->first;
        if (index >= need)
            break;
        list.append(iter->second);
        index++;
    }

    /*3.当前两次匹配没有结果，根据关键型号数字搜索*/
    if (list.count() <= 0) {
        foreach (const QString &source, models) {
            if (isMatchModelIDNumber(modelNumber, source) && (!modelNumber.isEmpty())) {
                if (list.count() >= 3)
                    break;
                list.append(source);
            }
        }
    }

    qCInfo(COMMONMOUDLE) << QString("Got %1").arg(list.join(","));
    return list;
}

static QStringList getPPDNameFromCommandSet(QString strCMD)
{
    QStringList list, cmds;
    QStringList models;
    QStringList mdls;
    QMap<QString, QString> *pmodels = g_ppdsDirct.value("generic");

    if (pmodels)
        models = pmodels->keys();

    strCMD = strCMD.toLower();
    cmds = strCMD.split(",");
    if (cmds.contains("postscript") || cmds.contains("postscript2")
            || cmds.contains("postscript level 2 emulation"))
        mdls << "postscript";
    else if (cmds.contains("pclxl") || cmds.contains("pcl-xl") || cmds.contains("pcl6") || cmds.contains("pcl 6 emulation"))
        mdls << "PCL 6/PCL XL"
             << "PCL Laser";
    else if (cmds.contains("pcl5e"))
        mdls << "PCL 5e"
             << "PCL Laser";
    else if (cmds.contains("pcl5c"))
        mdls << "PCL 5c"
             << "PCL Laser";
    else if (cmds.contains("pcl5"))
        mdls << "PCL 5"
             << "PCL Laser";
    else if (cmds.contains("pcl"))
        mdls << "PCL 3"
             << "PCL Laser";
    if (cmds.contains("escpl2") || cmds.contains("esc/p2")
            || cmds.contains("escp2e"))
        mdls << "ESC/P Dot Matrix";

    foreach (QString mdl, mdls) {
        qCDebug(COMMONMOUDLE) << QString("Find for CMD: %1, mdl: %2").arg(strCMD).arg(mdl);
        list += findBestMatchPPDs(models, mdl);
    }

    return list;
}

static QList<QMap<QString, QVariant>> getFuzzyMatchDrivers(const QString &strMake, const QString &strModel, const QString &strCMD)
{
    QList<QMap<QString, QVariant>> list;

    if (strMake.isEmpty() || strModel.isEmpty()) {
        qCWarning(COMMONMOUDLE) << "printer info is invaild";
        return list;
    }

    if (g_ppdsDirct.contains(strMake)) {
        QMap<QString, QString> *modelMap = g_ppdsDirct.value(strMake);
        if (!modelMap)
            return list;

        QStringList modellist = modelMap->keys();
        QStringList models = findBestMatchPPDs(modellist, strModel);
        foreach (QString mdl, models) {
            QString key = modelMap->value(mdl);
            QMap<QString, QVariant> driver = stringToVariant(g_ppds.value(key.toLower()));
            if (!driver.isEmpty()) {
                driver.insert(SD_KEY_excat, false);
                list.append(driver);
            }
        }
    }

    if (!strCMD.isEmpty()) {
        QList<QMap<QString, QVariant>> templist;
        QStringList commandsets = strCMD.split(",");
        qCDebug(COMMONMOUDLE) << QString("Device commandsets: %1").arg(strCMD);
        //判断找到的驱动commandsets信息符不符合
        for (int i = 0; i < list.count(); i++) {
            QString devId = list[i].value(CUPS_PPD_ID).toString();
            QMap<QString, QString> devDirct = parseDeviceID(devId);
            QString strcmd = devDirct.value("CMD");
            qCDebug(COMMONMOUDLE) << QString("PPD commandsets: %1").arg(strcmd);
            QStringList cmds = strcmd.split(",");
            foreach (QString cmd, cmds) {
                if (commandsets.contains(cmd)) {
                    templist.append(list[i]);
                    break;
                }
            }
        }

        //如果所有的驱动都不符合，保留驱动，如果只是部分不符合，去掉不符合的驱动
        if (!templist.isEmpty()) {
            int count = list.count();
            list = templist;
            if (count != list.count())
                qCDebug(COMMONMOUDLE) << QString("Remove some ppds, that commandsets not support %1").arg(strCMD);
        }

        if (list.isEmpty()) {
            //通过commandsets查找驱动
            QStringList models = getPPDNameFromCommandSet(strCMD);
            foreach (QString mdl, models) {
                QMap<QString, QString> *modelMap = g_ppdsDirct.value(strMake);
                if (modelMap) {
                    QString key = modelMap->value(mdl);
                    QMap<QString, QVariant> driver = stringToVariant(g_ppds.value(key.toLower()));
                    if (!driver.isEmpty()) {
                        driver.insert(SD_KEY_excat, false);
                        list.append(driver);
                    }
                }
            }
        }
    }

    qCInfo(COMMONMOUDLE) << "Got dirver count: " << list.count();
    return list;
}

static QList<QMap<QString, QVariant>> getExactMatchDrivers(const QString &strMFG, const QString &strMDL)
{
    QList<QMap<QString, QVariant>> list;
    QStringList strKeys;

    if (strMFG.isEmpty() || strMDL.isEmpty())
        return list;

    QMap<QString, QString> *modelMap = g_ppdsDirct.value(strMFG);
    if (!modelMap)
        return list;

    strKeys = modelMap->values(strMDL);
    if (!strKeys.isEmpty()) {
        foreach (QString strKey, strKeys) {
            QMap<QString, QVariant> driver = stringToVariant(g_ppds.value(strKey.toLower()));
            if (!driver.isEmpty()) {
                driver.insert(SD_KEY_excat, true);
                list.append(driver);
            }
        }
    }

    qCInfo(COMMONMOUDLE) << "Got driver count: " << list.count();

    return list;
}

static bool addToDirct(const QString &strMake, const QString &strModel, const QString &key)
{
    QString makel, model;

    if (key.isEmpty()) {
        qCWarning(COMMONMOUDLE) << QString("Dirct key is empty");
        return false;
    }

    makel = normalize(strMake);
    model = normalize(strModel);
    if (makel.isEmpty() || model.isEmpty()) {
        qCWarning(COMMONMOUDLE) << QString("make_and_model failed for %1").arg(key);
        return false;
    }

    QMap<QString, QString> *modelMap = g_ppdsDirct.value(makel);
    if (!modelMap) {
        modelMap = new QMap<QString, QString>();
        g_ppdsDirct.insert(makel, modelMap);
    }

    QString strkey = modelMap->value(model);
    if (strkey.isEmpty() || !g_driverManager->isSamePPD(key, strkey)) {
        modelMap->insertMulti(model, key);
        qCDebug(COMMONMOUDLE) << QString("Insert %1#%2#%3 to dirct").arg(makel).arg(model).arg(key);
        return true;
    } else {
        qCDebug(COMMONMOUDLE) << "Remove duplicate driver" << strMake << strModel << strkey << "->" << key;
        return false;
    }
}

static void getPpdMakeModel(QString &strMake, QString &strModel, QMap<QString, QString> &list)
{
    //先通过device id解析make model
    QString key = list.value(CUPS_PPD_ID);
    QMap<QString, QString> idInfo = parseDeviceID(key);
    strMake = idInfo["MFG"];
    strModel = idInfo["MDL"];
    ppdMakeModelSplit(strMake + " " + strModel, strMake, strModel);

    //如果无效，再通过ppd-make-and-model解析make model
    if (strMake.isEmpty() || strModel.isEmpty()) {
        //qDebug() << "get make and model from strMakeAndModel";
        QString strMakeAndModel = list.value(CUPS_PPD_MAKE_MODEL);
        if (!strMakeAndModel.isEmpty()) {
            QString make, model;
            ppdMakeModelSplit(strMakeAndModel, make, model);
            //qDebug() << QString("split MakeAndModel make and model: %1 , %2").arg(make).arg(model);
            if (strModel.isEmpty())
                strModel = model;
            if (strMake.isEmpty())
                strMake = make;
        }
    }

    //如果无效，再通过ppd-product解析make model
    if (strMake.isEmpty() || strModel.isEmpty()) {
        //qDebug() << "get make and model from ppd-product";
        QString strProduct = list.value("ppd-product");
        QString prodMake, prodModel;
        if (!strProduct.isEmpty() && strProduct.startsWith("(") && strProduct.endsWith(")")) {
            QString lmake = normalize(strMake);
            strProduct = strProduct.mid(1, strProduct.length() - 2);
            if (!strProduct.toLower().startsWith(lmake))
                strProduct = strMake + " " + strProduct;

            ppdMakeModelSplit(strProduct, prodMake, prodModel);
            //qDebug() << QString("split Product make and model: %1 , %2").arg(prodMake).arg(prodModel);

            if (strModel.isEmpty())
                strModel = prodModel;
            if (strMake.isEmpty())
                strMake = prodMake;
        }
    }
}

QByteArray encryptString(const QString &input)
{
    QByteArray enArry = input.toUtf8();
    for (int i = 0; i < enArry.count(); ++i) {
        enArry[i] = enArry[i] ^ 0x73;
    }
    return enArry;
}

QString decryptString(const QByteArray &input)
{
    QByteArray deArry(input.count(), 0);
    for (int i = 0; i < input.count(); ++i) {
        deArry[i] = input[i] ^ 0x73;
    }
    return QString::fromUtf8(deArry);
}

static int checkDriversInfo(const QString &make, const QString &model, const QString &driverMakeModel,
                            const QString &ppdname, const QString &debver, const QString &debfile, const QSqlDatabase &ppddb)
{
    QSqlQuery query(ppddb);
    QString queryStr = "SELECT * FROM drivers_info WHERE driver_make = ? AND driver_model = ?";

    if (!query.prepare(queryStr)) {
        qCWarning(COMMONMOUDLE) << "Failed to prepare query:" << query.lastError().text();
        return -1;
    }

    // 绑定参数
    query.bindValue(0, make); // 第一个问号（?）绑定到make
    query.bindValue(1, model); // 第二个问号（?）绑定到model

    // 执行查询
    if (!query.exec()) {
        qCWarning(COMMONMOUDLE) << "Query failed:" << query.lastError().text();
        return -2;
    }

    // 遍历查询结果
    while (query.next()) {
        QString make_model = decryptString(query.value("driver_make_model").toByteArray());
        QString ppd_name = decryptString(query.value("ppd_name").toByteArray());
        QString deb_file = decryptString(query.value("deb_file").toByteArray());
        QString deb_ver = decryptString(query.value("deb_ver").toByteArray());

    //    qCWarning(COMMONMOUDLE) << query.value("driver_make_model");
    //    qCWarning(COMMONMOUDLE) << query.value("ppd_name");
    //    qCWarning(COMMONMOUDLE) << query.value("deb_file");
    //    qCWarning(COMMONMOUDLE) << query.value("deb_ver");

        if (ppd_name == ppdname && debfile == deb_file && deb_ver == debver && make_model == driverMakeModel) return 0;
    }

    // 如果没有找到任何匹配项，输出一条消息
    if (!query.first()) {
        qCWarning(COMMONMOUDLE) << "No drivers found with make:" << make << "and model:" << model;
        return -3;
    }

    return -4;
}

static int save_ppd_to_db(QString &debInfoFile, const QString &packname, QSqlDatabase &ppddb)
{
    QFile ppdinfo(debInfoFile);
    if (!ppdinfo.open(QIODevice::ReadOnly)) return -1;

    QString debMd5 = ppdinfo.readLine(1024);
    debMd5 = debMd5.trimmed();

    if(!debMd5.contains(packname)) {
        qCWarning(COMMONMOUDLE) << debInfoFile << "deb info not contains " << packname;
        return -2;
    }

    QStringList debinfo = debMd5.split(" ");
    if (debinfo.count() != 3) {
        qCWarning(COMMONMOUDLE) << debInfoFile << "deb info not right " << packname;
        return -3;
    }

    QString debVer = debinfo[1];
    // QString debFile = debinfo[2];
    QString debFile = packname;

    while(true) {
        QString ppdName = ppdinfo.readLine(1024);
        ppdName = ppdName.trimmed();
        if (ppdName.isEmpty()) break;

        QString ppdkey = ppdName.toLower();
        if (!g_ppds.contains(ppdkey)) {
            qCWarning(COMMONMOUDLE) << ppdName << "not install";
            return -7;
        }

        /* 保存ppd信息到数据库中
         * 数据库表头  driver_make  driver_model  ppd_name  deb_file deb_ver db_ver
         */
        QMap<QString, QString> ppdinfodict = g_ppds.value(ppdkey);
        // 插入一条记录
        QString driverMake = normalize(ppdinfodict[CUPS_PPD_MAKE]);
        QString driverModel = normalize(ppdinfodict[CUPS_PPD_MODEL]);
        QString driverMakeModel = ppdinfodict[CUPS_PPD_MAKE_MODEL];

        // driverModel为空只会出现在重复ppd没有放到查询字典中，没有初始化model信息
        if (driverModel.isEmpty()) continue;

        QSqlQuery query(ppddb);
        if (!query.prepare("INSERT INTO drivers_info (driver_make, driver_model, driver_make_model, ppd_name, deb_file, deb_ver, db_ver) "
                           "VALUES (:driver_make, :driver_model, :driver_make_model, :ppd_name, :deb_file, :deb_ver, :db_ver)")) {
            qCWarning(COMMONMOUDLE)  << "准备插入语句失败：" << query.lastError().text();
            return -4;
        } else {
            query.bindValue(":driver_make", driverMake);
            query.bindValue(":driver_model", driverModel);
            query.bindValue(":driver_make_model", encryptString(driverMakeModel));
            query.bindValue(":ppd_name", encryptString(ppdName));
            query.bindValue(":deb_file", encryptString(debFile));
            query.bindValue(":deb_ver", encryptString(debVer));
            query.bindValue(":db_ver", g_dbversion);

            if (!query.exec()) {
                qCWarning(COMMONMOUDLE) << "插入数据失败：" << query.lastError().text();
                return -5;
            }
        }

        if (0 != checkDriversInfo(driverMake, driverModel, driverMakeModel, ppdName, debVer, debFile, ppddb)) {
            qCWarning(COMMONMOUDLE) << "数据检查失败:" << driverMake << driverModel << ppdName << debVer << debFile;
            return -6;
        }
    }

    return 0;
}

int RefreshLocalPPDS::save_driver_info(const QString &debInfoDir)
{
    QSqlDatabase ppddb = QSqlDatabase::addDatabase("QSQLITE");
    QStringList supportArchs;

    supportArchs << "amd64" << "arm64" << "loongarch64";

    foreach (QString strArch, supportArchs) {
        QString archDirName = debInfoDir+"/"+strArch;
        QDir archDir(archDirName);
        if (!archDir.exists()) return -1;

        ppddb.setDatabaseName(archDirName+"/"+g_ppddbname);
        if (!ppddb.open()) {
            qCWarning(COMMONMOUDLE) << archDirName+"/"+g_ppddbname << ppddb.lastError().text();
            return -3;
        }

        QSqlQuery query(ppddb);
        if (!query.exec("CREATE TABLE IF NOT EXISTS drivers_info ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "driver_make TEXT NOT NULL,"
                         "driver_model TEXT NOT NULL,"
                         "driver_make_model TEXT NOT NULL,"
                         "ppd_name BLOB NOT NULL,"
                         "deb_file BLOB NOT NULL,"
                         "deb_ver BLOB NOT NULL,"
                         "db_ver TEXT NOT NULL)")) {
            qCWarning(COMMONMOUDLE) << "create drivers_info failed：" << query.lastError().text();
            ppddb.close();
            return -4;
        }

        QStringList allPackNames = archDir.entryList(QDir::Files);
        foreach (QString packname, allPackNames) {
            if (packname == g_ppddbname) continue;

            QString debInfoFile = debInfoDir+"/"+strArch+"/"+packname;
            if (0 != save_ppd_to_db(debInfoFile, packname, ppddb) ) {
                ppddb.close();
                return -2;
            }
        }

        (void)ppddb.commit();
        ppddb.close();
    }
    return 0;
}

int RefreshLocalPPDS::doWork()
{
    map<string, map<string, string>> allPPDS;
    map<string, map<string, string>>::iterator itall;

    qCDebug(COMMONMOUDLE) << QString("Starting...");

    QMutexLocker locker(&g_mutex);
    g_iStatus = TStat_Running;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            allPPDS = conPtr->getPPDs2(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, -1, nullptr, nullptr, nullptr);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    };

    if (m_bQuit)
        return 0;

    int count = 0;
    qCInfo(COMMONMOUDLE) << "format ppd info";
    for (itall = allPPDS.begin(); itall != allPPDS.end(); itall++) {
        qCDebug(COMMONMOUDLE) << QString("*****************************");
        QMap<QString, QString> list;
        map<string, string> mapValue = itall->second;
        map<string, string>::iterator itinfo = mapValue.begin();

        if (m_bQuit)
            return 0;

        QString ppdname = STQ(itall->first);
        qCDebug(COMMONMOUDLE) << CUPS_PPD_NAME << ":" << ppdname;
        list.insert(CUPS_PPD_NAME, ppdname);
        for (; itinfo != mapValue.end(); itinfo++) {
            QString attrName = STQ(itinfo->first);
            QString attrValue = attrValueToQString(itinfo->second);
            qCDebug(COMMONMOUDLE) << attrName << ":" << attrValue;
            list.insert(attrName, attrValue);
        }

        if (!ppdname.isEmpty()) {
            QString key = ppdname.toLower();
            QString strMake, strModel;
            getPpdMakeModel(strMake, strModel, list);

            count++;
            //make sure has make and model name
            if (strModel.isEmpty() || strMake.isEmpty() || key.isEmpty()) {
                qCWarning(COMMONMOUDLE) << QString("invaild ppd %1").arg(ppdname);
                continue;
            } else if (addToDirct(strMake, strModel, key)) {
                strMake = toNormalName(strMake);
                strModel = toNormalName(strModel);
                list.insert(CUPS_PPD_MODEL, strModel);
                list.insert(CUPS_PPD_MAKE, strMake);

                QMap<QString, QString> *modelMap = g_ppdsMakeModelNames.value(strMake);
                if (!modelMap) {
                    modelMap = new QMap<QString, QString>();
                    g_ppdsMakeModelNames.insert(strMake, modelMap);
                }
                modelMap->insertMulti(strModel, key);
                ;

                if (g_textPPd.isEmpty() && (ppdname.endsWith("textonly.ppd") || ppdname.endsWith("postscript.ppd")))
                    g_textPPd = list;
            }

            g_ppds.insertMulti(key, list);
        }
        qCDebug(COMMONMOUDLE) << QString("*****************************");
    }

    qCInfo(COMMONMOUDLE) << QString("Got ppds count: %1").arg(count);
    g_iStatus = TStat_Suc;
    return 0;
}

DriverSearcher::DriverSearcher(const TDeviceInfo &printer, QObject *parent)
    : QObject(parent)
    , m_matchLocalDriver(true)
{
    m_printer = printer;
    m_localIndex = 0;

    //通过device id中的MFG、MDL、CMD字段匹配
    if (!m_printer.strDeviceId.isEmpty()) {
        QMap<QString, QString> id_dirct;
        id_dirct = parseDeviceID(m_printer.strDeviceId);
        m_strCMD = id_dirct["CMD"];
        m_strMake = id_dirct["MFG"];
        m_strModel = id_dirct["MDL"];
        ppdMakeModelSplit(m_strMake + " " + m_strModel, m_strMake, m_strModel);
    } else if (!m_printer.strMakeAndModel.isEmpty()) {
        //如果没有device id,尝试通过make_and_model解析出来的make和model匹配
        ppdMakeModelSplit(m_printer.strMakeAndModel, m_strMake, m_strModel);
    }

    qCDebug(COMMONMOUDLE) << QString("Find driver for %1, %2, %3").arg(m_printer.uriList[0]).arg(m_printer.strMakeAndModel).arg(m_printer.strDeviceId);
}

void DriverSearcher::startSearch()
{
    QString strMake, strModel;

    /* 1. 匹配EveryWhere driver驱动, 查询到需要继续查询其他可用驱动 */
    QMap<QString, QVariant> driver = g_driverManager->getEveryWhereDriver(m_printer.uriList[0]);
    if (!driver.isEmpty()) {
        m_drivers.append(driver);
        qCInfo(COMMONMOUDLE) << "Got EveryWhere driver";
    }

    /* 等待服务器查找结果返回之后再开始查找本地驱动
     * 如果服务有精确查找到驱动，再执行一次本地精确查找
    */

    PrinterServerInterface *search = g_printerServer->searchDriverSolution(m_strMake, m_strModel, m_printer.strDeviceId);
    if (search) {
        connect(search, &PrinterServerInterface::signalDone, this, &DriverSearcher::slotDriverDone);
        search->getFromServer();
    } else {
        askForFinish();
    }
}

QList<QMap<QString, QVariant>> DriverSearcher::getDrivers()
{
    return m_drivers;
}

TDeviceInfo DriverSearcher::getPrinter()
{
    return m_printer;
}

void DriverSearcher::setMatchLocalDriver(bool match)
{
    m_matchLocalDriver = match;
}

static void insertDriver(QList<QMap<QString, QVariant>> &drivers, QList<QMap<QString, QVariant>> &mdrivers)
{
    QList<QMap<QString, QVariant>> net, local, open, migration;

    for (int i = 0; i < mdrivers.count(); ++i) { // 可用驱动类型
        auto item = mdrivers[i];
        if (item.contains(SD_KEY_recommended)) {
            net.append(item);
        } else if (item[SD_KEY_from].toInt() != PPDFrom_EveryWhere) {
            bool isMigration = item[CUPS_PPD_NAME].toString().contains("printer-driver-deepinwine");
            bool isOpen = item[CUPS_PPD_MAKE_MODEL].toString().contains("cups");
            if (!isMigration && !isOpen) {
                local.append(item);
            } else if (isOpen) {
                open.append(item);
            } else {
                migration.append(item);
            }
        }
    }

    drivers.append(net);
    if (mdrivers[0][SD_KEY_from].toInt() == PPDFrom_EveryWhere) {
        drivers.append(mdrivers[0]);
    }
    drivers.append(local);
    drivers.append(open);
    drivers.append(migration);
}

void DriverSearcher::sortDrivers()
{
    /* 驱动以本地集成优先，网络为次。适配推荐顺序为原生大于开源大于毕昇 */
    if (m_drivers.isEmpty())
        return;

    QList<QMap<QString, QVariant>> drivers;

    if (m_localIndex < 0 || m_localIndex > m_drivers.count()) {
        return;
    }

    // driverless设备限制使用driverless驱动，非driverless设备不使用driverless驱动
    for (int i = 0; i < m_drivers.count(); ++i) {
        bool isDriverlessDriver = m_drivers[i]["ppd-name"].toString().contains("driverless");
        bool isDriverlessDevice = m_printer.strInfo.contains("driverless");
        if ((isDriverlessDevice && !isDriverlessDriver) ||
            (!isDriverlessDevice && isDriverlessDriver)) {
                m_drivers.removeAt(i);
                --i;
        }
    }

    /* 先从本地查找的驱动开始遍历，保证本地驱动先插入列表中
     * 然后遍历从服务获取的驱动，如果服务器获取的是精确查找的结果，则可以插入到本地驱动之前
     * 如果服务器获取的不是精确查找的结果，则插入到本地驱动之后
     * 如果和本地驱动重复，则删除服务器查找的结果
    */
    if (m_drivers.count() > 1) {
        insertDriver(drivers, m_drivers);
        m_drivers = drivers;
    }
}

bool DriverSearcher::hasExcatDriver()
{
    foreach (auto driver, m_drivers) {
        if (driver[SD_KEY_excat].toBool()) {
            return true;
        }
    }

    return false;
}

void DriverSearcher::askForFinish()
{
    /* 首次尝试通过本地和服务器查找驱动之后，可能本地驱动还没有初始化完成。
     * 如果首次没有查找到精确匹配的驱动，等待本地驱动初始化完成之后再执行一次本地精确查找
     * usb即插即用时，网络驱动优先；如果没有网络驱动进行本地查找
    */
    QStringList supportArchs;
    supportArchs << "amd64" << "arm64" << "loongarch64";

    if (m_drivers.isEmpty() && !m_matchLocalDriver) { // usb需要初始化驱动信息，有网络驱动不进行本地初始化
        qCDebug(COMMONMOUDLE) << "usb init local ppds";
        g_driverManager->refreshPpds();
    }

    if ((m_matchLocalDriver && g_iStatus < TStat_Suc) ||
       (m_drivers.isEmpty() && !m_matchLocalDriver && g_iStatus < TStat_Suc)){
        qCInfo(COMMONMOUDLE) << "Wait ppd init";
        connect(g_driverManager, &DriverManager::signalStatus, this, &DriverSearcher::slotDriverInit, Qt::UniqueConnection);
        return;
    }

    // 驱动初始化完成，再执行一次本地精确查找；usb时，如果网络找到驱动，不进行本地查找
    if (m_drivers.isEmpty() || m_matchLocalDriver) {
        getLocalDrivers();
    }

    if (m_drivers.isEmpty() && supportArchs.contains(g_Settings->getSystemArch()) && isLastoreConfigVerAccord()) {
        getLocalDbDrivers();
    }

    if (TStat_Suc == g_iStatus && !hasExcatDriver() && m_matchLocalDriver &&
        (!m_strMake.isEmpty() || !m_strModel.isEmpty())) { // 模糊匹配
        QString strMake, strModel;
        QMutexLocker locker(&g_mutex);

        strMake = normalize(m_strMake);
        strModel = normalize(m_strModel);
        QList<QMap<QString, QVariant>> list = getFuzzyMatchDrivers(strMake, strModel, m_strCMD);
        if (!list.isEmpty()) {
            m_localIndex = m_drivers.count();
            m_drivers += list;
        }
    }

    sortDrivers();
    emit signalDone();
}

void DriverSearcher::parseJsonInfo(QJsonArray value)
{
    QList<QMap<QString, QVariant>> ppds;
    QStringList ppdNames;
    for (int i = 0; i < value.size(); ++i) {
        QJsonObject ppdobject = value[i].toObject();
        QJsonObject ppdsobject = ppdobject.value("ppds")[0].toObject();
        QMap<QString, QVariant> ppd;
        ppd.insert(SD_KEY_from, PPDFrom_Server);
        ppd.insert(CUPS_PPD_MAKE_MODEL, ppdsobject.value("desc").toString());
        ppd.insert(SD_KEY_driver, ppdobject.value("packages"));
        ppd.insert(SD_KEY_excat, QJsonValue(true));
        ppd.insert(CUPS_PPD_NAME, QJsonValue::fromVariant(ppdsobject.value("source").toString()));
        ppd.insert(SD_KEY_debver, ppdobject.value(SD_KEY_debver).toString());
        ppd.insert(SD_KEY_recommended,  ppdobject.value(SD_KEY_recommended).toBool());

        if (ppd[CUPS_PPD_MAKE_MODEL].toString().isEmpty() || ppd[SD_KEY_driver].toString().isEmpty() ||
            ppd[CUPS_PPD_NAME].toString().isEmpty() || ppd[SD_KEY_recommended].toString().isEmpty()) {
            qCDebug(COMMONMOUDLE) << "empty item";
            continue;
        }

        QString ppdName = ppd[SD_KEY_driver].toString();
        if (ppds.empty()) {
           ppds.append(ppd);
           ppdNames.append(ppdName);
           continue;
        }

        foreach (const auto &ppdin, ppds) { // 选取高版本，保留多个不同名的包
            if (ppdin[SD_KEY_driver].toString() == ppdName) {
                if (ppdin[SD_KEY_debver].toString() >= ppd[SD_KEY_debver].toString()) {
                    continue;
                }
                ppds.removeOne(ppdin);
                ppdNames.removeOne(ppdName);
            }

            if (!ppdNames.contains(ppdName)) {
                ppdNames.append(ppdName);
                ppds.append(ppd);
            }
        }
    }

    foreach (const auto &ppd, ppds) {
        m_drivers.append(ppd);
    }
}

void DriverSearcher::WriteSearchLog(int iCode, const QString &reply)
{
    QJsonObject obj;
    if (!isEventSdkInit()) {
        loadEventlib();
    }

    pfWriteEventLog WriteEventLog = getWriteEventLog();
    if (WriteEventLog) {
        obj.insert("tid", 1000100002);
        obj.insert("mfg", QJsonValue(m_strMake));
        obj.insert("model", QJsonValue(m_strModel));
        obj.insert("1284id", QJsonValue(m_printer.strDeviceId));
        obj.insert("driverInfo", QJsonValue(reply));
        obj.insert("replyCode", iCode);
        QString platformInfo = QString(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        platformInfo.replace("\\", "");
        WriteEventLog(platformInfo.toStdString());
    }
}

void DriverSearcher::slotDriverDone(int iCode, const QByteArray &result)
{
    QString reply;
    if (QNetworkReply::NoError == iCode && !result.isNull()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(result, &err);
        QJsonObject rootObject = doc.object()["data"].toObject();
        QJsonArray array = rootObject.value("list").toArray();

        reply = doc.toJson(QJsonDocument::Compact).constData();

        if (array.isEmpty()) {
            WriteSearchLog(iCode, reply);
            qCInfo(COMMONMOUDLE) << "List is empty!";
            sender()->deleteLater();
            askForFinish();
            return;
        }

        parseJsonInfo(array);
    } else if (iCode == QNetworkReply::UnknownNetworkError || iCode == QNetworkReply::HostNotFoundError) { // 网络未连接
        m_isNetOffline = true;
    }

    WriteSearchLog(iCode, reply);
    sender()->deleteLater();
    qCInfo(COMMONMOUDLE) << "Got net driver count:" << m_drivers.count();
    askForFinish();
}

void genOfflineDriver()
{
    /* 0. 解析离线json文件 */
    QString filePath = "/usr/share/dde-printer/offline.json";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo() << QString("Fail to open offline file.");
        return;
    }

    QByteArray array = file.readAll();
    file.close();

    /* 1. 开始解析: 将架构相同、型号存放到g_offlineDriver中 */
    QJsonParseError jsonParseError;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(array, &jsonParseError));
    if (QJsonParseError::NoError != jsonParseError.error) {
        qInfo() << QString("JsonParseError: %1").arg(jsonParseError.errorString());
        return;
    }

    QJsonArray rootJsonArrays = jsonDocument.array();
    static QString arch = g_Settings->getSystemArch();
    for (auto iter = rootJsonArrays.constBegin(); iter != rootJsonArrays.constEnd(); ++iter) {
        QJsonObject branchObject = (*iter).toObject();
        if ((branchObject.value("arch") == arch || branchObject.value("arch") == "all") && // 依据架构和型号查询
            !branchObject.value("ppds").isNull()) {
            QJsonArray ppdArrays = branchObject.value("ppds").toArray();
            for (auto iterPpd = ppdArrays.constBegin(); iterPpd != ppdArrays.constEnd(); ++iterPpd) {
                QJsonObject ppdObject = (*iterPpd).toObject();
                g_offlineDriver.insert(ppdObject.value("desc").toString());
            }
        }
    }
}

bool DriverSearcher::searchOffineDriver(QString mfg, QString model)
{
    Q_UNUSED(mfg);
    if (g_offlineDriver.empty()) {
        genOfflineDriver();
    }

    for (auto iter = g_offlineDriver.begin(); iter != g_offlineDriver.end(); ++iter) {
        if (iter->contains(model)) {
            return true;
        }
        continue;
    }
    return false;
}

bool DriverSearcher::hasOfflineDriver()
{
    return m_isOfflineDriverExist;
}

void DriverSearcher::slotDriverInit(int id, int state)
{
    Q_UNUSED(id);

    if (state >= TStat_Suc) {
        askForFinish();
    }
}

int DriverSearcher::getLocalDrivers()
{
    QString strMake, strModel;

    if (g_iStatus < TStat_Suc) {
        qCWarning(COMMONMOUDLE) << "PPD not init";
        m_localIndex = -1;
        return -1;
    }

    if (m_strMake.isEmpty() || m_strModel.isEmpty()) {
        qCWarning(COMMONMOUDLE) << "printer info is invaild";
        return -2;
    }

    strMake = normalize(m_strMake);
    strModel = normalize(m_strModel);
    QMutexLocker locker(&g_mutex);

    if (g_ppdsDirct.isEmpty() || g_ppds.isEmpty()) {
        qCWarning(COMMONMOUDLE) << QString("PPD dirct is empty");
        return -3;
    }

    if (strMake == "hp" && strModel.contains("colorlaserjet") && !m_printer.strInfo.contains("driverless")) {
        strModel.replace("colorlaserjet", "color laserjet");
    }

    QList<QMap<QString, QVariant>> list = getExactMatchDrivers(strMake, strModel);
    if (!list.isEmpty()) {
        m_drivers += list;
        m_localIndex = m_drivers.count();
    }

    qCInfo(COMMONMOUDLE) << QString("Got %1 drivers").arg(list.count());

    return list.count();
}

int DriverSearcher::getLocalDbDrivers()
{
    qCDebug(COMMONMOUDLE) << "db start";
    QString strMake, strModel;
    QStringList ppdList;
    if (m_strMake.isEmpty() || m_strModel.isEmpty()) {
        qCWarning(COMMONMOUDLE) << "printer info is invaild";
        return -2;
    }

    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
    }
    db.setDatabaseName(g_dbpath + "/" + g_ppddbname);

    if (!db.open()) {
        qCDebug(COMMONMOUDLE) << "db open failed: " << db.lastError().text();
        return -6;
    }

    QSqlQuery query(db);
    QString tableName = "drivers_info";
    QString queryStr = QString("SELECT * FROM %1 WHERE driver_make LIKE :driver_make AND driver_model LIKE :driver_model").arg(tableName);
    (void)query.prepare(queryStr);

    strMake = normalize(m_strMake);
    strModel = normalize(m_strModel);
    if (strMake == "hp" && strModel.contains("colorlaserjet")) {
        strModel.replace("colorlaserjet", "color laserjet");
    }

    query.bindValue(":driver_make", strMake);
    query.bindValue(":driver_model", strModel);

    if (!query.exec()) {
        qCDebug(COMMONMOUDLE) << "db query: " << query.lastError().text();
        goto dbend;
    }

    /* 查找匹配规则
       1. 使用型号精确匹配，如果匹配到，结束；
       2. 如果未精确匹配，再次查找厂商信息，保存厂商下的型号信息；
       3. 进行模糊查找；
       4. 均未查到退出
     */
    while (query.next()) {
        QString make = query.value("driver_make").toByteArray();
        QString model = query.value("driver_model").toByteArray();
        QString make_model = decryptString(query.value("driver_make_model").toByteArray());
        QString ppd_name = decryptString(query.value("ppd_name").toByteArray());
        QString deb_file = decryptString(query.value("deb_file").toByteArray());
        QString deb_ver = decryptString(query.value("deb_ver").toByteArray());

        QMap<QString, QVariant> ppd;
        ppd.insert(SD_KEY_from, PPDFrom_Server);
        ppd.insert(CUPS_PPD_MAKE_MODEL, make_model);
        ppd.insert(SD_KEY_driver, deb_file);
        ppd.insert(SD_KEY_excat, QJsonValue(true));
        ppd.insert(CUPS_PPD_NAME, ppd_name);
        ppd.insert(SD_KEY_debver, deb_ver);

        if (!ppdList.contains(make_model)) {
            ppdList << make_model;
        } else {
            continue;
        }

        m_drivers.append(ppd);
        qCDebug(COMMONMOUDLE) << "make:" << make << "model:" << model  << "make_model:" << make_model << "ppd_name:" << ppd_name << "deb_file:" << deb_file << deb_ver ;
    }

    if (m_drivers.isEmpty()) {
        QStringList modelList;
        QString queryMakeStr = QString("SELECT * FROM %1 WHERE driver_make LIKE :driver_make").arg(tableName);
        (void)query.prepare(queryMakeStr);
        query.bindValue(":driver_make", strMake);
        if (!query.exec()) {
            qCDebug(COMMONMOUDLE) << "db query make: " << query.lastError().text();
            goto dbend;
        }

        while (query.next()) {
            modelList << query.value("driver_model").toByteArray();
        }
        if (modelList.isEmpty()) { // 如果为空返回
            goto dbend;
        }
        // 查询到模糊匹配的型号
        QStringList models = findBestMatchPPDs(modelList, strModel);
        qCDebug(COMMONMOUDLE) << "fuzzy models:" << models;
        if (models.isEmpty()) {
            goto dbend;
        }

        QString queryFuzzyStr = QString("SELECT * FROM %1 WHERE driver_make LIKE :driver_make AND driver_model IN ('%2')").arg(tableName).arg(models.join("', '"));
        (void)query.prepare(queryFuzzyStr);
        query.bindValue(":driver_make", strMake);
        if (!query.exec()) {
            qCDebug(COMMONMOUDLE) << "db query models: " << query.lastError().text();
            goto dbend;
        }

        while (query.next()) {
            QString make_model = decryptString(query.value("driver_make_model").toByteArray());
            QString ppd_name = decryptString(query.value("ppd_name").toByteArray());
            QString deb_file = decryptString(query.value("deb_file").toByteArray());
            QString deb_ver = decryptString(query.value("deb_ver").toByteArray());

            QMap<QString, QVariant> ppd;
            ppd.insert(SD_KEY_from, PPDFrom_Server);
            ppd.insert(CUPS_PPD_MAKE_MODEL, make_model);
            ppd.insert(SD_KEY_driver, deb_file);
            ppd.insert(CUPS_PPD_NAME, ppd_name);
            ppd.insert(SD_KEY_debver, deb_ver);

            if (!ppdList.contains(make_model)) {
                ppdList << make_model;
            } else {
                continue;
            }

            m_drivers.append(ppd);
            qCDebug(COMMONMOUDLE) << "make_model:" << make_model << "ppd_name:" << ppd_name << "deb_file:" << deb_file << deb_ver ;
        }
    }

dbend:

    db.close();
    QSqlDatabase::removeDatabase("QSQLITE");

    qCInfo(COMMONMOUDLE) << QString("Got db %1 drivers").arg(m_drivers.count());
    return 0;
}

DriverManager *DriverManager::getInstance()
{
    static DriverManager *ppdInstance = nullptr;
    if (ppdInstance == nullptr) {
        ppdInstance = new DriverManager;
    }
    return ppdInstance;
}

DriverManager::DriverManager(QObject *parent)
    : QObject(parent)
{
    m_refreshTask = nullptr;
}

int DriverManager::getStatus()
{
    return g_iStatus;
}

int DriverManager::stop()
{
    if (m_refreshTask) {
        m_refreshTask->stop();
        m_refreshTask->deleteLater();
        m_refreshTask = nullptr;
    }

    QMutexLocker locker(&g_mutex);
    g_iStatus = TStat_None;
    qCInfo(COMMONMOUDLE) << "Clear local driver dirct";

    QList<QMap<QString, QString> *> models = g_ppdsDirct.values();
    foreach (auto model, models) {
        if (model)
            delete model;
    }
    models = g_ppdsMakeModelNames.values();
    foreach (auto model, models) {
        if (model)
            delete model;
    }
    g_ppds.clear();
    g_ppdsDirct.clear();
    g_ppdsMakeModelNames.clear();
    g_textPPd.clear();
    g_offlineDriver.clear();
    return 0;
}

DriverSearcher *DriverManager::createSearcher(const TDeviceInfo &device)
{
    DriverSearcher *searcher = new DriverSearcher(device, this);
    return searcher;
}

int DriverManager::refreshPpds()
{
    QMutexLocker locker(&g_mutex);
    if (TStat_Running == g_iStatus || m_refreshTask)
        return 0;

    m_refreshTask = new RefreshLocalPPDS();
    connect(m_refreshTask, &RefreshLocalPPDS::signalStatus, this, &DriverManager::signalStatus);
    m_refreshTask->start();

    return 0;
}

QStringList DriverManager::getAllMakes()
{
    QStringList makes;

    if (TStat_Suc == g_iStatus) {
        makes = g_ppdsMakeModelNames.keys();
    }

    return makes;
}

const QMap<QString, QString> *DriverManager::getModelsByMake(const QString &strMake)
{
    if (TStat_Suc > g_iStatus) {
        qCWarning(COMMONMOUDLE) << "PPD is not inited";
        return nullptr;
    }

    return g_ppdsMakeModelNames.value(strMake);
}

const QMap<QString, QMap<QString, QString>> *DriverManager::getPPDs()
{
    if (TStat_Suc > g_iStatus) {
        qCWarning(COMMONMOUDLE) << "PPD is not inited";
        return nullptr;
    }

    return &g_ppds;
}

QMap<QString, QString> DriverManager::getTextPPD()
{
    if (TStat_Suc > g_iStatus)
        return QMap<QString, QString>();

    return g_textPPd;
}

QMap<QString, QVariant> DriverManager::getEveryWhereDriver(const QString &strUri)
{
    QMap<QString, QVariant> driver;

    /* LanguageEncoding in generated ppd file is always ISOLatin1,
     * but it may contains utf-8 encoded character, so we don't use
     * everywhere model unless this issuse solved.
     * CUPS issue #5362 */
    /*http equal ipp */
    if ((strUri.startsWith("dnssd://") && strUri.contains("/cups")) ||
            ((strUri.startsWith("ipp://") || strUri.startsWith("ipps") || strUri.startsWith("http") || strUri.startsWith("https")) && strUri.contains("/printers"))) {
        driver.insert(SD_KEY_from, PPDFrom_EveryWhere);
        driver.insert(CUPS_PPD_MAKE_MODEL, tr("EveryWhere driver"));
        driver.insert(CUPS_PPD_NAME, "EveryWhere driver");
        driver.insert(SD_KEY_excat, true);
        qCDebug(COMMONMOUDLE) << "Got everywhere driver for" << strUri;
    }

    return driver;
}

QStringList DriverManager::getDriverDepends(const char *strPPD)
{
    PPD p;
    std::vector<Attribute> attrs;
    QStringList depends;

    try {
        if (QFile::exists(strPPD)) {
            p.load(strPPD);
        } else {
            auto conPtr = CupsConnectionFactory::createConnectionBySettings();
            if (!conPtr)
                return depends;
            string ppdfile = conPtr->getServerPPD(strPPD);

            qCDebug(COMMONMOUDLE) << strPPD << STQ(ppdfile);
            p.load(ppdfile.c_str());
        }
        attrs = p.getAttributes();
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return depends;
    };

    for (size_t i = 0; i < attrs.size(); i++) {
        QString strName = STQ(attrs[i].getName());
        QString strValue = STQ(attrs[i].getValue());
        qCDebug(COMMONMOUDLE) << strName << strValue;
        if (strName == "NickName") {
            QString strMake, strModel;
            ppdMakeModelSplit(strValue, strMake, strModel);
            if (strMake.toLower() == "hp" && strValue.contains("requires proprietary plugin")) {
                QString arch = g_Settings->getSystemArch();
                QRegExp regex("(\\d+\\.\\d+\\.\\d+)");
                if (regex.indexIn(strValue) == -1) {
                    break;
                }

                QVersionNumber curVer = QVersionNumber::fromString(regex.cap(1));
                QVersionNumber baseVersion = QVersionNumber::fromString("3.18.12");
                if (curVer <= baseVersion && !(arch.contains("mips") || arch.contains("loongarch"))) {
                    depends << "hplip-plugin";
                }
            }
            break;
        }
    }

    qCInfo(COMMONMOUDLE) << strPPD << depends;
    return depends;
}

bool DriverManager::isSamePPD(const QString &ppd1, const QString &ppd2)
{
    if (ppd1.isEmpty() || ppd2.isEmpty())
        return false;
    QString ppd1l = ppd1.toLower();
    QString ppd2l = ppd2.toLower();
    if (ppd1l.endsWith(ppd2l) || ppd2l.endsWith(ppd1l))
        return true;

    return false;
}
