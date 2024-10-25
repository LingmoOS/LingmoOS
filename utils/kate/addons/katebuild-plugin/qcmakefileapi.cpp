/*
    SPDX-FileCopyrightText: 2023 Alexander Neundorf <neundorf@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qcmakefileapi.h"
#include "kate_buildplugin_debug.h"

#include "hostprocess.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>

QCMakeFileApi::QCMakeFileApi(const QString &cmakeCacheFile, bool withSourceFiles)
    : QObject()
    , m_cmakeExecutable()
    , m_cacheFile()
    , m_buildDir()
    , m_withSourceFiles(withSourceFiles)
{
    qCDebug(KTEBUILD) << "QCMakeFileApi cachefile: " << cmakeCacheFile;
    QFileInfo fi(cmakeCacheFile);
    if (fi.isDir()) {
        qCDebug(KTEBUILD) << "isDir filepath: " << fi.absoluteFilePath() << " path " << fi.absolutePath();
        m_buildDir = fi.absoluteFilePath();
        m_cacheFile = m_buildDir + QStringLiteral("/CMakeCache.txt");

    } else {
        m_buildDir = fi.absolutePath();
        m_cacheFile = fi.absoluteFilePath();
    }

    qCDebug(KTEBUILD) << "builddir: " << m_buildDir << " cachefile: " << m_cacheFile;

    // get cmake name from file, ensure in any case we compute some absolute name, beside inside containers
    m_cmakeExecutable = safeExecutableName(findCMakeExecutable(m_cacheFile));
    m_cmakeGuiExecutable = safeExecutableName(findCMakeGuiExecutable(m_cmakeExecutable));
}

const QString &QCMakeFileApi::getCMakeExecutable() const
{
    return m_cmakeExecutable;
}

QString QCMakeFileApi::getCMakeGuiExecutable() const
{
    return m_cmakeGuiExecutable;
}

QString QCMakeFileApi::findCMakeGuiExecutable(const QString &cmakeExecutable) const
{
    if (!cmakeExecutable.isEmpty()) {
        QFileInfo fi(cmakeExecutable);
        //    qDebug() << "++++++++++ cmake: " << m_cmakeExecutable << " abs: " << fi.
        QString cmakeGui =
            fi.absolutePath() + QStringLiteral("/cmake-gui") + (cmakeExecutable.endsWith(QStringLiteral(".exe")) ? QStringLiteral(".exe") : QString());
        QFileInfo guiFi(cmakeGui);
        if (guiFi.isAbsolute() && guiFi.isFile() && guiFi.isExecutable()) {
            return cmakeGui;
        }
    }
    return QString();
}

QString QCMakeFileApi::findCMakeExecutable(const QString &cmakeCacheFile) const
{
    QFile cacheFile(cmakeCacheFile);
    if (cacheFile.open(QIODevice::ReadOnly)) {
        QRegularExpression re(QStringLiteral("CMAKE_COMMAND:.+=(.+)$"));

        QTextStream in(&cacheFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                QString cmakeExecutable = match.captured(1);
                QFileInfo fi(cmakeExecutable);
                if (fi.isAbsolute() && fi.isFile() && fi.isExecutable()) {
                    return cmakeExecutable;
                }
                break;
            }
        }
    }

    return QString();
}

QStringList QCMakeFileApi::getCMakeRequestCommandLine() const
{
    if (m_cmakeExecutable.isEmpty()) {
        return QStringList();
    }

    const QStringList commandLine = {m_cmakeExecutable, QStringLiteral("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"), m_buildDir};
    return {m_cmakeExecutable, QStringLiteral("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"), m_buildDir};
}

bool QCMakeFileApi::runCMake()
{
    if (m_cmakeExecutable.isEmpty()) {
        return false;
    }

    QStringList commandLine = getCMakeRequestCommandLine();

    m_cmakeSuccess = true;
    QProcess cmakeProc;
    cmakeProc.setProgram(commandLine.takeFirst());
    cmakeProc.setArguments(commandLine);
    connect(&cmakeProc, &QProcess::started, this, &QCMakeFileApi::handleStarted);
    connect(&cmakeProc, &QProcess::stateChanged, this, &QCMakeFileApi::handleStateChanged);
    connect(&cmakeProc, &QProcess::errorOccurred, this, &QCMakeFileApi::handleError);
    startHostProcess(cmakeProc);

    cmakeProc.waitForFinished();
    return m_cmakeSuccess;
}

void QCMakeFileApi::handleStarted()
{
    qCDebug(KTEBUILD) << "CMake process started !";
}

void QCMakeFileApi::handleStateChanged(QProcess::ProcessState newState)
{
    qCDebug(KTEBUILD) << "CMake process state changed: " << (int)newState << "!";
}

void QCMakeFileApi::handleError()
{
    qCDebug(KTEBUILD) << "CMakeFileAPI QProcess error !";
    m_cmakeSuccess = false;
}

bool QCMakeFileApi::writeQueryFiles()
{
    bool success = true;
    success = success && writeQueryFile("codemodel", 2);
    success = success && writeQueryFile("cmakeFiles", 1);
    return success;
}

bool QCMakeFileApi::writeQueryFile(const char *objectKind, int version)
{
    QDir buildDir(m_buildDir);
    QString queryFileDir = QStringLiteral("%1/.cmake/api/v1/query/client-kate/").arg(m_buildDir);
    buildDir.mkpath(queryFileDir);

    QString queryFilePath = QStringLiteral("%1/%2-v%3").arg(queryFileDir).arg(QLatin1String(objectKind)).arg(version);
    QFile file(queryFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    return true;
}

QJsonObject QCMakeFileApi::readJsonFile(const QString &filename) const
{
    const QDir replyDir(QStringLiteral("%1/.cmake/api/v1/reply/").arg(m_buildDir));

    QString absFileName = replyDir.absoluteFilePath(filename);
    qCDebug(KTEBUILD) << "Reading file: " << absFileName;

    QFile file(absFileName);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContents = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContents);
    QJsonObject docObj = jsonDoc.object();

    return docObj;
}

bool QCMakeFileApi::haveKateReplyFiles() const
{
    const QDir replyDir(QStringLiteral("%1/.cmake/api/v1/reply/").arg(m_buildDir));
    QStringList indexFiles = replyDir.entryList({QStringLiteral("index-*.json")}, QDir::Files);
    if (indexFiles.isEmpty()) {
        return false;
    }

    QString indexFile = replyDir.absoluteFilePath(indexFiles.at(0));

    QJsonObject docObj = readJsonFile(indexFile);

    QJsonObject replyObj = docObj.value(QStringLiteral("reply")).toObject();

    return (replyObj.contains(QStringLiteral("client-kate")) && (replyObj.value(QStringLiteral("client-kate")).isObject()));
}

QCMakeFileApi::TargetType QCMakeFileApi::typeFromJson(const QString &typeStr) const
{
    if (typeStr == QStringLiteral("EXECUTABLE")) {
        return TargetType::Executable;
    } else if (typeStr == QStringLiteral("UTILITY")) {
        return TargetType::Utility;
    } else if (typeStr.contains(QStringLiteral("LIBRARY"))) {
        return TargetType::Library;
    }
    return TargetType::Unknown;
}

bool QCMakeFileApi::readReplyFiles()
{
    const QDir replyDir(QStringLiteral("%1/.cmake/api/v1/reply/").arg(m_buildDir));
    QStringList indexFiles = replyDir.entryList({QStringLiteral("index-*.json")}, QDir::Files);
    if (indexFiles.isEmpty()) {
        return false;
    }

    m_sourceDir.clear();
    m_sourceFiles.clear();
    m_targets.clear();

    QString indexFile = replyDir.absoluteFilePath(indexFiles.at(0));
    qCDebug(KTEBUILD) << "CMakeFileAPI reply index file: " << indexFile;

    QJsonObject docObj = readJsonFile(indexFile);
    // qWarning() << "docObj: " << docObj;

    QJsonObject cmakeObj = docObj.value(QStringLiteral("cmake")).toObject();
    // qWarning() << "cmake: " << cmakeObj;

    QJsonObject replyObj = docObj.value(QStringLiteral("reply")).toObject();
    // qWarning() << "reply: " << replyObj;

    QJsonObject kateObj = replyObj.value(QStringLiteral("client-kate")).toObject();
    // qWarning() << "kate: " << kateObj;

    QJsonObject cmakeFilesObj = kateObj.value(QStringLiteral("cmakeFiles-v1")).toObject();
    // qWarning() << "cmakefiles: " << cmakeFilesObj;
    QString cmakeFilesFile = cmakeFilesObj.value(QStringLiteral("jsonFile")).toString();

    QJsonObject codeModelObj = kateObj.value(QStringLiteral("codemodel-v2")).toObject();
    // qWarning() << "code model: " << codeModelObj;
    QString codemodelFile = codeModelObj.value(QStringLiteral("jsonFile")).toString();

    // qWarning() << "cmakeFiles: " << cmakeFilesFile << " codemodelfile: " << codemodelFile;

    QJsonObject cmakeFilesDoc = readJsonFile(cmakeFilesFile);

    QJsonObject pathsObj = cmakeFilesDoc.value(QStringLiteral("paths")).toObject();
    m_sourceDir = pathsObj.value(QStringLiteral("source")).toString();
    // qWarning() << "srcdir: " << m_sourceDir;

    if (m_withSourceFiles) {
        QJsonArray cmakeFiles = cmakeFilesDoc.value(QStringLiteral("inputs")).toArray();
        qCDebug(KTEBUILD) << "Found " << cmakeFiles.count() << " files";
        for (int i = 0; i < cmakeFiles.count(); i++) {
            QJsonObject fileObj = cmakeFiles.at(i).toObject();
            bool isCMake = fileObj.value(QStringLiteral("isCMake")).toBool();
            bool isGenerated = fileObj.value(QStringLiteral("isGenerated")).toBool();
            QString filename = fileObj.value(QStringLiteral("path")).toString();
            if ((isCMake == false) && (isGenerated == false)) {
                qCDebug(KTEBUILD) << "#" << i << ": " << filename << " cmake: " << isCMake << " gen: " << isGenerated;
                m_sourceFiles.insert(filename);
            }
        }
    }

    QJsonObject codeModelDoc = readJsonFile(codemodelFile);
    QJsonArray configs = codeModelDoc.value(QStringLiteral("configurations")).toArray();
    // qWarning() << "configs: " << configs;

    for (int i = 0; i < configs.count(); i++) {
        std::vector<Target> targetsVec;
        QJsonObject configObj = configs.at(i).toObject();
        QString configName = configObj.value(QStringLiteral("name")).toString();
        if (configName.isEmpty()) {
            configName = QStringLiteral("NONE");
        }
        qCDebug(KTEBUILD) << "config: " << configName;
        m_configs.push_back(configName);
        QJsonArray projects = configObj.value(QStringLiteral("projects")).toArray();
        for (int projIdx = 0; projIdx < projects.count(); projIdx++) {
            QJsonObject projectObj = projects.at(projIdx).toObject();
            QString projectName = projectObj.value(QStringLiteral("name")).toString();
            if (!projectName.isEmpty()) {
                // this will usually be the first one. Should we check that this
                // one contains directoryIndex 0 ?
                qCDebug(KTEBUILD) << "projectname: " << projectName;
                m_projectName = projectName;
                break;
            }
        }
        QJsonArray targets = configObj.value(QStringLiteral("targets")).toArray();
        for (int tgtIdx = 0; tgtIdx < targets.count(); tgtIdx++) {
            QJsonObject targetObj = targets.at(tgtIdx).toObject();
            QString targetName = targetObj.value(QStringLiteral("name")).toString();

            QString targetJsonFile = targetObj.value(QStringLiteral("jsonFile")).toString();

            // qWarning() << "config: " << configName << " target: " << targetName << " json: " << targetJsonFile;

            QJsonObject targetDoc = readJsonFile(targetJsonFile);
            bool fromGenerator = targetDoc.value(QStringLiteral("isGeneratorProvided")).toBool();
            if (!fromGenerator) {
                TargetType type = typeFromJson(targetDoc.value(QStringLiteral("type")).toString());
                targetsVec.push_back({targetName, type});

                if (m_withSourceFiles) {
                    QJsonArray sources = targetDoc.value(QStringLiteral("sources")).toArray();
                    for (int srcIdx = 0; srcIdx < sources.count(); srcIdx++) {
                        QJsonObject sourceObj = sources.at(srcIdx).toObject();
                        QString filePath = sourceObj.value(QStringLiteral("path")).toString();
                        // qWarning() << "source: " << filePath;
                        m_sourceFiles.insert(filePath);
                    }
                }
            }
        }
        m_targets[configName] = targetsVec;
    }

    return true;
}

const QString &QCMakeFileApi::getProjectName() const
{
    return m_projectName;
}

const std::vector<QString> &QCMakeFileApi::getConfigurations() const
{
    return m_configs;
}

const std::set<QString> &QCMakeFileApi::getSourceFiles() const
{
    return m_sourceFiles;
}

const std::vector<QCMakeFileApi::Target> &QCMakeFileApi::getTargets(const QString &config) const
{
    auto it = m_targets.find(config);
    if (it == m_targets.end()) {
        return m_emptyTargets;
    }

    return it->second;
}

const QString &QCMakeFileApi::getSourceDir() const
{
    return m_sourceDir;
}

const QString &QCMakeFileApi::getBuildDir() const
{
    return m_buildDir;
}
