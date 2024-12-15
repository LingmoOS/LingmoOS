// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnotea2tmanager.h"

#include <DSysInfo>

#include <QDBusError>

DCORE_USE_NAMESPACE

/**
 * @brief VNoteA2TManager::VNoteA2TManager
 * @param parent
 */
VNoteA2TManager::VNoteA2TManager(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief VNoteA2TManager::initSession
 * @return 错误码
 */
int VNoteA2TManager::initSession()
{
    m_session.reset(new com::iflytek::aiservice::session(
        "com.iflytek.aiservice",
        "/",
        QDBusConnection::sessionBus()));

    const QString ability("asr");
    const QString appName = qApp->applicationName();
    int errorCode = -1;

    QDBusObjectPath qdPath = m_session->createSession(appName, ability, errorCode);

    m_asrInterface.reset(new com::iflytek::aiservice::asr(
        "com.iflytek.aiservice",
        qdPath.path(),
        QDBusConnection::sessionBus()));

    connect(m_asrInterface.get(), &com::iflytek::aiservice::asr::onNotify,
            this, &VNoteA2TManager::onNotify);

    return errorCode;
}

/**
 * @brief VNoteA2TManager::startAsr
 * @param filePath 文件路径
 * @param fileDuration 文件时长
 * @param srcLanguage 源语言
 * @param targetLanguage 目标语言
 */
void VNoteA2TManager::startAsr(QString filePath,
                               qint64 fileDuration,
                               QString srcLanguage,
                               QString targetLanguage)
{
    int ret = initSession();
    if (ret != 0) {
        emit asrError(AudioOther);
        qInfo() << "createSession->errorCode=" << ret;
        return;
    }

    QVariantMap param;

    param.insert("filePath", filePath);
    param.insert("fileDuration", fileDuration);

    if (!srcLanguage.isEmpty()) {
        param.insert("language", srcLanguage);
    }

    if (!targetLanguage.isEmpty()) {
        param.insert("targetLanguage", targetLanguage);
    }

    QString retStr = m_asrInterface->startAsr(param);

    if (retStr != CODE_SUCCESS) {
        asrMsg error;
        error.code = retStr;
        emit asrError(getErrorCode(error));
    }
}

/**
 * @brief VNoteA2TManager::stopAsr
 */
void VNoteA2TManager::stopAsr()
{
    m_asrInterface->stopAsr();
}

/**
 * @brief VNoteA2TManager::onNotify
 * @param msg 数据信息
 */
void VNoteA2TManager::onNotify(const QString &msg)
{
    asrMsg asrData;

    asrJsonParser(msg, asrData);

    qInfo() << "msg:" << msg;

    if (CODE_SUCCESS == asrData.code) {
        if (XF_finish == asrData.status) {
            //Finish convertion
            emit asrSuccess(asrData.text);
        } else if (XF_fail == asrData.status) {
            //Failed convertion
            emit asrError(getErrorCode(asrData));
        } else {
            //We ingore other state BCS we don't
            // care about it
        }
    } else {
        emit asrError(getErrorCode(asrData));
    }
}

/**
 * @brief VNoteA2TManager::asrJsonParser
 * @param msg json串
 * @param asrData 解析后的数据
 */
void VNoteA2TManager::asrJsonParser(const QString &msg, asrMsg &asrData)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);

    if (QJsonParseError::NoError == error.error) {
        QVariantMap map = doc.toVariant().toMap();

        if (map.contains("code")) {
            asrData.code = map["code"].toString();
        }

        if (map.contains("descInfo")) {
            asrData.descInfo = map["descInfo"].toString();
        }

        if (map.contains("failType")) {
            asrData.failType = map["failType"].toInt();
        }

        if (map.contains("status")) {
            asrData.status = map["status"].toInt();
        }

        if (map.contains("text")) {
            asrData.text = map["text"].toString();
        }
    }
}

/**
 * @brief VNoteA2TManager::getErrorCode
 * @param 消息数据
 * @return 错误码
 */
VNoteA2TManager::ErrorCode VNoteA2TManager::getErrorCode(const asrMsg &asrData)
{
    ErrorCode error = Success;

    if (CODE_SUCCESS == asrData.code && XF_fail == asrData.status) {
        switch (asrData.failType) {
        case XF_upload: {
            error = UploadAudioFileFail;
        } break;
        case XF_decode: {
            error = AudioDecodeFail;
        } break;
        case XF_recognize: {
            error = AudioRecognizeFail;
        } break;
        case XF_limit: {
            error = AudioExceedeLimit;
        } break;
        case XF_verify: {
            error = AudioVerifyFailed;
        } break;
        case XF_mute: {
            error = AudioMuteFile;
        } break;
        case XF_other: {
            error = AudioOther;
        } break;

        } //End switch failType
    } else if (CODE_NETWORK == asrData.code) {
        error = NetworkError;
    } else {
        //TODO:
        //    Now we don't care this error,may be handle
        //it in furture if needed
        error = DontCareError;
    }
    return error;
}
