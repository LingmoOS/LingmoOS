// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DataHanderInterface.h"
#include "pagecontext.h"
#include "pagescene.h"
#include "ddfproccessor.h"
#include "ddfhander.h"
#include "filewatcher.h"

#include <QFile>
#include <QDataStream>
#include <QtConcurrent>
#include <QImageReader>
#include <QPdfWriter>
#include <QImageWriter>
#include <QPainter>

class FileHander::FileHander_private
{
public:
    explicit FileHander_private(FileHander *hander): _hander(hander) {}
    ~FileHander_private()
    {
    }

    void unsetError()
    {
        lastError = 0;
        lastErrorDescribe = "";
    }
    void setError(int error, const QString &errorString = "")
    {
        lastError = error;
        lastErrorDescribe = errorString;
    }

    FileHander *_hander;


    DdfHander *_ddfHander = nullptr;
    ImageHander *_imgHander = nullptr;

    int     lastError = 0;
    QString lastErrorDescribe;

    friend class FileHander;
};

/********************************************  LOAD STATIC FUNCTIONS ********************************************/
//keep compatibility
//static int loadDdfWithNoCombinGroup(const QString &path, PageContext *contex, FileHander *hander)
//{
//    QString error;
//    int ret = FileHander::NoError;
//    QFile readFile(path);
//    if (readFile.open(QIODevice::ReadOnly)) {
//        QDataStream in(&readFile);

//        CGraphics head;
//        in >> head;
//        qDebug() << QString("load ddf(%1)").arg(path) << " ddf version = " << head.version << "graphics count = " << head.unitCount <<
//                 "scene size = " << head.rect;

//        bool firstBlurFlag = true;
//        bool firstDrawPen = true;
//        for (int i = 0; i < head.unitCount; i++) {
//            Unit unit;
//            unit.reson = ESaveToDDf;
//            in >> unit;

//            if (TextType == unit.head.dataType) {
//                contex->addSceneItem(unit);
//            } else {
//                //如果有老的模糊图元 那么进行交互提示模糊图元会被丢失，或者放弃加载
//                bool foundBlur = (unit.head.dataType == BlurType);
//                if (foundBlur && firstBlurFlag) {
//                    firstBlurFlag = false;
//                    bool finished = false;
//                    int anwser = DrawBoard::exeMessage(QObject::tr("The blur effect will be lost as the file is in old version. Proceed to open it?"),
//                                                       DrawBoard::EWarningMsg, false, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
//                                                       QList<int>() << 1 << 0);
//                    if (anwser == 1 || anwser == -1) {
//                        finished = true;
//                    }
//                    if (finished) {
//                        unit.release();
//                        readFile.close();
//                        ret = FileHander::EUserCancelLoad_OldBlur;
//                        error = "user cancel old blur unit.";
//                        goto END;
//                    }
//                }

//                bool foundDpen = (unit.head.dataType == PenType);
//                if (foundDpen && firstDrawPen) {
//                    firstDrawPen = false;
//                    bool finished = false;
//                    int ret = DrawBoard::exeMessage(QObject::tr("The file is in an older version, and the properties of elements will be changed. " \
//                                                                "Proceed to open it?"),
//                                                    DrawBoard::EWarningMsg, false, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
//                                                    QList<int>() << 1 << 0);
//                    if (ret == 1 || ret == -1) {
//                        finished = true;
//                    }
//                    if (finished) {
//                        unit.release();
//                        readFile.close();
//                        ret = FileHander::EUserCancelLoad_OldPen;
//                        error = "user cancel old pen unit.";
//                        goto END;
//                    } else {
//                        if (!(unit.head.pen.width() > 0)) {
//                            unit.release();
//                            emit hander->progressChanged(i + 1, head.unitCount, "");
//                            continue;
//                        }
//                    }
//                }
//                //EDdf5_9_0_3_LATER之后的版本不再存在模糊图元,所以如果加载EDdf5_9_0_3_LATER之前的ddf文件那么不用加载模糊
//                if (unit.head.dataType != BlurType) {
//                    contex->addSceneItem(unit);
//                } else {
//                    unit.release();
//                }
//            }
//            qApp->processEvents();
//            emit hander->progressChanged(i + 1, head.unitCount, "");
//        }
//        in >> head.version;
//        readFile.close();
//    } else {
//        ret = readFile.error();
//        error = readFile.errorString();
//    }
//END:
//    if (ret == FileHander::NoError)
//        contex->setFile(path);

//    hander->d_pri()->setError(ret, error);

//    return ret;
//}

//UnitTree deserializationToTree_helper(QDataStream &inStream, int &outBzItemCount, int &outGroupCount, FileHander *hander,
//                                      bool &bcomplete, bool &firstMeetPen, std::function<void(int, int)> f = nullptr)
//{
//    UnitTree result;
//    int groupCount = 0;
//    inStream >> groupCount;
//    for (int i = 0; i < groupCount; ++i) {
//        UnitTree child = deserializationToTree_helper(inStream, outBzItemCount, outGroupCount, hander, bcomplete, firstMeetPen, f);
//        result.childTrees.append(child);
//    }
//    int bzItemCount = 0;
//    inStream >> bzItemCount;
//    for (int i = 0; i < bzItemCount; ++i) {
//        Unit unit;
//        unit.reson = ESaveToDDf;
//        inStream >> unit;

//        if (unit.head.dataType == PenType) {
//            if (firstMeetPen) {
//                firstMeetPen = false;
//                bool finished = false;

//                int ret = DrawBoard::exeMessage(QObject::tr("The file is in an older version, and the properties of elements will be changed. " \
//                                                            "Proceed to open it?"),
//                                                DrawBoard::EWarningMsg, false, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
//                                                QList<int>() << 1 << 0);
//                if (ret == 1) {
//                    finished = true;
//                }
//                if (finished) {
//                    bcomplete = false;
//                    hander->d_pri()->lastError = FileHander::EUserCancelLoad_OldPen;
//                    hander->d_pri()->lastErrorDescribe = "user cancel old pen unit.";
//                    unit.release();
//                    return UnitTree();
//                }
//            }

//            if (!(unit.head.pen.width() > 0)) {
//                ++outBzItemCount;
//                if (f != nullptr) {
//                    f(outBzItemCount, outGroupCount);
//                }
//                continue;
//            }
//        }
//        result.bzItems.append(unit);
//        ++outBzItemCount;
//        if (f != nullptr) {
//            f(outBzItemCount, outGroupCount);
//        }
//    }
//    Unit unit;
//    unit.reson = ESaveToDDf;
//    inStream >> unit;
//    //qDebug() << "read group info   = " << unit.head.dataType;
//    result.treeUnit = unit;
//    ++outGroupCount;
//    if (f != nullptr) {
//        f(outBzItemCount, outGroupCount);
//    }
//    return result;
//}
//static int loadDdfWithCombinGroup(const QString &path, PageContext *contex, FileHander *hander)
//{
//    int ret = FileHander::NoError;
//    QFile readFile(path);
//    if (readFile.open(QIODevice::ReadOnly)) {
//        QDataStream in(&readFile);

//        //1.反序列化到头结构中
//        CGraphics head;
//        in >> head;
//        qint64 totalItemsCountHead = head.unitCount;
//        //2.设置当前场景的大小
//        contex->setPageRect(head.rect);

//        //3.反序列化生成图元结构树,同时获取基本图元和组合图元的个数
//        int bzItemsCount   = 0;
//        int groupItemCount = 0;
//        bool bcomplete = true;
//        std::function<void(int, int)> fProcess = [ = ](int bzCount, int gpCount) -> void {
//            if (totalItemsCountHead != 0)
//            {
//                qApp->processEvents();
//                emit hander->progressChanged(bzCount + gpCount, totalItemsCountHead, "");
//            }
//        };
//        bool firstMeetPen = true;
//        UnitTree tree = deserializationToTree_helper(in, bzItemsCount,
//                                                     groupItemCount, hander,
//                                                     bcomplete, firstMeetPen,
//                                                     fProcess);
//        if (!bcomplete) {
//            //选择不使用老版本
//            readFile.close();
//            ret = FileHander::EUserCancelLoad_OldPen;
//            return ret;
//        }
//        int totalItemsCount = bzItemsCount + groupItemCount;
//        if (totalItemsCountHead != totalItemsCount) {
//            qWarning() << "get total items count from ddf head that = " << totalItemsCountHead;
//            qWarning() << "get total items count from ddf deserialization items that = " << totalItemsCount;
//            qWarning() << "items count from size not match deserialization items count !!! now we use the latter.";
//        }

//        //4.加载图元结构树
//        {
//            auto scene = contex->scene();
//            //禁止选中和自动赋予z值的操作(z值可以通过数据加载确定)
//            scene->blockAssignZValue(true);
//            scene->blockSelect(true);

//            CGraphicsItemGroup *pGroup = scene->loadGroupTreeInfo(tree);
//            scene->cancelGroup(pGroup, false);
//            delete pGroup;

//            scene->blockSelect(false);
//            scene->blockAssignZValue(false);

//        }

//        //5.关闭文件结束
//        readFile.close();
//    } else {
//        ret = readFile.error();
//        hander->d_pri()->setError(ret, readFile.errorString());
//    }
//    contex->setFile(path);
//    return ret;
//}

QImage loadImage_helper(const QString &path, FileHander *hander)
{
    auto fOptimalConditions = [ = ](const QImageReader & reader) {
        QSize orgSz = reader.size();
        return (orgSz.width() > 1920 && orgSz.height() > 1080);
    };

    QImageReader reader;
    reader.setFileName(path);
    bool shouldOptimal = fOptimalConditions(reader);
    qreal radio = shouldOptimal ? 0.5 : 1.0;
    QSize orgSize = reader.size();
    QSize newSize = orgSize * radio;
    reader.setScaledSize(newSize);

    if (shouldOptimal)
        reader.setQuality(30);

    //当不能读取数据时,设置以内容判断格式进行加载,这样可以避免后缀名修改后文件无法打开的情况
    if (!reader.canRead()) {
        reader.setAutoDetectImageFormat(true);
        reader.setDecideFormatFromContent(true);
        reader.setFileName(path); //必须重新设置一下文件,才能触发内部加载方式的切换
    }

    if (reader.canRead()) {
        QImage img = reader.read();
        //维持原大小
        bool haveOptimal = shouldOptimal;
        if (haveOptimal) {
            img = img.scaled(orgSize);
        }
        return img;
    }
    return QImage();
}
/********************************************  LOAD  FUNCTIONS ********************************************/


/********************************************  SAVE STATIC FUNCTIONS ********************************************/
//struct STreePlatInfo {
//    QByteArray headBytes;
//    QByteArray itemsBytes;
//    QByteArray md5;
//    int bzItemCount = 0;
//    int groupCount = 0;
//};
//void serializationTree_helper_1(QDataStream &countStream, int &bzItemCount, int &groupCount,
//                                const UnitTree &tree, std::function<void(int, int)> f = nullptr)
//{
//    countStream << tree.childTrees.size();

//    qDebug() << "save group count  = " << tree.childTrees.size();

//    foreach (auto p, tree.childTrees) {
//        qApp->processEvents();
//        serializationTree_helper_1(countStream, bzItemCount, groupCount, p, f);
//    }

//    countStream << tree.bzItems.size();

//    qDebug() << "save item count   = " << tree.bzItems.size();

//    foreach (auto i, tree.bzItems) {
//        qApp->processEvents();
//        countStream << i;
//        qDebug() << "save item info    = " << i.head.dataType;

//        ++bzItemCount;
//        if (f != nullptr) {
//            f(bzItemCount, groupCount);
//        }
//    }

//    //bzItemCount += tree.bzItems.size();

//    countStream << tree.treeUnit;

//    qDebug() << "save group info   = " << tree.treeUnit.head.dataType;

//    ++groupCount;
//    if (f != nullptr) {
//        f(bzItemCount, groupCount);
//    }
//}
//STreePlatInfo serializationTreeToBytes_1(const UnitTree &tree)
//{
//    STreePlatInfo info;

//    QDataStream streamForCountBytes(&info.itemsBytes, QIODevice::WriteOnly);
//    serializationTree_helper_1(streamForCountBytes, info.bzItemCount, info.groupCount, tree);

//    return info;
//}

//QByteArray serializationHeadToBytes(const CGraphics &head)
//{
//    QByteArray  array;
//    QDataStream streamForCountBytes(&array, QIODevice::WriteOnly);
//    streamForCountBytes << head;

//    return array;
//}
//int saveDdfWithCombinGroup(const QString &path, PageContext *contex, FileHander *hander)
//{
//    QString errorString;
//    int error = FileHander::NoError;

//    auto pDrawScen = contex->scene();

//    UnitTree treeInfo = pDrawScen->getGroupTreeInfo(nullptr, ESaveToDDf, true);

//    //0.开始之前设置当前要保存的文件名,并去掉针对该文件的监视(自身程序修改该文件不需要提示文件被修改了)

//    //1.将treeInfo进行序列化到内存这个过程很快(很多图片的时候内存是否足够?)
//    STreePlatInfo info = serializationTreeToBytes_1(treeInfo);

//    //2.初始化文件的头信息(主要包含了ddf版本号,总的图元个数,场景的大小)并序列化到内存
//    CGraphics head;
//    head.version   = qint32(EDdfCurVersion);
//    head.unitCount = info.groupCount + info.bzItemCount;         //在加载时可快速获知图元总数
//    head.rect      = pDrawScen->sceneRect();                     //场景的大小
//    info.headBytes = serializationHeadToBytes(head);

//    //3.将树结构的二进制流数据进行md5加密
//    info.md5 = QCryptographicHash::hash(info.headBytes + info.itemsBytes, QCryptographicHash::Md5);

//    qDebug() << "save head size = " << info.headBytes.count() << "byte size = " << info.itemsBytes.count() << "md5 count = " << info.md5.count();

//    //4.再进行磁盘检查(判断目标文件所处的磁盘分区空间是否足够),如果未通过,交互提示返回
//    if (!hander->isVolumeSpaceAvailabel(path, info.itemsBytes.size() + info.md5.size())) {
//        qWarning() << "volum space is not enough !!!";
//        errorString = QObject::tr("Unable to save. There is not enough disk space.");
//        error = FileHander::EInsufficientPartitionSpace;
//    } else {
//        //5.将数据保存到文件
//        QFile writeFile(path);
//        if (writeFile.open(QIODevice::WriteOnly/* | QIODevice::ReadOnly | QIODevice::Truncate*/)) {
//            QDataStream out(&writeFile);

//            //5.1 首先保存沿用的老的头信息(主要包含了ddf版本号,总的图元个数,场景的大小)
//            int resultByteSize = out.writeRawData(info.headBytes.data(), info.headBytes.size());
//            qDebug() << "write heads bytes: " << info.headBytes.size() << "success byteSize: " << resultByteSize;

//            //5.2 写入图元的数据(安排一下进度?)
//            int granularityByteSize = 200; //设置字节粒度
//            resultByteSize = 0;
//            while (resultByteSize < info.itemsBytes.size()) {
//                qApp->processEvents();
//                QByteArray bytes = info.itemsBytes.mid(resultByteSize, granularityByteSize);
//                resultByteSize += out.writeRawData(bytes.data(), bytes.size());
//                emit hander->progressChanged(resultByteSize, info.itemsBytes.size(), "");
//            }
//            qDebug() << "write items bytes: " << info.itemsBytes.size() << "success byteSize: " << resultByteSize;

//            //5.3 写入md5数据
//            resultByteSize = out.writeRawData(info.md5.data(), info.md5.size());
//            qDebug() << "write  md5  bytes: " << info.md5.size() << "success byteSize: " << resultByteSize << "md5 = " << info.md5.toHex().toUpper();

//            //5.4 关闭文件(执行后才会真的写入到磁盘)
//            writeFile.close();

//            if (writeFile.error() != QFileDevice::NoError) {
//                error = writeFile.error();
//                errorString = writeFile.errorString();
//            } else {
//                contex->setFile(path);
//                contex->setDirty(false);
//            }
//            qDebug() << "save ddf end---------------- " << writeFile.error();
//        } else {
//            error = writeFile.error();
//            qWarning() << "PermissionsError ==== " << error << QFile::PermissionsError;
//            errorString = writeFile.errorString();
//        }
//    }
//    hander->d_pri()->setError(error, errorString);
//    return error;
//}

/********************************************  SAVE STATIC FUNCTIONS ********************************************/



FileHander::FileHander(QObject *parent): QObject(parent), FileHander_d(new FileHander_private(this))
{
}

FileHander::~FileHander()
{
}
QStringList FileHander::supPictureSuffix()
{
    static QStringList supPictureSuffixs = QStringList() << "png" << "jpg" << "bmp" << "tif" << "jpeg" ;
    return supPictureSuffixs;
//    QStringList supPictureSuffixs;
//    foreach (auto it, QImageWriter::supportedImageFormats()) {
//        supPictureSuffixs.append(it);
//    }
//    return supPictureSuffixs;
}

QStringList FileHander::supDdfStuffix()
{
    static QStringList supDdfSuffixs = QStringList() << "ddf";
    return supDdfSuffixs;
}

bool FileHander::isFileLoadable(const QString &file)
{
    if (isLegalFile(file)) {
        auto stuff = QFileInfo(file).suffix().toLower();
        if (supDdfStuffix().contains(stuff) || supPictureSuffix().contains(stuff)) {
            return true;
        }
    }
    return false;
}
bool FileHander::isLegalFile(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }

    QRegExp regExp("[:\\*\\?\"<>\\|]");

    if (path.contains(regExp)) {
        return false;
    }

    QRegExp splitExp("[/\\\\]");


    int pos = splitExp.indexIn(path, 0);

    while (pos != -1) {
        QString dirStr = path.left(pos + 1);
        if (dirStr.count() > 1) {
            QDir dir(dirStr);
            if (!dir.exists()) {
                return false;
            }
        }
        pos = splitExp.indexIn(path, pos + 1);
    }

    bool isdir = (path.endsWith('/') || path.endsWith('\\'));
    return !isdir;
}

QString FileHander::toLegalFile(const QString &filePath)
{
    QString result = filePath;
    QFileInfo info(filePath);
    if (!info.exists()) {
        QUrl url(filePath);
        if (url.isLocalFile()) {
            result = url.toLocalFile();
        }
    }

    if (!isLegalFile(result))
        return "";

    return result;
}

PageContext *FileHander::loadDdf(const QString &file)
{
    d_FileHander()->unsetError();

    emit progressBegin(tr("Opening..."));

    PageContext *result = nullptr;
    if (checkFileBeforeLoad(file, true)) {
        auto legalPath = toLegalFile(file);
        DdfHander ddfHander(this);
        connect(&ddfHander, &DdfHander::message_waitAnswer, this, [ = ](const SMessage & message, int &retureRet) {
            retureRet = MessageDlg::execMessage(message);
        });
        connect(&ddfHander, &DdfHander::progressChanged, this, [ = ](int progress, int total, const QString & describe) {
            qApp->processEvents();
            emit progressChanged(progress, total, describe);
        });
        result = ddfHander.load(legalPath);
        d_FileHander()->setError(ddfHander.error(), ddfHander.errorString());
    }
    emit progressEnd(d_FileHander()->lastError, d_FileHander()->lastErrorDescribe);
    return result;
}

bool FileHander::saveToDdf(PageContext *context, const QString &file)
{
    d_FileHander()->unsetError();

    emit progressBegin(tr("Saving..."));

    auto filePath = file.isEmpty() ? context->file() : file;
    if (checkFileBeforeSave(filePath, true)) {
        filePath = toLegalFile(filePath);

        if (context->page() != nullptr)
            context->page()->borad()->fileWatcher()->removePath(filePath);

        DdfHander ddfHander(this);
        connect(&ddfHander, &DdfHander::progressChanged, this, [ = ](int progress, int total, const QString & describe) {
            qApp->processEvents();
            emit progressChanged(progress, total, describe);
        });
        ddfHander.save(context, filePath);
        d_FileHander()->setError(ddfHander.error(), ddfHander.errorString());

        if (context->page() != nullptr)
            context->page()->borad()->fileWatcher()->addWather(filePath);
    }

    emit progressEnd(d_FileHander()->lastError, d_FileHander()->lastErrorDescribe);

    return  d_FileHander()->lastError == NoError;
}

QImage FileHander::loadImage(const QString &file)
{
    d_FileHander()->unsetError();
    if (checkFileBeforeLoad(file, false)) {
        auto legalPath = toLegalFile(file);
        QImage img = loadImage_helper(legalPath, this);
        if (img.isNull()) {
            d_FileHander()->setError(EDamagedImageFile, tr("Damaged file, unable to open it"));
        }
        return img;
    }
    return QImage();
}

bool FileHander::saveToImage(PageContext *context,
                             const QString &file,
                             const QSize &desImageSize,
                             int imageQuility)
{
    d_FileHander()->unsetError();

    if (checkFileBeforeSave(file, false)) {

        auto image = context->renderToImage(Qt::transparent, desImageSize);

        QFileInfo info(file);
        auto stuff = info.suffix().toLower();

        if (stuff.toLower() == "pdf") {
            QPdfWriter writer(file);
            int ww = image.width();
            int wh = image.height();
            writer.setResolution(96);
            writer.setPageSizeMM(QSizeF(25.4 * ww / 96, 25.4 * wh / 96));
            QPainter painter(&writer);
            painter.drawImage(0, 0, image);
            return true;
        }
        return image.save(file, stuff.toLocal8Bit(), imageQuility);
    }
    return false;
}

QString FileHander::lastErrorDescribe() const
{
    return d_FileHander()->lastErrorDescribe;
}

int FileHander::lastError() const
{
    return d_FileHander()->lastError;
}

bool FileHander::checkFileBeforeLoad(const QString &file, bool isDdf)
{
    //1 check if name illegal.
    auto legalPath = toLegalFile(file);
    if (legalPath.isEmpty()) {
        d_FileHander()->setError(EFileNameIllegal, "EFileNameIllegal");
        return false;
    }

    //2 check if file exist.
    if (!checkFileExist(legalPath)) {
        return false;
    }

    //3 check if file readable.
    if (!checkFileReadable(legalPath)) {
        return false;
    }

    //4 check if file be supported.
    QFileInfo info(legalPath);
    auto stuff = info.suffix().toLower();
    QStringList list = isDdf ? supDdfStuffix() : supPictureSuffix();
    if (!list.contains(stuff)) {
        d_FileHander()->setError(EUnSupportFile, tr("Unable to open \"%1\", unsupported file format").arg(info.fileName()));
        return false;
    }

    if (isDdf) {
        //5 check if ddf file version right.
        if (!checkDdfVersionIllegal(legalPath)) {
            return false;
        }

        //6 check if ddf file md5 right.
        if (!checkDdfMd5(legalPath)) {
            return false;
        }
    }

    return true;
}

bool FileHander::checkFileBeforeSave(const QString &file, bool toDdf)
{
    Q_UNUSED(toDdf);
    auto fileLocal = toLegalFile(file);
    //0.check file if lege
    if (fileLocal.isEmpty()) {
        d_FileHander()->setError(EFileNameIllegal, "EFileNameIllegal");
        return false;
    }

    //1 check if writable.
    if (!checkFileWritable(fileLocal)) {
        return false;
    }
    return true;
}
bool FileHander::isVolumeSpaceAvailabel(const QString &desFile, const int needBytesSize)
{
    /* 如果空间不足那么提示 */
    QString dirPath = QFileInfo(desFile).absolutePath();
    QStorageInfo volume(dirPath);
    //QList<QStorageInfo> storageList = QStorageInfo::mountedVolumes();
    if (volume.isValid()) {
        qint64 availabelCount = volume.bytesAvailable();
        qint64 bytesFree      = volume.bytesFree() ;
        qDebug() << "availabelCount = " << availabelCount << "bytesFree = " << bytesFree;
        if (!volume.isReady() || volume.isReadOnly() || availabelCount < needBytesSize) {
            return false;
        }
    }
    return true;
}

int FileHander::getDdfVersion(const QString &file) const
{
    return DdfUnitProccessor::getDdfVersion(file);
}
bool FileHander::isDdfFileDirty(const QString &filePath)const
{
    // 先通过版本号判断是否ddf拥有md5校验值
    int ver = getDdfVersion(filePath);
    if (ver >= EDdf5_8_0_20) {
        QFile file(filePath);
        if (file.exists()) {
            if (file.open(QFile::ReadOnly)) {
                QByteArray allBins = file.readAll();
                QByteArray md5    = allBins.right(16);

                qDebug() << "load  head+bytes = " << (allBins.count() - md5.count()) << "md5 count = " << md5.count();
                qDebug() << "direct read MD5 form ddffile file = " << filePath << " MD5 = " << md5.toHex().toUpper();

                QByteArray contex = allBins.left(allBins.size() - md5.size());

                QByteArray nMd5 = QCryptographicHash::hash(contex, QCryptographicHash::Md5);

                qDebug() << "recalculate MD5 form ddffile file = " << filePath << " MD5 = " << nMd5.toHex().toUpper();

                return (md5 != nMd5);
            }
        }
    } else  if (ver == EDdfUnknowed) {
        return true;
    } else {
        return false;
    }
    return true;
}

bool FileHander::checkFileExist(const QString &file) const
{
    //1 check if exist.
    QFileInfo info(file);
    if (!info.exists()) {
        d_FileHander()->lastError = EFileNotExist;
        d_FileHander()->lastErrorDescribe = "EFileNotExist";
        return false;
    }
    return true;
}

bool FileHander::checkFileReadable(const QString &file) const
{
    //2 check if readable.
    QFileInfo info(file);
    if (!info.isReadable()) {
        d_FileHander()->setError(EUnReadableFile, tr("Unable to open the write-only file \"%1\"").arg(info.fileName()));
        return false;
    }
    return true;
}

bool FileHander::checkFileWritable(const QString &file) const
{
    QFileInfo info(file);
    //if file not exists,we should check if the dir writable;
    //if file exists,we only check if the file writable.
    if (info.exists()) {
        if (!info.isWritable()) {
            d_FileHander()->setError(EUnWritableFile, tr("This file is read-only, please save with another name"));
            return false;
        }

    } else {
        QString dirPath = info.absolutePath();
        QFileInfo dir(dirPath);
        if (dir.isDir() && !dir.isWritable()) {
            d_FileHander()->setError(EUnWritableDir, tr("This file is read-only, please save with another name"));
            return false;
        }
        return true;
    }
    return true;
}

bool FileHander::checkDdfVersionIllegal(const QString &ddfFile) const
{
    //3.first to check ddf version.
    int ddfVersion = getDdfVersion(ddfFile);
    if (ddfVersion > EDdfCurVersion) {
        qWarning() << "The file is incompatible with the old app, please install the latest version.";


        //文件版本与当前应用不兼容，请安装最新版应用
//        DrawBoard::exeMessage(tr("The file is incompatible with the old app, please install the latest version"),
//                              DrawBoard::EWarningMsg, false);

        d_FileHander()->lastError = EExcessiveDdfVersion;
        d_FileHander()->lastErrorDescribe = tr("The file is incompatible with the old app, please install the latest version");
        return false;

    } else if (ddfVersion < 0) {
        qWarning() << "Cannot open unknown version file!";
        d_FileHander()->lastError = EUnKnowedDdfVersion;
        d_FileHander()->lastErrorDescribe = "unknowedDdfVersion";
        return false;
    }
    return true;
}

bool FileHander::checkDdfMd5(const QString &ddfFile) const
{
    if (isDdfFileDirty(ddfFile)) {
        d_FileHander()->lastError = EDdfFileMD5Error;
        d_FileHander()->lastErrorDescribe = tr("Unable to open the broken file \"%1\"").arg(QFileInfo(ddfFile).fileName());
        return false;
    }
    return true;
}


