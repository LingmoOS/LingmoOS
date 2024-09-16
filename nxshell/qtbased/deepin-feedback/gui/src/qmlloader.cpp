/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "qmlloader.h"
#include <QProcess>
#include <QKeyEvent>
#include <QApplication>

QmlLoader::QmlLoader(QObject *parent)
    :QObject(parent)
{
    engine = new QQmlEngine(this);
    component = new QQmlComponent(engine, this);
    rootContext = new QQmlContext(engine, this);
    this->mDbusProxyer = new QmlLoaderDBus(this);

    init();

    qApp->installEventFilter(this);
}

QmlLoader::~QmlLoader()
{
    delete this->mDbusProxyer;
    delete this->rootContext;
    delete this->component;
    delete this->engine;
    delete this->adjunctAide;
}


void QmlLoader::load(QUrl url)
{
    this->component->loadUrl(url);
    this->rootObject = this->component->beginCreate(this->rootContext);
    if ( this->component->isReady() )
    {
        this->component->completeCreate();
    }
    else
    {
        qWarning() << this->component->errorString();
    }
}

void QmlLoader::reportBug()
{
    QMetaObject::invokeMethod(
                this->rootObject,
                "showMainWindow"
                );
}

void QmlLoader::reportBug(const QString &target)
{
    //set specified target
    QVariant contentValue = QVariant(target);
    QMetaObject::invokeMethod(
                this->rootObject,
                "switchProject",
                Q_ARG(QVariant, contentValue)
                );


    updateUiDraftData(target);

    QMetaObject::invokeMethod(
                this->rootObject,
                "showMainWindow"
                );
}


void QmlLoader::init()
{
    //makesure path exist
    QDir configDir;
    if (!configDir.exists(DRAFT_SAVE_PATH_NARMAL))
    {
        configDir.mkpath(DRAFT_SAVE_PATH_NARMAL);
    }

    adjunctAide = new AdjunctAide();
    connect(adjunctAide, SIGNAL(getScreenshotFinish(QString)), this, SIGNAL(getScreenshotFinish(QString)));
}

QString QmlLoader::getHomeDir()
{
    return QDir::homePath();
}

QStringList QmlLoader::getSupportAppList()
{
    QVariant returnValue;
    QMetaObject::invokeMethod(
                this->rootObject,
                "getSupportAppList",
                Q_RETURN_ARG(QVariant, returnValue)
                );
    return returnValue.toStringList();
}

bool QmlLoader::saveDraft(const QString &targetApp,
                          const DataConverter::FeedbackType type,
                          const QString &title,
                          const QString &email,
                          const bool &helpDeepin,
                          const QString &content)
{
    QString targetPath = DRAFT_SAVE_PATH_NARMAL + targetApp;
    QDir tmpDir;
    if (!tmpDir.exists(targetPath))
        tmpDir.mkpath(targetPath);

    if (!tmpDir.exists(targetPath + "/" + ADJUNCT_DIR_NAME))
        tmpDir.mkdir(targetPath + "/" + ADJUNCT_DIR_NAME);

    //write simple entries file
    QJsonObject jsonObj;
    jsonObj.insert("FeedbackType", int(type));
    jsonObj.insert("Title", title);
    jsonObj.insert("Email", email);
    jsonObj.insert("HelpDeepin",helpDeepin);

    QJsonDocument jsonDocument;
    jsonDocument.setObject(jsonObj);

    QFile simpleEntriesFile(targetPath + "/" + SIMPLE_ENTRIES_FILE_NAME);
    if (simpleEntriesFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        simpleEntriesFile.write(jsonDocument.toJson(QJsonDocument::Compact));
        simpleEntriesFile.close();
    }
    else
    {
        qWarning() << "Open simple entries file error: " << (targetPath + "/" + SIMPLE_ENTRIES_FILE_NAME);
        return false;
    }

    //write content file
    QFile contentFile(targetPath + "/" + CONTENT_FILE_NAME);
    if (contentFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        contentFile.write(content.toUtf8());
        contentFile.close();
    }
    else
    {
        qWarning() << "Open content file error: " << (targetPath + "/" + CONTENT_FILE_NAME);
        return false;
    }

    return true;
}

void QmlLoader::showManual()
{
    if (m_manualPro.isNull()) {
        const QString pro = "dman";
        const QStringList args("deepin-feedback");
        m_manualPro = new QProcess(this);
        connect(m_manualPro.data(), SIGNAL(finished(int)), m_manualPro.data(), SLOT(deleteLater()));
        m_manualPro->start(pro, args);
    }
}

void QmlLoader::clearAllDraft()
{
    AdjunctAide::removeDirWidthContent(DRAFT_SAVE_PATH_NARMAL);
    QDir tmpDir;
    tmpDir.mkpath(DRAFT_SAVE_PATH_NARMAL);
}

void QmlLoader::clearDraft(const QString &targetApp)
{
    AdjunctAide::removeDirWidthContent(DRAFT_SAVE_PATH_NARMAL + targetApp);
}

void QmlLoader::clearSysAdjuncts(const QString &targetApp)
{
    AdjunctAide::removeSysAdjuncts(DRAFT_SAVE_PATH_NARMAL + targetApp + "/Adjunct");
}

QString QmlLoader::addAdjunct(const QString &filePath, const QString &target)
{
    if (QFile::exists(target))
    {
        qWarning() << "Attachment already exist: " << target;
        return "";
    }

    QString targetPath = DRAFT_SAVE_PATH_NARMAL + target + "/" + ADJUNCT_DIR_NAME;
    QDir tmpDir;
    if (!tmpDir.exists(targetPath))
    {
        tmpDir.mkpath(targetPath);
    }

    QString targetFileName = targetPath + getFileNameFromPath(filePath);
    //copy file from target path to draft location
    if (QFile::copy(filePath, targetFileName))
        return targetFileName;
    else
        return "";
}

void QmlLoader::removeAdjunct(const QString &filePath)
{
    QFile::remove(filePath);
}

bool QmlLoader::draftTargetExist(const QString &target)
{
    QDir tmpDir(DRAFT_SAVE_PATH_NARMAL + target);
    tmpDir.setFilter(QDir::Files |QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
    return tmpDir.exists() && tmpDir.entryList().length() > 0;
}

bool QmlLoader::draftNotEmpty(const QString &target)
{
    //get draft
    Draft draft = getDraft(target);

    return draft.title != "" || draft.content != "" || draft.email != "" ||
            draft.feedbackType != DataConverter::DFeedback_Bug ||
            draft.helpDeepin != true || draft.adjunctPathList.length() > 0;
}

void QmlLoader::updateUiDraftData(const QString &target)
{
    //get draft
    Draft draft = getDraft(target);

    //init value
    QVariant contentValue = QVariant(draft.content);
    QMetaObject::invokeMethod(
                this->rootObject,
                "setReportContent",
                Q_ARG(QVariant, contentValue)
                );

    QVariant listValue = QVariant(draft.adjunctPathList);
    QMetaObject::invokeMethod(
                this->rootObject,
                "setAdjunctsPathList",
                Q_ARG(QVariant, listValue)
                );

    QVariant feedbackTypeValue = QVariant(draft.feedbackType);
    QVariant reportTitleValue = QVariant(draft.title);
    QVariant emailValue = QVariant(draft.email);
    QVariant helpDeepinValue = QVariant(draft.helpDeepin);
    QMetaObject::invokeMethod(
                this->rootObject,
                "setSimpleEntries",
                Q_ARG(QVariant,feedbackTypeValue),
                Q_ARG(QVariant,reportTitleValue),
                Q_ARG(QVariant,emailValue),
                Q_ARG(QVariant,helpDeepinValue)
                );
}

void QmlLoader::getScreenShot(const QString &target)
{
    adjunctAide->getScreenShot(target);
}

bool QmlLoader::canAddAdjunct(const QString &target)
{
    return getAdjunctsSize(target) < ADJUNCTS_MAX_SIZE;
}

Draft QmlLoader::getDraft(const QString &targetApp)
{
    Draft tmpDraft;
    QDir configDir;
    if (!configDir.exists(DRAFT_SAVE_PATH_NARMAL + targetApp))
    {
        //not exist,return empty value
        return tmpDraft;
    }
    else
    {
        //get content
        if (QFile::exists(DRAFT_SAVE_PATH_NARMAL + targetApp + "/" + CONTENT_FILE_NAME))
        {
            QFile contentFile(DRAFT_SAVE_PATH_NARMAL + targetApp + "/" + CONTENT_FILE_NAME);
            if (contentFile.open(QIODevice::ReadOnly))
            {
                tmpDraft.content = QString(contentFile.readAll());
                contentFile.close();
            }
        }

        //get simple entries from json file
        if (QFile::exists(DRAFT_SAVE_PATH_NARMAL + targetApp + "/" + SIMPLE_ENTRIES_FILE_NAME))
        {
            QFile entriesFile(DRAFT_SAVE_PATH_NARMAL + targetApp + "/" + SIMPLE_ENTRIES_FILE_NAME);
            if (entriesFile.open(QIODevice::ReadOnly))
            {
                QByteArray tmpByteArry = entriesFile.readAll();
                parseJsonData(tmpByteArry,&tmpDraft);
                entriesFile.close();
            }
        }

        //get adjuncts path's list
        if (QFile::exists(DRAFT_SAVE_PATH_NARMAL + targetApp + "/" + ADJUNCT_DIR_NAME))
        {
            QDir tmpDir(DRAFT_SAVE_PATH_NARMAL + targetApp + "/" + ADJUNCT_DIR_NAME);
            QFileInfoList infoList = tmpDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot ,QDir::Name);
            for (int i = 0; i < infoList.length(); i ++)
            {
                if (infoList.at(i).isFile())
                    tmpDraft.adjunctPathList.append(infoList.at(i).filePath());
            }
        }

        return tmpDraft;
    }
}

void QmlLoader::parseJsonData(const QByteArray &byteArray, Draft *draft)
{
    QJsonParseError jsonError;
    QJsonDocument parseDoucment = QJsonDocument::fromJson(byteArray, &jsonError);
    if(jsonError.error == QJsonParseError::NoError)
    {
        if(parseDoucment.isObject())
        {
            QJsonObject obj = parseDoucment.object();
            if(obj.contains("FeedbackType"))
            {
                QJsonValue feedbackTypeValue = obj.take("FeedbackType");
                if(feedbackTypeValue.isDouble())
                    draft->feedbackType = DataConverter::FeedbackType(feedbackTypeValue.toVariant().toInt());
            }
            if(obj.contains("Title"))
            {
                QJsonValue titleValue = obj.take("Title");
                if(titleValue.isString())
                    draft->title = titleValue.toString();
            }
            if(obj.contains("Email"))
            {
                QJsonValue emailValue = obj.take("Email");
                if(emailValue.isString())
                    draft->email = emailValue.toString();
            }
            if (obj.contains("HelpDeepin"))
            {
                QJsonValue helpDeepinValue = obj.take("HelpDeepin");
                if (helpDeepinValue.isBool())
                    draft->helpDeepin = helpDeepinValue.toBool();
            }
        }
    }
}

void QmlLoader::parseJsonArray(const QByteArray &byteArray, QStringList *emailsList)
{
    QJsonParseError jsonError;
    QJsonDocument parseDoucment = QJsonDocument::fromJson(byteArray, &jsonError);
    if(jsonError.error == QJsonParseError::NoError && parseDoucment.isArray())
    {
        QJsonArray array = parseDoucment.array();
        int arrayLength = array.count();

        for (int i = 0; i < arrayLength; i ++)
        {
            emailsList->append(array.at(i).toString());
        }
    }
}

QString QmlLoader::getFileNameFromPath(const QString &filePath)
{
    int tmpIndex = filePath.lastIndexOf("/");
    if (tmpIndex == -1)
        return "";
    else
        return filePath.mid(tmpIndex + 1, filePath.length() - tmpIndex - 1);
}

qint64 QmlLoader::getAdjunctsSize(const QString &target)
{
    qint64 tmpSize = 0;
    if (QFile::exists(DRAFT_SAVE_PATH_NARMAL + target + "/" + ADJUNCT_DIR_NAME))
    {
        QDir tmpDir(DRAFT_SAVE_PATH_NARMAL + target + "/" + ADJUNCT_DIR_NAME);
        QFileInfoList infoList = tmpDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot ,QDir::Name);
        for (int i = 0; i < infoList.length(); i ++)
        {
            if (infoList.at(i).isFile()){
                tmpSize += infoList.at(i).size();
            }
        }

        return tmpSize;
    }
    else
        return 0;
}

qint64 QmlLoader::getAdjunctSize(const QString &fileName)
{
    QFile tmpFile(fileName);
    return tmpFile.size();
}

bool QmlLoader::adjunctExist(const QString &filePath, const QString &target)
{
    QString targetFileName = DRAFT_SAVE_PATH_NARMAL + target + "/" + ADJUNCT_DIR_NAME + getFileNameFromPath(filePath);
    if (QFile::exists(targetFileName))
        return true;
    else
        return false;
}

void QmlLoader::saveEmail(const QString &email)
{
    QFile file(EMAILS_LIST_FILE_NAME);
    if (!file.exists(EMAILS_LIST_FILE_NAME))
    {
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    QStringList emailsList = getEmails();
    int oldIndex = emailsList.indexOf(email);
    if (oldIndex != -1)//already exist
    {
        emailsList.removeAt(oldIndex);
    }
    emailsList.insert(0, email);

    QJsonDocument jsonDocument;
    jsonDocument.setArray(QJsonArray::fromStringList(emailsList));

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write(jsonDocument.toJson(QJsonDocument::Compact));
        file.close();
    }
    else
        qWarning() << "Open emails list file error!";
}

QStringList QmlLoader::getEmails()
{
    QStringList emailsList;
    QFile entriesFile(EMAILS_LIST_FILE_NAME);
    if (entriesFile.open(QIODevice::ReadOnly))
    {
        QByteArray tmpByteArry = entriesFile.readAll();
        parseJsonArray(tmpByteArry,&emailsList);
        entriesFile.close();
    }

    return emailsList;
}

QString QmlLoader::getMatchEmailPart(const QString &text)
{
    QStringList emailList = getEmails();
    foreach (QString email, emailList) {
        if (email.indexOf(text, 0, Qt::CaseInsensitive) == 0)
            return email.mid(text.length());
    }

    return "";
}

bool QmlLoader::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_F1) {
            qDebug() << "show manual for help...";
            showManual();
            return true;
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

QmlLoaderDBus::QmlLoaderDBus(QmlLoader *parent):
    QDBusAbstractAdaptor(parent),
    m_parent(parent)
{
    QDBusConnection::sessionBus().registerObject(DBUS_PATH, parent);
}

QmlLoaderDBus::~QmlLoaderDBus()
{

}

void QmlLoaderDBus::switchProject(QString name)
{
    m_parent->reportBug(name);
}
