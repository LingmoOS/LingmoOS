/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "ukccbluetoothconfig.h"

QGSettings* ukccbluetoothconfig::ukccGsetting = new QGSettings(GSETTING_SCHEMA_UKCC,GSETTING_PACH_UKCC);

ukccbluetoothconfig::ukccbluetoothconfig()
{

}

ukccbluetoothconfig::~ukccbluetoothconfig()
{
    ukccGsetting->deleteLater();
}


void ukccbluetoothconfig::launchBluetoothServiceStart(const QString &processName)
{
    KyDebug();
    QProcess *process = new QProcess();
    QString cmd = processName;
    //cmd.append(" -o");
    KyDebug() << cmd ;
    process->startDetached(cmd);
}

void ukccbluetoothconfig::killAppProcess(const quint64 &processId)
{
    KyDebug() << processId;
    QProcess *process = new QProcess();
    QString cmd = QString("kill -9 %1").arg(processId);
    process->startDetached(cmd);
}

bool ukccbluetoothconfig::checkProcessRunning(const QString &processName, QList<quint64> &listProcessId)
{
    KyDebug();

    bool res(false);
    QString strCommand;
    //if(processName == BluetoothServiceExePath)
    //    strCommand = "ps -ef|grep '" + processName + " -o' |grep -v grep |awk '{print $2}'";
    //else
        strCommand = "ps -ef|grep '" + processName + "' |grep -v grep |awk '{print $2}'";
    KyDebug()  << strCommand ;

    QByteArray ba = strCommand.toLatin1();
    const char* strFind_ComName = ba.data();
    //const char* strFind_ComName = convertQString2char(strCommand);
    FILE * pPipe = popen(strFind_ComName, "r");
    if (pPipe)
    {
        std::string com;
        char name[512] = { 0 };
        while (fgets(name, sizeof(name), pPipe) != NULL)
        {
            int nLen = strlen(name);
            if (nLen > 0
                && name[nLen - 1] == '\n')
                //&& name[0] == '/')
            {
                name[nLen - 1] = '\0';
                listProcessId.append(atoi(name));
                res = true;
                break;
            }
        }
        pclose(pPipe);
    }
    return res;
}

static QString executeLinuxCmd(QString strCmd)
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();
    return strResult.toLower();
}


void ukccbluetoothconfig::setEnvPCValue()
{
    KyDebug();
    unsigned int productFeatures = 0;
    char * projectName = "";
    char * projectSubName = "";
    productFeatures = kdk_system_get_productFeatures();
    projectName = kdk_system_get_projectName();
    QString qs_projectName = QString(projectName);
    projectSubName = kdk_system_get_projectSubName();
    QString qs_projectSubName = QString(projectSubName);

    envPC = Environment::NOMAL;
    QString str = executeLinuxCmd("cat /proc/cpuinfo | grep Hardware");
    if(str.length() == 0)
    {
        goto FuncEnd;
    }

    if(str.indexOf("huawei") != -1 || str.indexOf("pangu") != -1 ||
            str.indexOf("kirin") != -1)
    {
        envPC = Environment::HUAWEI;
    }

FuncEnd:
    if (QFile::exists("/etc/apt/ota_version"))
        envPC = Environment::LAIKA;
    else if(qs_projectName.contains("V10SP1-edu",Qt::CaseInsensitive) &&
            qs_projectSubName.contains("mavis",Qt::CaseInsensitive))
    {
        envPC = Environment::MAVIS;
    }

    KyInfo() << envPC ;
}

const QImage ukccbluetoothconfig::loadSvgImage(const QPixmap &source, const PixmapColor &cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                switch (cgColor) {
                case PixmapColor::WHITE:
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLACK:
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::GRAY:
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLUE:
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                    break;
                default:
                    return img;
                    break;
                }
            }
        }
    }
    return img;
}

const QPixmap ukccbluetoothconfig::loadSvg(const QPixmap &source, const PixmapColor &cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                switch (cgColor) {
                case PixmapColor::WHITE:
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLACK:
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::GRAY:
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLUE:
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                    break;
                default:
                    return source;
                    break;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

bool ukccbluetoothconfig::ukccBtBuriedSettings(QString pluginName, QString settingsName, QString action, QString value)
{
#ifdef UKCC_BLUETOOTH_DATA_BURIAL_POINT
    try
    {
        bool res = ukcc::UkccCommon::buriedSettings(pluginName,settingsName,action,value);
        return res;
    }
    catch(std::exception &e)
    {
        KyWarning() << e.what();
        return false;
    }
#endif
    return false;
}
