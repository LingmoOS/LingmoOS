// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileloadthread.h"
#include "utils.h"
#include "../encodes/detectcode.h"
#include <QFile>
#include <QDebug>

FileLoadThread::FileLoadThread(const QString &filepath, QObject *parent)
    : QThread(parent),
      m_strFilePath(filepath)
{

}

FileLoadThread::~FileLoadThread()
{
}

void FileLoadThread::run()
{
    QFile file(m_strFilePath);

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray encode;
        QByteArray indata;

        // 判断文件大小是否超过40MB, 超过40MB的文件过大，需要调整读取策略，优先加载头部文件
        static const int s_maxDirectReadLen = 40 * DATA_SIZE_1024 * DATA_SIZE_1024;
        if (file.size() > s_maxDirectReadLen) {
            // 先读取1MB数据
            indata = file.read(DATA_SIZE_1024 * DATA_SIZE_1024);
            encode = DetectCode::GetFileEncodingFormat(m_strFilePath, indata);

            // 发送文件头信息，用于预先加载数据
            QString textEncode = QString::fromLocal8Bit(encode);
            if (textEncode.contains("ASCII", Qt::CaseInsensitive) || textEncode.contains("UTF-8", Qt::CaseInsensitive)) {
                emit sigPreProcess(encode, indata);
            } else {
                QByteArray outHeadData;
                DetectCode::ChangeFileEncodingFormat(indata, outHeadData, textEncode, QString("UTF-8"));
                emit sigPreProcess(encode, outHeadData);
            }
        }

        // 读取申请开辟内存空间时，捕获可能出现的 std::bad_alloc() 异常，防止闪退。
        try {
            // reads all remaining data from the file.
            indata += file.read(file.size());
            file.close();
        } catch (const std::exception &e) {
            qWarning() << Q_FUNC_INFO << "Read file data error, " << QString(e.what());

            file.close();
            emit sigLoadFinished(encode, indata, true);
            return;
        }

        if (encode.isEmpty()) {
            //编码识别，如果文件数据大于1M，则只裁剪出1M文件数据去做编码探测
            QByteArray dateUsedForCodeIdentify;
            if (indata.length() > DATA_SIZE_1024 * DATA_SIZE_1024) {
                dateUsedForCodeIdentify = indata.mid(0, DATA_SIZE_1024 * DATA_SIZE_1024);
            } else {
                dateUsedForCodeIdentify = indata;
            }
            encode = DetectCode::GetFileEncodingFormat(m_strFilePath, dateUsedForCodeIdentify);
        }

        QString textEncode = QString::fromLocal8Bit(encode);
        if (textEncode.contains("ASCII", Qt::CaseInsensitive) || textEncode.contains("UTF-8", Qt::CaseInsensitive)) {
            emit sigLoadFinished(encode, indata, false);
        } else {
            QByteArray outData;
            DetectCode::ChangeFileEncodingFormat(indata, outData, textEncode, QString("UTF-8"));
            emit sigLoadFinished(encode, outData, false);
        }
    }

    this->quit();
    this->deleteLater();
}
