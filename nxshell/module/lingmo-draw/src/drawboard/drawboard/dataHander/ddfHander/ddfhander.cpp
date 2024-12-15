// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfhander.h"
#include "sitemdata.h"
#include "pagecontext.h"
#include "pagescene.h"
//#include "cviewmanagement.h"
#include "filewatcher.h"
#include "ddfProccessor_5_9_9_0.h"
#include "ddfproccessordrawboard.h"

#include <QFileInfo>
#include <QVariant>
#include <QMap>
#include <QSharedPointer>
#include <QApplication>
class DdfProcesserManager
{
public:
    DdfProcesserManager() {}
    ~DdfProcesserManager()
    {
        foreach (auto processr, installedProcessors) {
            processr->deleteLater();
        }
        installedProcessors.clear();
    }
    QMap<int, DdfUnitProccessor *> installedProcessors;
};
class DdfHander::DdfHander_private
{
public:
    explicit DdfHander_private(DdfHander *ddfHander): hander(ddfHander)
    {
    }

    PageContext *context = nullptr;
    DdfHander *hander;

    int ddfFileVersion = -1;

    QString file;

    static DdfProcesserManager ddfProcessorManager;

    DdfUnitProccessor *saveProcessor    = nullptr;
    DdfUnitProccessor *loadProcessor    = nullptr;
    DdfUnitProccessor *runningProcessor = nullptr;
};
DdfProcesserManager DdfHander::DdfHander_private::ddfProcessorManager = DdfProcesserManager();

DdfHander::DdfHander(QObject *parent): DataHander(parent), DdfHander_d(new DdfHander_private(this))
{
}

DdfHander::DdfHander(const QString &file, QObject *parent): DdfHander(parent)
{
    d_DdfHander()->file = file;
}

DdfHander::DdfHander(const QString &file, PageContext *context, QObject *parent): DdfHander(file, parent)
{
    setContext(context);
}

DdfHander::~DdfHander()
{
}

void DdfHander::setSaveProcessor(DdfUnitProccessor *processor)
{
    d_DdfHander()->saveProcessor = processor;
}

DdfUnitProccessor *DdfHander::saveProcessor() const
{
    return d_DdfHander()->saveProcessor;
}

void DdfHander::setLoadProcessor(DdfUnitProccessor *processor)
{
    d_DdfHander()->loadProcessor = processor;
}

DdfUnitProccessor *DdfHander::loadProcessor() const
{
    return d_DdfHander()->loadProcessor;
}

bool DdfHander::checkFileBeforeSave(const QString &file)
{
    if (context() == nullptr) {
        setError(EDdfNotFoundContextToSave, "there is no context to save.");
        return false;
    }
    return DataHander::checkFileBeforeSave(file);
}

bool DdfHander::checkFileBeforeLoad(const QString &file)
{
    if (context() == nullptr) {
        setError(EDdfLoadNeedOneContext, "before load, you should set one context to load data(use setContext).");
        return false;
    }
    bool checkLoad = DataHander::checkFileBeforeLoad(file);
    if(checkLoad){
        checkLoad = checkMd5Valid(file);
    }
    return checkLoad;
}

void DdfHander::installProcessor(DdfUnitProccessor *processor)
{
    DdfHander_private::ddfProcessorManager.installedProcessors.insert(processor->version(), processor);
}

void DdfHander::removeProcessor(DdfUnitProccessor *processor)
{
    DdfHander_private::ddfProcessorManager.installedProcessors.remove(processor->version());
}

void DdfHander::initProcessors()
{
    installProcessor(new DdfUnitProccessor_5_8_0_Compatibel());
    installProcessor(new DdfUnitProccessor_5_8_0_10_1());
    installProcessor(new DdfUnitProccessor_5_8_0_10_2());
    installProcessor(new DdfUnitProccessor_5_8_0_16_1());
    installProcessor(new DdfUnitProccessor_5_8_0_20());
    installProcessor(new DdfUnitProccessor_5_8_0_48());
    installProcessor(new DdfUnitProccessor_5_8_0_84());
    installProcessor(new DdfUnitProccessor_5_8_0_9());
    installProcessor(new DdfUnitProccessor_5_9_0_1());
    installProcessor(new DdfUnitProccessor_5_9_0_3_Compatibel());
    installProcessor(new DdfUnitProccessor_5_9_9_0());
    installProcessor(new DdfUnitProccessor_5_10_0());
    installProcessor(new DdfProccessorDrawBoard);
}

void DdfHander::clearProcessors()
{
    foreach (auto processr, DdfHander_private::ddfProcessorManager.installedProcessors) {
        processr->deleteLater();
    }
    DdfHander_private::ddfProcessorManager.installedProcessors.clear();
}

DdfUnitProccessor *DdfHander::newerProcessor()
{
    if (DdfHander_private::ddfProcessorManager.installedProcessors.isEmpty()) {
        initProcessors();
    }
    if (!DdfHander_private::ddfProcessorManager.installedProcessors.isEmpty()) {
        return DdfHander_private::ddfProcessorManager.installedProcessors.last();
    }
    return nullptr;
}

DdfUnitProccessor *DdfHander::processor(int version)
{
    if (DdfHander_private::ddfProcessorManager.installedProcessors.isEmpty()) {
        initProcessors();
    }
    auto itf = DdfHander_private::ddfProcessorManager.installedProcessors.find(version);
    if (itf != DdfHander_private::ddfProcessorManager.installedProcessors.end()) {
        return itf.value();
    }
    return nullptr;
}

DdfUnitProccessor *DdfHander::processor(const QString &ddfFile)
{
    return processor(DdfUnitProccessor::getDdfVersion(ddfFile));
}

extern void PageContext_setContextFilePath(PageContext *context, const QString &file);
PageContext *DdfHander::load(const QString &file)
{
    DdfHander hander(file, new PageContext);
    if (!hander.load()) {
        hander.context()->deleteLater();
        hander.setContext(nullptr);
    }
    return hander.context();
}

bool DdfHander::save(PageContext *context, const QString &file)
{
    DdfHander hander(file, context);
    return hander.save();
}

QString DdfHander::file() const
{
    return d_DdfHander()->file;
}

void DdfHander::setFile(const QString &file)
{
    d_DdfHander()->file = file;
}

PageContext *DdfHander::context() const
{
    return d_DdfHander()->context;
}

void DdfHander::setContext(PageContext *context)
{
    d_DdfHander()->context = context;
}

bool DdfHander::load()
{
    PageContext *context = this->context();
    if (context == nullptr)
        return false;
    if (file().isEmpty())
        return false;

    bool loadsuccess = false;
    unsetError();
    emit progressBegin(tr("Opening..."));
    if (checkFileBeforeLoad(file())) {
        auto processor = loadProcessor() != nullptr ? loadProcessor() : DdfHander::processor(file());
        if (processor != nullptr) {
            d_DdfHander()->ddfFileVersion = DdfUnitProccessor::getDdfVersion(file());
            processor->initForHander(this);
            PageContextData data = processor->load(file(), this);
            if (error() == NoError) {
                //context->setFile(this->file());
                PageContext_setContextFilePath(context, file());
                //打开文件视为未修改数据
                context->setDirty(false);
                context->moveToThread(qApp->thread());
                QMetaObject::invokeMethod(context, [ = ]() {
                    processor->pageContextLoadData(context, data);
                }, Qt::QueuedConnection);
                loadsuccess = true;
            }
            processor->clearForHander(this);
        }
    }
    emit progressEnd(error(), errorString());
    return loadsuccess;
}

bool DdfHander::save()
{
    if (file().isEmpty())
        return false;
    if (context() == nullptr) {
        return false;
    }
    emit progressBegin(tr("Saving..."));
    unsetError();
    bool  ret  = false;
    if (checkFileBeforeSave(file())) {
        auto processor = saveProcessor() != nullptr ? saveProcessor() : DdfHander::newerProcessor();
        if (processor != nullptr) {
            context()->page()->borad()->fileWatcher()->removePath(file());
            //d_pri()->file = file;
            //d_pri()->context = context;
            d_DdfHander()->ddfFileVersion = processor->version();
            processor->initForHander(this);
            ret = processor->save(context(), file(), this);
            processor->clearForHander(this);

            if (error() == 0) {
                //context()->setFile(file());
                PageContext_setContextFilePath(context(), file());
                context()->setDirty(false);
            }

            context()->page()->borad()->fileWatcher()->addWather(file());
        }
    } else {
        messageUnSupport(file());
    }
    emit progressEnd(error(), errorString());
    return ret;
}

int DdfHander::ddfVersion() const
{
    return d_DdfHander()->ddfFileVersion;
}

bool DdfHander::isSupportedFile(const QString &file)
{
    QFileInfo info(file);
    if (info.suffix().toLower() == "ddf") {
        return true;
    }
    return false;
}
bool DdfHander::checkMd5Valid(const QString &file_path)
{
    if (isDdfFileDirty(file_path)) {
        messageMd5valid(file_path);
        setError(EMd5Valid, tr("Unable to open the broken file"));
        return false;
    }
    return true;
}

bool DdfHander::isDdfFileDirty(const QString &file_path)
{
    QFile file(file_path);
    if (file.exists()) {
        if (file.open(QFile::ReadOnly)) {
            QDataStream s(&file);
            // 先通过版本号判断是否ddf拥有md5校验值
            int ver = DdfUnitProccessor::getDdfVersion(file_path);;
            if (ver >= EDdf5_8_0_20) {
                QByteArray allBins = file.readAll();
                QByteArray md5    = allBins.right(16);

                qDebug() << "load  head+bytes = " << (allBins.count() - md5.count()) << "md5 count = " << md5.count();
                qDebug() << "direct read MD5 form ddffile file = " << file_path << " MD5 = " << md5.toHex().toUpper();

                QByteArray contex = allBins.left(allBins.size() - md5.size());

                QByteArray nMd5 = QCryptographicHash::hash(contex, QCryptographicHash::Md5);

                qDebug() << "recalculate MD5 form ddffile file = " << file_path << " MD5 = " << nMd5.toHex().toUpper();

                if (md5 != nMd5) {
                    return true;
                }
            } else  if (ver == EDdfUnknowed) {
                return true;
            }
            return false;
        }
    }
    return true;
}
//bool DdfHander::save(const QString &file,
//                     const QVariant &saveObj,
//                     const QList<QVariant> &params)
//{
//    Q_UNUSED(params)
//    return save(saveObj.value<PageContext *>(), file);
//}

//QVariant DdfHander::load(const QString &file,
//                         const QList<QVariant> &params)
//{
//    Q_UNUSED(params)
//    PageContext *contex = load(file);
//    QVariant var;
//    var.setValue<PageContext *>(contex);
//    return var;
//}













