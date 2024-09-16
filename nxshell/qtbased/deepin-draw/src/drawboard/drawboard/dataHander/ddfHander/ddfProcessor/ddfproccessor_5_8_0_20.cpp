// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_8_0_20.h"
#include "dialog.h"

#include <QFileInfo>
#include <QFile>

DdfUnitProccessor_5_8_0_20::DdfUnitProccessor_5_8_0_20(QObject *parent): DdfUnitProccessor_5_8_0_16_1(parent)
{

}

int DdfUnitProccessor_5_8_0_20::version()
{
    return EDdf5_8_0_20;
}

bool DdfUnitProccessor_5_8_0_20::saveTo(const PageContextData &data, const QString &file, DdfHander *hander)
{
    bool b = DdfUnitProccessor_5_8_0_16_1::saveTo(data, file, hander);

    saveMd5ToFile(file);

    return b;
}

PageContextData DdfUnitProccessor_5_8_0_20::loadFrom(const QString &file, DdfHander *hander)
{
    //this version should do md5 check
    if (checkMd5BeforeLoad(file)) {
        return DdfUnitProccessor_5_8_0::loadFrom(file, hander);
    } else {
        messageFileBroken(file, hander);
        return PageContextData();
    }
}

qint64 DdfUnitProccessor_5_8_0_20::calTotalBytes(const PageContextData &data, DdfHander *hander)
{
    return DdfUnitProccessor_5_8_0_16_1::calTotalBytes(data, hander) + 16;
}

void DdfUnitProccessor_5_8_0_20::saveMd5ToFile(const QString &filePath)
{
    qDebug() << "write Md5 To DdfFile begin, file = " << filePath;
    bool result = false;
    QByteArray srcBinArry;
    QFile f(filePath);
    if (f.open(QFile::ReadWrite)) {

        srcBinArry = f.readAll();

        QDataStream stream(&f);

        //加密得到md5值
        QByteArray md5 = QCryptographicHash::hash(srcBinArry, QCryptographicHash::Md5);

        stream.device()->seek(srcBinArry.size());

        //防止 << 操作符写入长度，所以用这个writeRawData函数 (只写入md5值不写md5的长度，因为md5是固定16个字节的)
        stream.writeRawData(md5.data(), md5.size());

        f.close();

        result = true;

        qDebug() << "ddfFile file contex bin size = " << srcBinArry.size() << "result md5 = " << md5.toHex().toUpper();
    }
    qDebug() << "write Md5 To DdfFile end, file = " << filePath << " result = " << result;
}

bool DdfUnitProccessor_5_8_0_20::checkMd5BeforeLoad(const QString &filePath)
{
    QFile file(filePath);
    if (file.exists()) {
        if (file.open(QFile::ReadOnly)) {
            QDataStream s(&file);

            QByteArray allBins = file.readAll();
            QByteArray md5    = allBins.right(16);

            qDebug() << "load  head+bytes = " << (allBins.count() - md5.count()) << "md5 count = " << md5.count();
            qDebug() << "direct read MD5 form ddffile file = " << filePath << " MD5 = " << md5.toHex().toUpper();

            QByteArray contex = allBins.left(allBins.size() - md5.size());

            QByteArray nMd5 = QCryptographicHash::hash(contex, QCryptographicHash::Md5);

            qDebug() << "recalculate MD5 form ddffile file = " << filePath << " MD5 = " << nMd5.toHex().toUpper();

            if (md5 != nMd5) {
                return false;
            }
            return true;
        }
    }
    return false;
}

void DdfUnitProccessor_5_8_0_20::messageFileBroken(const QString &file, DdfHander *hander)
{
    emit hander->message(tr("Unable to open the broken file \"%1\"").arg(QFileInfo(file).fileName()));
}

