// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "config.h"
#include "qtconvert.h"
#include "cupsattrnames.h"
#include "cupsconnectionfactory.h"

#include <QVariant>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QProcess>
#include <QDebug>
#include <QUrl>
#include <QStringList>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRegExpValidator>
#include <QDebug>
#include <QDateTime>
#include <QLibrary>

#include <netdb.h>

static bool isSdkInit = false;
typedef bool (*pfInitialize)(const std::string&, bool);
static pfInitialize InitializeSdk = nullptr;
static pfWriteEventLog WriteEventLog = nullptr;
const QString LibEventLog = "/usr/lib/libdeepin-event-log.so";
static QLibrary *eventLib = nullptr;
static QStringList timeList;

Q_LOGGING_CATEGORY(COMMONMOUDLE, "org.deepin.dde-printer.common")

void loadEventlib()
{
    if (!isSdkInit) {
        eventLib = new QLibrary(LibEventLog);
        if (eventLib == nullptr) {
            qCWarning(COMMONMOUDLE) << "no mem";
            return;
        }

        eventLib->load();
        if (!eventLib->isLoaded()) {
            delete eventLib;
            eventLib = nullptr;
            qCWarning(COMMONMOUDLE) << "Load libdeepin-event-log.so failed!";
            return;
        }

        InitializeSdk = (pfInitialize)(eventLib->resolve("Initialize"));
        WriteEventLog = (pfWriteEventLog)eventLib->resolve("WriteEventLog");
        if (InitializeSdk && InitializeSdk(APPNAME, true)) {
            qCDebug(COMMONMOUDLE) << "sdk load success" ;
            isSdkInit = true; // sdk初始化状态
        }
    }
}

bool isEventSdkInit()
{
    return isSdkInit;
}

void unloadEventLib()
{
    if (eventLib != nullptr && eventLib->isLoaded()) {
        eventLib->unload();
        delete eventLib;
        eventLib = nullptr;
    }

    if (isSdkInit) {
        isSdkInit = false;
        InitializeSdk = nullptr;
        WriteEventLog = nullptr;
    }
}

pfWriteEventLog getWriteEventLog()
{
    return WriteEventLog;
}

QStringList getCurrentTime(time_record_t type)
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString time = current_date_time.toString("yyyy-MM-dd hh:mm:ss");
    if (type == RESET_TIME) {
        timeList.clear();
    } else {
        if (type == ADD_TIME) {
            timeList.clear();
        }
        timeList << time;
    }
    return timeList;
}

QString getPrinterPPD(const char *name)
{
    QString strPPD;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            strPPD = STQ(conPtr->getPPD(name));
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return QString();
    };

    return strPPD;
}

QString getPrinterNameFromUri(const QString &uri)
{
    if (uri.split("/", QString::SkipEmptyParts).count() < 2)
        return QString();

    QByteArray bytes = QByteArray::fromPercentEncoding(uri.toUtf8());
    QString strurl = QString::fromUtf8(bytes);
    if (strurl.startsWith("dnssd://")) {
        QString strInfo = strurl.split("/", QString::SkipEmptyParts).at(1);
        return strInfo.split("@").first().trimmed();
    }

    return strurl.split("/").last();
}

QString getPrinterUri(const char *name)
{
    vector<string> requestList;
    map<string, string> attrs;
    QString strUri;

    try {
        requestList.push_back(CUPS_DEV_URI);
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            attrs = conPtr->getPrinterAttributes(name, nullptr, &requestList);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return QString();
    };

    strUri = attrValueToQString(attrs[CUPS_DEV_URI]);

    return strUri;
}

QString getHostFromUri(const QString &strUri)
{
    QUrl url(strUri);

    //fix for hp:/net/HP_Color_LaserJet_Pro_M252dw?ip=10.0.12.6
    QRegularExpression re("ip=(((?:(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))))");
    QRegularExpressionMatch match = re.match(strUri);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    //排除格式不对的
    if (strUri.indexOf("://") == -1)
        return QString();

    //smb格式uri：smb://[username:password@][workgroup/]server/printer
    if (strUri.startsWith("smb://")) {
        QStringList strlist = strUri.split("/", QString::SkipEmptyParts);
        QString str = strlist.count() > 3 ? strlist[strlist.count() - 2] : "";

        if (str.isEmpty())
            return QString();

        strlist = str.split("@");
        return strlist.last();
    }

    //dnssd格式uri: dnssd://printername @ host.*.*.local/*
    if (strUri.startsWith("dnssd://")) {
        QStringList strlist = QUrl::fromPercentEncoding(strUri.toUtf8()).split("/", QString::SkipEmptyParts);
        if (strlist.count() < 2)
            return QString();

        strlist = strlist[1].split(" ", QString::SkipEmptyParts);
        if (strlist.isEmpty())
            return QString();

        strlist = strlist.last().split(".", QString::SkipEmptyParts);
        if (strlist.count() < 2)
            return QString();

        return strlist.first() + "." + strlist.last();
    }

    //排除直连设备和文件设备
    if (strUri.startsWith("hp") || strUri.startsWith("usb") || strUri.startsWith("file"))
        return QString();

    return url.host();
}

QString reslovedHost(const QString &strHost)
{
    if (strHost.isEmpty())
        return strHost;

    struct hostent *host = gethostbyname(strHost.toUtf8().data());
    if (nullptr == host && HOST_NOT_FOUND == h_errno) {
        return QObject::tr("%1 not found, please ask the administrator for help, or enter the printer IP address in URI of Properties.").arg(strHost);
    }

    return QString();
}

bool isPackageExists(const QString &package)
{
    QDBusInterface newinterface
    {
        "org.deepin.dde.Lastore1",
        "/org/deepin/dde/Lastore1",
        "org.deepin.dde.Lastore1.Manager",
        QDBusConnection::systemBus()
    };

    QDBusReply<bool> isExists;
    if (newinterface.isValid()) {
        isExists = newinterface.call("PackageExists", package);
    } else {
        QDBusInterface interface
        {
            "com.deepin.lastore",
            "/com/deepin/lastore",
            "com.deepin.lastore.Manager",
            QDBusConnection::systemBus()
        };

        isExists = interface.call("PackageExists", package);
    }
    return isExists.isValid() && isExists;
}

QVariant ipp_attribute_value(ipp_attribute_t *attr, int i)
{
    QVariant val;
    //    char unknown[100];
    //    int lower, upper;
    //    int xres, yres;
    //    ipp_res_t units;

    switch (ippGetValueTag(attr)) {
    case IPP_TAG_NAME:
    case IPP_TAG_TEXT:
    case IPP_TAG_KEYWORD:
    case IPP_TAG_URI:
    case IPP_TAG_CHARSET:
    case IPP_TAG_MIMETYPE:
    case IPP_TAG_LANGUAGE:
        qCDebug(COMMONMOUDLE) << QString("Got %1 : %2").arg(UTF8_T_S(ippGetName(attr))).arg(UTF8_T_S(ippGetString(attr, i, nullptr)));
        val = QVariant(UTF8_T_S(ippGetString(attr, i, nullptr)).trimmed());
        break;
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        qCDebug(COMMONMOUDLE) << QString("Got %1 : %2").arg(UTF8_T_S(ippGetName(attr))).arg(ippGetInteger(attr, i));
        val = QVariant(ippGetInteger(attr, i));
        break;
    case IPP_TAG_BOOLEAN:
        qCDebug(COMMONMOUDLE) << QString("Got %1 : %2").arg(UTF8_T_S(ippGetName(attr))).arg(ippGetBoolean(attr, i) ? "true" : "false");
        val = QVariant(ippGetBoolean(attr, i));
        break;
    //    case IPP_TAG_RANGE:
    //        lower = ippGetRange (attr, i, &upper);
    //        val = Py_BuildValue ("(ii)",
    //                 lower,
    //                 upper);
    //        break;
    //    case IPP_TAG_NOVALUE:
    //        Py_RETURN_NONE;
    //        break;
    //    // TODO:
    //    case IPP_TAG_DATE:
    //        val = PyUnicode_FromString ("(IPP_TAG_DATE)");
    //        break;
    //    case IPP_TAG_RESOLUTION:
    //        xres = ippGetResolution(attr, i, &yres, &units);
    //        val = Py_BuildValue ("(iii)",
    //                 xres,
    //                 yres,
    //                 units);
    //        break;
    default:
        //        snprintf (unknown, sizeof (unknown),
        //              "(unknown IPP value tag 0x%x)", ippGetValueTag(attr));
        //        val = PyUnicode_FromString (unknown);
        break;
    }

    return val;
}

int shellCmd(const QString &cmd, QString &out, QString &strErr, int timeout)
{
    QProcess proc;
    proc.start(cmd);
    if (proc.waitForFinished(timeout)) {
        out = proc.readAll();
        if (proc.exitCode() != 0 || proc.exitStatus() != QProcess::NormalExit) {
            strErr = QString("err %1, string: %2").arg(proc.exitCode()).arg(QString::fromUtf8(proc.readAllStandardError()));
            qCDebug(COMMONMOUDLE) << cmd;
            qCWarning(COMMONMOUDLE) << "shellCmd exit with err: " << strErr;
            return -1;
        }
    } else {
        qCWarning(COMMONMOUDLE) << "shellCmd timeout";
        return -2;
    }

    return 0;
}

//将每个单词统一成首字母大写
QString toNormalName(const QString &name)
{
    QString norName;
    QStringList list = name.split(" ");

    if (0 == name.compare("hp", Qt::CaseInsensitive))
        return "HP";
    else if (0 == name.compare("CIAAT", Qt::CaseInsensitive))
        return "CIAAT";
    else if (0 == name.compare("DEC", Qt::CaseInsensitive))
        return "DEC";
    else if (0 == name.compare("IBM", Qt::CaseInsensitive))
        return "IBM";
    else if (0 == name.compare("NEC", Qt::CaseInsensitive))
        return "NEC";
    else if (0 == name.compare("NRG", Qt::CaseInsensitive))
        return "NRG";
    else if (0 == name.compare("PCPI", Qt::CaseInsensitive))
        return "PCPI";
    else if (0 == name.compare("QMS", Qt::CaseInsensitive))
        return "QMS";

    foreach (QString str, list) {
        norName += str.left(1).toUpper();
        norName += str.right(str.length() - 1).toLower();
        norName += " ";
    }
    norName = norName.trimmed();
    return norName;
}

//通过空格分割字符串，在""内的空格不算
QStringList splitStdoutString(const QString &str)
{
    QStringList list;
    QString filed;
    bool quoting = false;

    for (auto it = str.begin(); it != str.end(); ++it) {
        if (*it == QChar(' ')) {
            if (!quoting) {
                list << filed;
                filed.clear();
            } else
                filed += *it;
        } else if (*it == QChar('"')) {
            quoting = !quoting;
        } else {
            if (*it == QChar('\\')) {
                auto next = it + 1;
                if (next != str.end() && (*next == QChar('\\') || *next == QChar('"')))
                    ++it;
            }
            filed += *it;
        }
    }

    if (!filed.isEmpty())
        list << filed;

    return list;
}

/*# This function normalizes manufacturer and model names for comparing.
# The string is turned to lower case and leading and trailing white
# space is removed. After that each sequence of non-alphanumeric
# characters (including white space) is replaced by a single space and
# also at each change between letters and numbers a single space is added.
# This makes the comparison only done by alphanumeric characters and the
# words formed from them. So mostly two strings which sound the same when
# you pronounce them are considered equal. Printer manufacturers do not
# market two models whose names sound the same but differ only by
# upper/lower case, spaces, dashes, ..., but in printer drivers names can
# be easily supplied with these details of the name written in the wrong
# way, especially if the IEEE-1284 device ID of the printer is not known.
# This way we get a very reliable matching of printer model names.
# Examples:
# - Epson PM-A820 -> epson pm a 820
# - Epson PM A820 -> epson pm a 820
# - HP PhotoSmart C 8100 -> hp photosmart c 8100
# - hp Photosmart C8100  -> hp photosmart c 8100*/
QString normalize(const QString &strin)
{
    QString normalized;
    QString lstrin = strin.trimmed().toLower();
    bool alnumfound = false;
    enum { BLANK,
           LETTER,
           DIGIT
         };
    int lastchar = BLANK;

    if (strin.isEmpty())
        return strin;

    foreach (QChar ch, lstrin) {
        if (ch.isLetter()) {
            if (LETTER != lastchar && alnumfound)
                normalized += " ";
            lastchar = LETTER;
        } else if (ch.isDigit()) {
            if (DIGIT != lastchar && alnumfound)
                normalized += " ";
            lastchar = DIGIT;
        } else
            lastchar = BLANK;

        if (ch.isLetterOrNumber()) {
            normalized += ch;
            alnumfound = true;
        }
    }

    return normalized;
}

QMap<QString, QString> parseDeviceID(const QString &strId)
{
    QMap<QString, QString> map;
    QStringList list = strId.split(";", QString::SkipEmptyParts);
    foreach (QString str, list) {
        QStringList val = str.split(":", QString::SkipEmptyParts);
        if (val.count() > 1)
            map.insert(val[0].trimmed(), val[1].trimmed());
    }

    if (!map.contains("MFG") && map.contains("MANUFACTURER"))
        map.insert("MFG", map["MANUFACTURER"]);
    if (!map.contains("MFG") && map.contains("G"))
        map.insert("MFG", map["G"]);
    if (!map.contains("MDL") && map.contains("MODEL"))
        map.insert("MDL", map["MODEL"]);
    if (!map.contains("CMD") && map.contains("COMMAND SET"))
        map.insert("CMD", map["COMMAND SET"]);

    return map;
}

static const char *g_replaceMap[][2] = {{"lexmark international", "Lexmark"},
    {"kyocera mita", "Kyocera"},
    {"hewlettpackard", "HP"},
    {"minoltaqms", "minolta"},
    {"datamaxoneil", "datamax"},
    {"eastman kodak company", "kodak"},
    {"fuji xerox", "fuji xerox"},
    {"konica minolta", "konica minolta"}
};
QString replaceMakeName(QString &make_and_model, int *len)
{
    make_and_model = make_and_model.replace(QRegularExpression("_|-|\'"), "");
    if (make_and_model.isEmpty())
        return make_and_model;

    QString strMM = make_and_model.toLower();

    int size = sizeof(g_replaceMap) / sizeof(g_replaceMap[0]);
    for (int i = 0; i < size; i++) {
        QString str = g_replaceMap[i][0];

        if (strMM.startsWith(str)) {
            if (len)
                *len = str.length();

            return g_replaceMap[i][1];
        }
    }

    return make_and_model.split(" ").first();
}

typedef struct tagMakeRegular {
    QString strMake;
    QString strRegular;
} TMakeRegular;

TMakeRegular _MFR_BY_RANGE[] = {
    // Fill in missing manufacturer names based on model name
    {
        "HP", QString("deskjet"
                      "|dj[ 0-9]?"
                      "|laserjet"
                      "|lj"
                      "|color laserjet"
                      "|color lj"
                      "|designjet"
                      "|officejet"
                      "|oj"
                      "|photosmart"
                      "|ps "
                      "|psc"
                      "|edgeline")
    },
    {"Epson", QString("stylus|aculaser")},
    {
        "Apple", QString("stylewriter"
                         "|imagewriter"
                         "|deskwriter"
                         "|laserwriter")
    },
    {
        "Canon", QString("pixus"
                         "|pixma"
                         "|selphy"
                         "|imagerunner"
                         "|bj"
                         "|lbp")
    },
    {"Brother", QString("hl|dcp|mfc")},
    {
        "Xerox", QString("docuprint"
                         "|docupage"
                         "|phaser"
                         "|workcentre"
                         "|homecentre")
    },
    {"Lexmark", QString("optra|(:color )?jetprinter")},
    {
        "KONICA MINOLTA", QString("magicolor"
                                  "|pageworks"
                                  "|pagepro")
    },
    {
        "Kyocera", QString("fs-"
                           "|km-"
                           "|taskalfa")
    },
    {"Ricoh", QString("aficio")},
    {"Oce", QString("varioprint")},
    {"Oki", QString("okipage|microline")}
};

QString _RE_ignore_suffix = QString(","
                                    "| hpijs"
                                    "| foomatic/"
                                    "| - "
                                    "| w/"
                                    "| postscript"
                                    "| ps"
                                    "| pdf"
                                    "| pxl"
                                    "| zjs"
                                    "| zxs"
                                    "| pcl3"
                                    "| printer"
                                    "|_bt"
                                    "| pcl"
                                    "| ufr ii"
                                    "| br-script"
                                    "| for cups"
                                    "| series"
                                    "| all-in-one");

typedef struct tagHPMode {
    QString strShortName;
    QString strFullName;
} THPMode;

THPMode _HP_MODEL_BY_NAME[] = {{"dj", "DeskJet"},
    {"lj", "LaserJet"},
    {"oj", "OfficeJet"},
    {"color lj", "Color LaserJet"},
    {"ps ", "PhotoSmart"}
};

void removeMakeInModel(const QString &strMake, QString &strModel)
{
    QString modell = strModel.toLower();
    QString makel = strMake.toLower();
    if (modell.startsWith(makel)) {
        strModel = strModel.right(strModel.length() - makel.length());
        strModel = strModel.trimmed();
    }
}

void formatModelName(const QString &strMake, QString &strModel)
{
    // 去掉自带的bisheng字段
    strModel = strModel.replace("(Bisheng)", "");

    QString makel = strMake.toLower();
    QString modell = strModel.toLower();
    /*# HP and Canon PostScript PPDs give NickNames like:
    # *NickName: "HP LaserJet 4 Plus v2013.111 Postscript (recommended)"
    # *NickName: "Canon MG4100 series Ver.3.90"
    # Find the version number and truncate at that point.  But beware,
    # other model names can legitimately look like version numbers,
    # e.g. Epson PX V500.
    # Truncate only if the version number has only one digit, or a dot
    # with digits before and after.*/
    if (modell.contains(" v")) {
        /*# Look for " v" or " ver." followed by a digit, optionally
        # followed by more digits, a dot, and more digits; and
        # terminated by a space of the end of the line.*/
        int index = modell.indexOf(QRegularExpression(" v(?:er\\.)?\\d(?:\\d*\\.\\d+)?(?: |$)"));
        if (index > 0) {
            strModel = strModel.left(index);
            modell = modell.left(index);
        }
    }

    //remove right string contains ignores suffix
    int index = modell.indexOf(QRegularExpression(_RE_ignore_suffix));
    if (index > 0) {
        strModel = strModel.left(index);
        modell = modell.left(index);
    }

    if (makel == "hp") {
        int len = sizeof(_HP_MODEL_BY_NAME) / sizeof(THPMode);
        for (int i = 0; i < len; i++) {
            if (modell.startsWith(_HP_MODEL_BY_NAME[i].strShortName)) {
                strModel = _HP_MODEL_BY_NAME[i].strFullName + strModel.right(strModel.length() - _HP_MODEL_BY_NAME[i].strShortName.length());
                modell = strModel.toLower();
                break;
            }
        }
    }

    removeMakeInModel(strMake, strModel);
    strModel = strModel.replace(QRegularExpression("_|-|\'"), " ");
    strModel = strModel.trimmed();
}

void ppdMakeModelSplit(const QString &strMakeAndModel, QString &strMake, QString &strModel)
{
    QString Make_And_Model = strMakeAndModel;
    int len = 0;
    strMake = replaceMakeName(Make_And_Model, &len);
    if (strModel.isEmpty()) {
        if (len <= 0) {
            len = strMake.length();
        }

        strModel = Make_And_Model.mid(len + 1);
    }

    formatModelName(strMake, strModel);
}

bool isIpv4Address(const QString &str)
{
    QRegularExpression reg("((25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))");
    return reg.match(str).hasMatch();
}

int managerCanonPrinter(const QString &action, const QStringList &args)
{
    QDBusInterface interface("com.deepin.printer.manager", "/com/deepin/printer/manager", "com.deepin.printer.manager", QDBusConnection::systemBus());
    QDBusReply<int> ret;
    if (action == "add") {
        ret = interface.call("CanonPrinterInstall", args);
    } else {
        ret = interface.call("CanonPrinterRemove", args);
    }

    if (ret.isValid()) {
        return ret;
    }

    return 0;
}

bool isPackageInstalled(const QString &packageName)
{
    QProcess process;
    QString cmd = QString("dpkg -l %1 | grep -E '^ii.*%1'").arg(packageName);
    process.start("/bin/bash", QStringList() << "-c" << cmd);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();

    return !output.isEmpty();
}
