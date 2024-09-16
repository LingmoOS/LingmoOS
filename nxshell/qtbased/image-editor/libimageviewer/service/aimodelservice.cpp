// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aimodelservice.h"
#include "aimodelservice_p.h"

#include <QPushButton>
#include <QtConcurrent>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#include <DDialog>
#include <DFileDialog>
#include <DMessageManager>

#include "unionimage/unionimage.h"
#include "service/commonservice.h"

DWIDGET_USE_NAMESPACE

// 超时限制10分钟
static const int s_DBusTimeout = 1000 * 60 * 10;
// 模型信息
static const char *s_ModelColoring = "coloring";
static const char *s_ModelSuperResol = "super-resolution";
static const char *s_ModelBlurBkg = "blurredBackground";
static const char *s_ModelBkgCut = "portraitCutout";
static const char *s_ModelHand = "hand";
static const char *s_ModelCartoon2D = "cartoon2d";
static const char *s_ModelCartoon3D = "cartoon3d";
static const char *s_ModelSketch = "sketch";
// DBus
static const QString s_EnhanceService = "com.deepin.imageenhance";
static const QString s_EnhancePath = "/com/deepin/imageenhance";
static const QString s_EnhanceInterface = "com.deepin.imageenhance";
// DBus Interface
static const QString s_EnhanceProcMethod = "imageEnhance";
static const QString s_EnhanceBlurBkg = "blurredBackground";
static const QString s_EnhancePortraitCout = "portraitCutout";
// DBus Signal
static const QString s_EnhanceFinishSignal = "finishedEnhance";

AIModelServiceData::AIModelServiceData(AIModelService *q)
    : qptr(q)
{
    supportNameToModel = initDBusModelList();
    qInfo() << qPrintable("Support image enhance models:") << supportNameToModel;

    // QTemporaryDir::isValid() 会创建临时文件路径，在没有模型数据时不进行路径判断
    if (!supportNameToModel.isEmpty()) {
        enhanceTemp.reset(new QTemporaryDir);
        if (!enhanceTemp->isValid()) {
            qWarning() << qPrintable("Create enhance temp dir failed") << enhanceTemp->errorString();
        } else {
            qInfo() << qPrintable("Enhance temp dir:") << enhanceTemp->path();
        }

        convertTemp.reset(new QTemporaryDir);
        if (!convertTemp->isValid()) {
            qWarning() << qPrintable("Create convert temp dir failed") << convertTemp->errorString();
        } else {
            qInfo() << qPrintable("Convert temp dir:") << convertTemp->path();
        }
    }
}

/**
   @return 返回当前存在的图像增强模型列表，通过读取DBu接口属性 modelList 取得，仅初始化调用一次
 */
QList<QPair<int, QString>> AIModelServiceData::initDBusModelList()
{
    // 预期的模型项顺序
    QStringList sortModelList = {s_ModelColoring,
                                 s_ModelSuperResol,
                                 s_ModelBlurBkg,
                                 s_ModelBkgCut,
                                 s_ModelHand,
                                 s_ModelCartoon2D,
                                 s_ModelCartoon3D,
                                 s_ModelSketch};

    // 调用DBus接口获取模型列表
    QDBusInterface interface(s_EnhanceService, s_EnhancePath, s_EnhanceInterface, QDBusConnection::systemBus());
    QStringList modelList = interface.property("modelList").toStringList();

    if (modelList.isEmpty()) {
        auto error = interface.lastError();
        qInfo() << QString("[Enhance DBus] No AI models on device? Get model list failed, %1: %2").arg(error.name()).arg(error.message());
        return {};
    }

    // 调整模型顺序, 模型-名称排序列表
    QList<QPair<int, QString>> mapModelList;
    for (const QString &model : sortModelList) {
        if (modelList.contains(model)) {
            modelList.removeOne(model);

            auto ptr = createModelInfo(model);
            mapModelInfo.insert(ptr->modelID, ptr);
            mapModelList.append(qMakePair(ptr->modelID, ptr->modelTr));
        }
    }

    // 不在名单的模型追加到末尾
    for (const QString &appendModel : modelList) {
        auto ptr = createModelInfo(appendModel);
        mapModelInfo.insert(ptr->modelID, ptr);
        mapModelList.append(qMakePair(ptr->modelID, ptr->modelTr));
    }

    return mapModelList;
}

/**
   @brief 根据传入的模型名称创建模型信息，包含模型ID和翻译名称等。
 */
ModelPtr AIModelServiceData::createModelInfo(const QString &model)
{
    // 记录默认的模型-名称映射
    static QMap<QString, int> mapNameModel = {
        {s_ModelColoring, Coloring},
        {s_ModelSuperResol, SuperResol},
        {s_ModelBlurBkg, BackgroundBlur},
        {s_ModelBkgCut, BackgroundCut},
        {s_ModelHand, Hand},
        {s_ModelCartoon2D, Cartoon2D},
        {s_ModelCartoon3D, Cartoon3D},
        {s_ModelSketch, Sketch},
    };

    // 用于国际化翻译
    static QMap<int, QString> mapModelTrName = {
        {Coloring, QObject::tr("Colorize pictures")},
        {SuperResol, QObject::tr("Upgrade resolution")},
        {BackgroundBlur, QObject::tr("Blurred background")},
        {BackgroundCut, QObject::tr("Delete background")},
        {Hand, QObject::tr("Hand-drawn cartoons")},
        {Cartoon2D, QObject::tr("2D Manga")},
        {Cartoon3D, QObject::tr("3D Manga")},
        {Sketch, QObject::tr("Sketch")},
    };

    ModelPtr ptr(new ModelInfo);
    ptr->model = model;

    if (mapNameModel.contains(model)) {
        ptr->modelID = mapNameModel.value(model);
        ptr->modelTr = mapModelTrName.value(ptr->modelID);
    } else {
        ptr->modelTr = QObject::tr(model.toUtf8().data());
        ptr->modelID = UserType + userModelCount;
        userModelCount++;
    }

    return ptr;
}

/**
   @brief 创建用户重试的提示信息，
 */
DFloatingMessage *AIModelServiceData::createReloadMessage(const QString &output)
{
    DFloatingMessage *msg = new DFloatingMessage(DFloatingMessage::ResidentType);
    msg->setAttribute(Qt::WA_DeleteOnClose);
    msg->setIcon(QIcon(":/common/error.svg"));
    msg->setMessage(QObject::tr("Processing failure."));

    QPushButton *reload = new QPushButton(QObject::tr("Retry"), msg);
    msg->setWidget(reload);

    QObject::connect(qptr, &AIModelService::clearPreviousEnhance, msg, &DFloatingMessage::close);
    QObject::connect(reload, &QPushButton::clicked, qptr, [=]() {
        msg->close();
        qptr->reloadImageProcessing(output);
    });

    return msg;
}

void AIModelServiceData::startDBusTimer()
{
    if (!dbusTimer.isActive()) {
        dbusTimer.start(s_DBusTimeout, qptr);
    }
}

void AIModelServiceData::stopDBusTimer()
{
    dbusTimer.stop();
}

AIModelService::AIModelService(QObject *parent)
    : QObject(parent)
    , dptr(new AIModelServiceData(this))
{
    connect(&dptr->enhanceWatcher, &QFutureWatcherBase::finished, this, [this]() {
        EnhancePtr ptr = dptr->enhanceWatcher.result();
        if (ptr.isNull()) {
            return;
        }

        auto curState = static_cast<AIModelService::State>(ptr->state.loadAcquire());
        if (AIModelService::Cancel == curState) {
            // 处理中断，不继续处理
            return;
        } else if (AIModelService::LoadFailed == curState) {
            Q_EMIT enhanceEnd(ptr->source, ptr->output, curState);
        } else {
            // Note: 备用的超时机制
#if 0
            // 正常发送消息，等待消息结束
            dptr->startDBusTimer();
#endif
        }
    });

    // 绑定DBus信号
    bool conn = QDBusConnection::systemBus().connect(s_EnhanceService,
                                                     s_EnhancePath,
                                                     s_EnhanceInterface,
                                                     s_EnhanceFinishSignal,
                                                     this,
                                                     SLOT(onDBusEnhanceEnd(const QString &, int)));
    if (!conn) {
        qWarning()
            << QString("[Enhance DBus] Connect dbus %1 signal %2 failed").arg(s_EnhanceInterface).arg(s_EnhanceFinishSignal);
    }
}

AIModelService::~AIModelService() {}

AIModelService *AIModelService::instance()
{
    static AIModelService ins;
    return &ins;
}

/**
   @return 返回是否允许使用AI模型，目前仅在安装模型后启用。
 */
bool AIModelService::isValid() const
{
    return dptr && !dptr->supportNameToModel.isEmpty();
}

/**
   @return 返回文件 \a filePath 的图像增强状态，是否处于运行中。
 */
AIModelService::State AIModelService::enhanceState(const QString &filePath)
{
    if (isValid() && dptr->enhanceCache.contains(filePath)) {
        auto ptr = dptr->enhanceCache.value(filePath);
        return static_cast<State>(ptr->state.loadAcquire());
    }

    return None;
}

/**
   @return 当前提供的图像增强模型
 */
QList<QPair<int, QString>> AIModelService::supportModel() const
{
    return dptr->supportNameToModel;
}

/**
   @brief 判断传入模型是否允许使用，将根据图片信息进行判断
        1. 仅允许静态图
        2. 其他模型： （可转换方向）不允许像素大小超过 2160x1440
           超分辨率模型: 不允许像素大小 512x512
 */
AIModelService::Error AIModelService::modelEnabled(int modelID, const QString &filePath) const
{
    auto info = LibCommonService::instance()->getImgInfoByPath(filePath);
    if (imageViewerSpace::ImageTypeStatic != info.imageType) {
        return FormatError;
    }

    switch (modelID) {
        case AIModelServiceData::SuperResol: {
            const int resolLimitWidth = 512;
            const int resolLimitHeight = 512;

            if ((resolLimitWidth < info.imgOriginalWidth) || (resolLimitHeight < info.imgOriginalHeight)) {
                return PixelSizeError;
            }
            break;
        }
        default: {
            // 暂时不再需要看图前端进行图片大小限制，屏蔽此处判断
#if 0
            const int normalLimitWidth = 2160;
            const int normalLimitHeight = 1440;

            if (info.imgOriginalHeight < info.imgOriginalWidth) {
                if ((normalLimitWidth < info.imgOriginalWidth) || (normalLimitHeight < info.imgOriginalHeight)) {
                    return PixelSizeError;
                }
            } else {
                // 高比宽大，旋转方向
                if ((normalLimitWidth < info.imgOriginalHeight) || (normalLimitHeight < info.imgOriginalWidth)) {
                    return PixelSizeError;
                }
            }
#endif
            break;
        }
    }

    return NoError;
}

/**
   @brief 使用 \a type 类型模型执行图像增强处理 \a filePath，此函数会立即返回。
        当文件类型非 png 时，将根据传入的图片 \a image 信息转存图片为png格式。
        图像增强结果通过 enhanceEnd() 抛出。

        当前图像处理流程为:
        * 图像传入，记录原数据和转换类型
        * 数据传入子线程，主要用于将原始文件数据转换为PNG文件，耗时不定，移入子线程处理
        * 子线程中调用DBus接口图像增强处理
            * DBus接口调用失败，标记处理失败，子线程结束后抛出执行失败信号 enhanceEnd()
            * DBus接口调用成功，等待DBus处理完成信号 onDBusEnhanceEnd()
                * 在完成槽函数中调用处理完成信号 enhanceEnd()
        ** 图像处理过程中可终止任务，将标记任务状态为 Cancel ，后续流程对 Cancel 任务不再处理

   @sa enhanceEnd, onDBusEnhanceEnd
 */
QString AIModelService::imageProcessing(const QString &filePath, int modelID, const QImage &image)
{
    if (!dptr->mapModelInfo.contains(modelID)) {
        return {};
    }

    resetProcess();

    // 如果图片已是增强后的图片，则获取源图片进行处理
    QString sourceFile = sourceFilePath(filePath);

    // 生命周期交由子线程维护
    QImage caputureImage = image.copy();
    if (!dptr->enhanceTemp) {
        return {};
    }
    dptr->lastOutput = dptr->enhanceTemp->filePath(QString("%1.png").arg(dptr->enhanceCache.size()));
    QString model = dptr->mapModelInfo.value(modelID)->model;

    EnhancePtr ptr(new EnhanceInfo(sourceFile, dptr->lastOutput, model));
    ptr->index = dptr->enhanceCache.size();
    ptr->state = Loading;
    dptr->enhanceCache.insert(ptr->output, ptr);

    qInfo() << QString("Call enhance processing %1, %2").arg(dptr->lastOutput).arg(model);

    QFuture<EnhancePtr> f = QtConcurrent::run([=]() -> EnhancePtr {
        if (AIModelService::Cancel == ptr->state.loadAcquire()) {
            return ptr;
        }

        // 写入文件移动到子线程。
        QString tmpSrcFile = checkConvertFile(sourceFile, caputureImage);
        if (tmpSrcFile.isEmpty()) {
            tmpSrcFile = ptr->source;
        }

        // 若DBus调用失败，则直接返回错误
        bool ret = AIModelServiceData::sendImageEnhance(tmpSrcFile, ptr->output, ptr->model);
        if (!ret) {
            ptr->state.storeRelease(LoadFailed);
        }

        return ptr;
    });
    dptr->enhanceWatcher.setFuture(f);

    Q_EMIT enhanceStart();
    return dptr->lastOutput;
}

/**
   @brief 重新尝试之前的模型调用，仅允许重复最后一次调用
 */
void AIModelService::reloadImageProcessing(const QString &filePath)
{
    // 仅允许最后一次调用
    EnhancePtr ptr = dptr->enhanceCache.value(filePath);
    if (ptr.isNull() || ptr->index != dptr->enhanceCache.size() - 1) {
        return;
    }

    resetProcess();

    // 如果图片已是增强后的图片，则获取源图片进行处理
    QString sourceFile = sourceFilePath(filePath);
    ptr->state.storeRelease(Loading);
    qInfo() << QString("Reload enhance processing %1, %2").arg(ptr->output).arg(ptr->model);

    QFuture<EnhancePtr> f = QtConcurrent::run([=]() -> EnhancePtr {
        if (AIModelService::Cancel == ptr->state.loadAcquire()) {
            return ptr;
        }

        // 已处理过的数据，一般存在缓存
        QString tmpSrcFile = checkConvertFile(sourceFile, QImage());
        if (tmpSrcFile.isEmpty()) {
            tmpSrcFile = ptr->source;
        }

        // 若 DBus 调用失败，则直接返回错误
        bool ret = AIModelServiceData::sendImageEnhance(tmpSrcFile, ptr->output, ptr->model);
        if (!ret) {
            ptr->state.storeRelease(LoadFailed);
        }

        return ptr;
    });
    dptr->enhanceWatcher.setFuture(f);

    Q_EMIT enhanceReload(filePath);
}

/**
   @brief 复位进行中的图像处理，实际run调用的处理不会立即关闭，主要用于标记状态。
        同时发送信号清理之前的增强信息，例如关闭含有"重试"选项的浮动提示
 */
void AIModelService::resetProcess()
{
    if (dptr->enhanceWatcher.isRunning()) {
        dptr->enhanceWatcher.cancel();
    }
    // 清理之前的图像增强状态
    Q_EMIT clearPreviousEnhance();
}

/**
   @brief 终止 \a filePath 的加载，仅处理在 Loading 状态的服务
 */
void AIModelService::cancelProcess(const QString &output)
{
    resetProcess();
    // 标记当前处理图片状态为Cancel
    if (dptr->enhanceCache.contains(output)) {
        EnhancePtr ptr = dptr->enhanceCache.value(output);
        if (!ptr.isNull() && Loading == ptr->state.loadAcquire()) {
            ptr->state.store(Cancel);

            Q_EMIT enhanceEnd(ptr->source, ptr->output, Cancel);
        }
    }
}

/**
   @return 返回 \a filePath 是否为图像增强后的临时文件
 */
bool AIModelService::isTemporaryFile(const QString &filePath)
{
    return isValid() && dptr->enhanceCache.contains(filePath);
}

/**
   @return 若为图像增强文件，返回 \a filePath 指向的源文件，否则返回自身
 */
QString AIModelService::sourceFilePath(const QString &filePath)
{
    if (isValid() && dptr->enhanceCache.contains(filePath)) {
        auto ptr = dptr->enhanceCache.value(filePath);
        return ptr->source;
    }
    return filePath;
}

/**
   @return 返回最后处理的输出文件路径
 */
QString AIModelService::lastProcOutput() const
{
    return dptr->lastOutput;
}

/**
   @return 返回是否正在等待保存，若之前还未保存，不会重复弹窗
 */
bool AIModelService::isWaitSave() const
{
    return dptr->waitSave;
}

/**
   @brief 弹出对话框提示是否保存当前文件 \a filePath
 */
void AIModelService::saveFileDialog(const QString &filePath, QWidget *target)
{
    if (isWaitSave()) {
        return;
    }
    dptr->waitSave = true;

    Dtk::Widget::DDialog expiredDialog(target);
    expiredDialog.setIcon(QIcon::fromTheme("deepin-image-viewer"));
    expiredDialog.setMessage(tr("Image not saved, Do you want to save it?"));

    expiredDialog.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
    const int suggestRet = expiredDialog.addButton(tr("Save as"), true, Dtk::Widget::DDialog::ButtonRecommend);

    // DDialog 在 showEvent 中调整弹窗大小，但并未重新设置位置，捕获展示信号，重新设置坐标居中于父窗口
    QObject::connect(&expiredDialog, &DDialog::visibleChanged, this, [&](bool b) {
        if (b) {
            expiredDialog.moveToCenter();
        }
    });

    int ret = expiredDialog.exec();
    if (ret == suggestRet) {
        saveEnhanceFileAs(filePath, target);
    }

    dptr->waitSave = false;
}

/**
   @brief 保存增强后的图像 \a filePath ， 如果非图像增强文件则不进行保存
 */
void AIModelService::saveEnhanceFile(const QString &filePath)
{
    if (!isTemporaryFile(filePath)) {
        return;
    }

    // 覆盖原文件
    saveFile(filePath, sourceFilePath(filePath));
}

/**
   @brief 另存增强后的图像 \a filePath ， 如果非图像增强文件则不进行保存
 */
void AIModelService::saveEnhanceFileAs(const QString &filePath, QWidget *target)
{
    if (!isTemporaryFile(filePath)) {
        return;
    }

    saveTemporaryAs(filePath, sourceFilePath(filePath), target);
}

/**
   @brief 判断传入错误类型 \a error 并使用浮动提示窗提示。
        若错误通过这种方式提示，则返回 true
 */
bool AIModelService::detectErrorAndNotify(QWidget *targetWidget, AIModelService::Error error, const QString &output)
{
    bool detectError = true;
    switch (error) {
        case FormatError:
            DMessageManager::instance()->sendMessage(
                targetWidget, QIcon(":/common/error.svg"), tr("Image format is not supported, please switch the image."));
            break;
        case PixelSizeError:
            DMessageManager::instance()->sendMessage(targetWidget,
                                                     QIcon(":/common/error.svg"),
                                                     tr("The image resolution exceeds the limit, please switch the image."));
            break;
        case LoadFiledError:
            DMessageManager::instance()->sendMessage(targetWidget, dptr->createReloadMessage(output));
            break;
        case NotDetectPortraitError:
            DMessageManager::instance()->sendMessage(
                targetWidget, QIcon(":/common/error.svg"), tr("Portrait not detected, switch pictures."));
            break;
        default:
            detectError = false;
            break;
    }

    return detectError;
}

/**
   @brief 应用判断超时处理
 */
void AIModelService::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == dptr->dbusTimer.timerId()) {
        // 触发超时
        dptr->stopDBusTimer();
        // 获取最近的输出图片
        auto ptr = dptr->enhanceCache.value(dptr->lastOutput);
        if (ptr) {
            ptr->state.storeRelease(AIModelService::LoadTimeout);
            Q_EMIT enhanceEnd(ptr->source, ptr->output, LoadFailed);
        }
    }

    QObject::timerEvent(e);
}

/**
   @brief 检测文件路径 \a newPath 是否可用，只读文件和路径不可保存
 */
bool AIModelService::checkFileSavable(const QString &newPath, QWidget *target)
{
    // 如果文件存在，检查文件是否可写
    QFileInfo info(newPath);
    if (info.exists()) {
        if (!info.isWritable()) {
            showWarningDialog(tr("This file is read-only, please save with another name."), target);
            return false;
        }

    } else {
        // 如果文件不存在，检查目录是否可写
        QString dirPath = info.absolutePath();
        QFileInfo dir(dirPath);
        if (dir.isDir() && !dir.isWritable()) {
            showWarningDialog(tr("You do not have permission to save files here, please change and retry."), target);
            return false;
        }
        return true;
    }
    return true;
}

/**
   @brief 展示提示信息 \a notify .
 */
void AIModelService::showWarningDialog(const QString &notify, QWidget *target)
{
    DDialog dialog(target);
    dialog.setIcon(QIcon::fromTheme("deepin-image-viewer"));
    dialog.setMessage(" ");
    dialog.addButton(tr("OK"));

    // DDialog 在 showEvent 中调整弹窗大小，但并未重新设置位置，捕获展示信号，重新设置坐标居中于父窗口
    QObject::connect(&dialog, &DDialog::visibleChanged, this, [&](bool b) {
        if (b) {
            // NOTE: 异常处理，DDialog在部分情况下计算提示信息的布局错误，调整为弹出后重设文本
            //  以使得正常计算 messageLabel 的大小。
            dialog.setMessage(notify);
            dialog.moveToCenter();
        }
    });

    dialog.exec();
}

/**
   @brief 保存文件 \a filePath 到 \a newPath , 默认覆盖 \a newPath 存在文件时无效,
        需要先移除旧文件。
 */
bool AIModelService::saveFile(const QString &filePath, const QString &newPath)
{
    // QFile::copy() will not overwrite.
    if (QFile::exists(newPath)) {
        QFile file(newPath);
        if (!file.remove()) {
            qWarning() << QString("Remove previous file failed! %1").arg(file.errorString());
            return false;
        }
    }

    bool ret = QFile::copy(filePath, newPath);
    if (!ret) {
        qWarning() << QString("Copy temporary file %1 failed").arg(filePath);
    }

    return ret;
}

/**
   @brief 弹出保存框将 \a filePath 保存，建议目录为源文件目录 \a sourcePath
 */
void AIModelService::saveTemporaryAs(const QString &filePath, const QString &sourcePath, QWidget *target)
{
    // 保存文件路径异常会尝试重试
    bool retry = false;
    do {
        // 默认退出
        retry = false;

        QFileInfo info(sourcePath);
        QString dir = info.absolutePath();
        if (dir.isEmpty()) {
            dir = QDir::homePath();
        }

        Dtk::Widget::DFileDialog dialog(target, tr("Save"));
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setDirectory(dir);
        dialog.selectFile(info.completeBaseName() + ".png");
        dialog.setNameFilter("*.png");

        int mode = dialog.exec();
        if (QDialog::Accepted == mode) {
            auto files = dialog.selectedFiles();
            if (files.isEmpty()) {
                return;
            }

            QString newPath = files.value(0);

            // 判断文件是否可保存
            if (!checkFileSavable(newPath, target)) {
                retry = true;
                continue;
            }

            saveFile(filePath, newPath);
        }
    } while (retry);
}

/**
   @brief 判断模型是否支持源文件 \a filePath, 若为不支持类型，转换为 png 格式图片。
        需考虑图片变更的场景。
 */
QString AIModelService::checkConvertFile(const QString &filePath, const QImage &image) const
{
    // 判断图片变更
    QMutexLocker _locker(&dptr->cacheMutex);
    if (dptr->convertCache.contains(filePath)) {
        return dptr->convertCache.value(filePath);
    }

    if (image.isNull()) {
        return {};
    }

    if (!dptr->convertTemp) {
        return {};
    }

    QString cvtFile;
    cvtFile = dptr->convertTemp->filePath(QString("%1_%2.png").arg(dptr->convertCache.size()).arg(QFileInfo(filePath).fileName()));

    _locker.unlock();
    if (!image.save(cvtFile, "PNG")) {
        return {};
    }

    _locker.relock();
    dptr->convertCache.insert(filePath, cvtFile);
    return cvtFile;
}

/**
   @brief 发送DBus图像增强处理消息。
        删除背景，模糊背景使用单独接口。
 */
bool AIModelServiceData::sendImageEnhance(const QString &source, const QString &output, const QString &model)
{
    // 此接口调用后程序停止，因此不适用 QDBusInterface::isValid() 直接调用 call() 会唤醒 DBus 服务
    QDBusInterface interface(s_EnhanceService, s_EnhancePath, s_EnhanceInterface, QDBusConnection::systemBus());

    QDBusMessage message;
    // 删除背景，模糊背景单独处理
    QString procMethod;
    if (s_ModelBlurBkg == model) {
        procMethod = s_EnhanceBlurBkg;
        message = interface.call(s_EnhanceBlurBkg, source, output);
    } else if (s_ModelBkgCut == model) {
        procMethod = s_EnhancePortraitCout;
        message = interface.call(s_EnhancePortraitCout, source, output);
    } else {
        procMethod = s_EnhanceProcMethod;
        message = interface.call(s_EnhanceProcMethod, source, output, model);
    }

    QDBusError error = interface.lastError();
    if (QDBusError::NoError != error.type()) {
        qWarning() << QString("[Enhance DBus] DBus %1 call %2 error: type(%2) [%3] %4")
                          .arg(s_EnhanceService)
                          .arg(procMethod)
                          .arg(error.type())
                          .arg(error.name())
                          .arg(error.message());

    } else {
        QDBusReply<QVariant> reply(message);
        bool ret = reply.value().toBool();

        if (!ret) {
            qWarning() << QString("[Enhance DBus] Call %1 error: value(%2)").arg(procMethod).arg(ret);
        }
        return ret;
    }

    return false;
}

/**
   @brief 接收DBus接口处理完成的信号，\a output 是输出的文件路径。
 */
void AIModelService::onDBusEnhanceEnd(const QString &output, int error)
{
    // 多实例，可能传入其它实例的任务
    EnhancePtr ptr = dptr->enhanceCache.value(output);
    if (ptr.isNull()) {
        return;
    }
    qInfo() << QString("Receive DBus enhance result: %1 (%2)").arg(output).arg(error);

    // 只允许最新的图片更新
    if ((ptr->index != dptr->enhanceCache.size() - 1) && (output == dptr->lastOutput)) {
        return;
    }

    State state = static_cast<State>(ptr->state.loadAcquire());
    if (Cancel == state || LoadTimeout == state) {
        // 处理终止，不继续处理
        return;
    } else if (Loading != state) {
        qWarning() << qPrintable("[Enhance DBus] Reentrant enhance image process! ") << output << state;
    }

    // 判断接口反馈错误
    switch (error) {
        case AIModelServiceData::DBusNoError: {
            // 判断文件是否创建成功
            if (!QFile::exists(output)) {
                qWarning() << qPrintable("[Enhance DBus] Create enhance image failed! ") << output;
                state = LoadFailed;
            } else {
                state = LoadSucc;
            }
            break;
        }
        case AIModelServiceData::DBusNoPortrait:
            state = NotDetectPortrait;
            break;
        default:
            state = LoadFailed;
            break;
    }

    ptr->state.storeRelease(state);

    Q_EMIT enhanceEnd(ptr->source, output, state);
}
