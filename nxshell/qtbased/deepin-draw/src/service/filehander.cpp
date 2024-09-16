// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filehander.h"
#include "application.h"
#include "cdrawparamsigleton.h"
#include "cdrawscene.h"
#include "cviewmanagement.h"
#include "dialog.h"
#include "application.h"

#include <QFile>
#include <QDataStream>
#include <QtConcurrent>
#include <QImageReader>
#include <QPdfWriter>
#include <QImageWriter>
#include <QDesktopWidget>

using Future = QFuture<void>;

#define CURRENTTHREADID \
    reinterpret_cast<long long>(QThread::currentThreadId())
class FileHander::FileHander_private
{
public:
    explicit FileHander_private(FileHander *hander): _hander(hander), lastError(0) {}
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

    int     lastError;
    QString lastErrorDescribe;

    friend class FileHander;
};

/********************************************  LOAD STATIC FUNCTIONS ********************************************/
//keep compatibility
static int loadDdfWithNoCombinGroup(const QString &path, PageContext *contex, FileHander *hander)
{
    QString error;
    int ret = FileHander::NoError;
    QFile readFile(path);
    if (readFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&readFile);

        CGraphics head;
        in >> head;
        qDebug() << QString("load ddf(%1)").arg(path) << " ddf version = " << head.version << "graphics count = " << head.unitCount << "scene size = " << head.rect;

        //设置Qdatatream版本
        hander->setQDataStreamVersion(path, in, head);

        bool firstBlurFlag = true;
        bool firstDrawPen = true;
        for (int i = 0; i < head.unitCount; i++) {
            CGraphicsUnit unit;
            unit.reson = ESaveToDDf;
            in >> unit;

            if (TextType == unit.head.dataType) {
                contex->addSceneItem(unit);
            } else {
                //如果有老的模糊图元 那么进行交互提示模糊图元会被丢失，或者放弃加载
                bool foundBlur = (unit.head.dataType == BlurType);
                if (foundBlur && firstBlurFlag) {
                    firstBlurFlag = false;
                    bool finished = false;
//                    int anwser = DrawBoard::exeMessage(QObject::tr("The blur effect will be lost as the file is in old version. Proceed to open it?"),
//                                                       DrawBoard::EWarningMsg, false, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
//                                                       QList<int>() << 1 << 0);
                    int anwser = 1;

                    emit hander->message_waitAnswer(SMessage(QObject::tr("The blur effect will be lost as the file is in old version. Proceed to open it?"),
                                                             ENormalMsg, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
                                                             QList<EButtonType>() << EWarningMsgBtn << ENormalMsgBtn), anwser);
                    if (anwser == 1 || anwser == -1) {
                        finished = true;
                    }
                    if (finished) {
                        unit.release();
                        readFile.close();
                        ret = FileHander::EUserCancelLoad_OldBlur;
                        error = "user cancel old blur unit.";
                        goto END;
                    }
                }

                bool foundDpen = (unit.head.dataType == PenType);
                if (foundDpen) {
                    if (firstDrawPen) {
                        firstDrawPen = false;
                        bool finished = false;
//                        int options = DrawBoard::exeMessage(QObject::tr("The file is in an older version, and the properties of elements will be changed. " \
//                                                                        "Proceed to open it?"),
//                                                            DrawBoard::EWarningMsg, false, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
//                                                            QList<int>() << 1 << 0);
                        int options = 1;
                        emit hander->message_waitAnswer(SMessage(QObject::tr("The file is in an older version, and the properties of elements will be changed. Proceed to open it?"),
                                                                 ENormalMsg, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
                                                                 QList<EButtonType>() << EWarningMsgBtn << ENormalMsgBtn),
                                                        options);
                        if (options == 1 || options == -1) {
                            finished = true;
                        }
                        if (finished) {
                            unit.release();
                            readFile.close();
                            ret = FileHander::EUserCancelLoad_OldPen;
                            error = "user cancel old pen unit.";
                            goto END;
                        }
                    }
                    if (!(unit.head.pen.width() > 0)) {
                        unit.release();
                        emit hander->progressChanged(i + 1, head.unitCount, "");
                        continue;
                    }
                }
                //EDdf5_9_0_3_LATER之后的版本不再存在模糊图元,所以如果加载EDdf5_9_0_3_LATER之前的ddf文件那么不用加载模糊
                if (unit.head.dataType != BlurType) {
                    contex->addSceneItem(unit);
                } else {
                    unit.release();
                }
            }
            qApp->processEvents(QEventLoop::AllEvents, 200);
            emit hander->progressChanged(i + 1, head.unitCount, "");
        }
        in >> head.version;
        readFile.close();
    } else {
        ret = readFile.error();
        error = readFile.errorString();
    }
END:
    if (ret == FileHander::NoError)
        contex->setFile(path);

    hander->d_pri()->setError(ret, error);

    return ret;
}

CGroupBzItemsTreeInfo deserializationToTree_helper(QDataStream &inStream, int &outBzItemCount, int &outGroupCount, FileHander *hander,
                                                   bool &bcomplete, bool &firstMeetPen, std::function<void(int, int)> f = nullptr)
{
    CGroupBzItemsTreeInfo result;
    int groupCount = 0;
    inStream >> groupCount;
    for (int i = 0; i < groupCount; ++i) {
        CGroupBzItemsTreeInfo child = deserializationToTree_helper(inStream, outBzItemCount, outGroupCount, hander, bcomplete, firstMeetPen, f);
        result.childGroups.append(child);
    }
    int bzItemCount = 0;
    inStream >> bzItemCount;
    for (int i = 0; i < bzItemCount; ++i) {
        CGraphicsUnit unit;
        unit.reson = ESaveToDDf;
        inStream >> unit;

        if (unit.head.dataType == PenType) {
            if (firstMeetPen) {
                firstMeetPen = false;
                bool finished = false;

//                int ret = DrawBoard::exeMessage(QObject::tr("The file is in an older version, and the properties of elements will be changed. " \
//                                                            "Proceed to open it?"),
//                                                DrawBoard::EWarningMsg, false, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
//                                                QList<int>() << 1 << 0);
                int returnRet = 1;

                emit hander->message_waitAnswer(SMessage(QObject::tr("The file is in an older version, and the properties of elements will be changed. Proceed to open it?"),
                                                         ENormalMsg, QStringList() << QObject::tr("Open") << QObject::tr("Cancel"),
                                                         QList<EButtonType>() << EWarningMsgBtn << ENormalMsgBtn),
                                                returnRet);
                if (returnRet == 1) {
                    finished = true;
                }
                if (finished) {
                    bcomplete = false;
                    hander->d_pri()->lastError = FileHander::EUserCancelLoad_OldPen;
                    hander->d_pri()->lastErrorDescribe = "user cancel old pen unit.";
                    unit.release();
                    return CGroupBzItemsTreeInfo();
                }
            }

            if (!(unit.head.pen.width() > 0)) {
                ++outBzItemCount;
                if (f != nullptr) {
                    f(outBzItemCount, outGroupCount);
                }
                continue;
            }
        } else if (unit.head.dataType == DyLayer) {
            if (getVersion(inStream) < EDdf5_10_2_7_LATER) {
                //EDdf5_10_2_7_LATER 之前的版本是通过head中的rect和pos进行坐标还原（调用layer的setRect和setPos,都是加载的图像这种图层图元）
                //所以这里通过head中的rect和pos创建一个坐标命令，以保证数据的正确，对于手绘的图层图元，其实就不用生成（即使生成也不影响，大不了多一个坐标的命令）
                if (!unit.data.pDyLayer->baseImg.isNull()) {//非手绘的图层（图像图层）
                    JGeomeCommand *com = new JGeomeCommand(unit.head.pos.toPoint(), 0, unit.head.zValue, unit.head.rect, unit.head.trans);
                    unit.data.pDyLayer->commands.push_front(QSharedPointer<JDyLayerCmdBase>(com));
                }
            }
        }
        result.bzItems.append(unit);
        ++outBzItemCount;
        if (f != nullptr) {
            f(outBzItemCount, outGroupCount);
        }
    }
    CGraphicsUnit unit;
    unit.reson = ESaveToDDf;
    inStream >> unit;
    //qDebug() << "read group info   = " << unit.head.dataType;
    result.data.release();
    result.data = unit;
    ++outGroupCount;
    if (f != nullptr) {
        f(outBzItemCount, outGroupCount);
    }
    return result;
}
static int loadDdfWithCombinGroup(const QString &path, PageContext *contex, FileHander *hander)
{
    int ret = FileHander::NoError;
    QFile readFile(path);
    if (readFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&readFile);

        //1.反序列化到头结构中
        CGraphics head;
        in >> head;
        qint64 totalItemsCountHead = head.unitCount;
        //2.设置当前场景的大小
        contex->setPageRect(head.rect);

        //设置Qdatatream版本
        hander->setQDataStreamVersion(path, in, head);
        //3.反序列化生成图元结构树,同时获取基本图元和组合图元的个数
        int bzItemsCount   = 0;
        int groupItemCount = 0;
        bool bcomplete = true;
        std::function<void(int, int)> fProcess = [ = ](int bzCount, int gpCount) -> void {
            if (totalItemsCountHead != 0)
            {
                qApp->processEvents();
                emit hander->progressChanged(bzCount + gpCount, totalItemsCountHead, "");
            }
        };
        bool firstMeetPen = true;
        CGroupBzItemsTreeInfo tree = deserializationToTree_helper(in, bzItemsCount,
                                                                  groupItemCount, hander,
                                                                  bcomplete, firstMeetPen,
                                                                  fProcess);
        if (!bcomplete) {
            //选择不使用老版本
            readFile.close();
            ret = FileHander::EUserCancelLoad_OldPen;
            return ret;
        }
        int totalItemsCount = bzItemsCount + groupItemCount;
        if (totalItemsCountHead != totalItemsCount) {
            qWarning() << "get total items count from ddf head that = " << totalItemsCountHead;
            qWarning() << "get total items count from ddf deserialization items that = " << totalItemsCount;
            qWarning() << "items count from size not match deserialization items count !!! now we use the latter.";
        }

        //4.加载图元结构树
        QMetaObject::invokeMethod(contex, [ = ]() {
            auto scene = contex->scene();
            //禁止选中和自动赋予z值的操作(z值可以通过数据加载确定)
            scene->blockAssignZValue(true);
            scene->blockSelect(true);

            CGraphicsItemGroup *pGroup = scene->loadGroupTreeInfo(tree);
            scene->cancelGroup(pGroup, false);
            delete pGroup;
            PageScene::releaseBzItemsTreeInfo(tree);

            scene->blockSelect(false);
            scene->blockAssignZValue(false);

        }, Qt::AutoConnection);


        //5.关闭文件结束
        readFile.close();
    } else {
        ret = readFile.error();
        hander->d_pri()->setError(ret, readFile.errorString());
    }
    contex->setFile(path);
    return ret;
}

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
        auto desktop = QApplication::desktop();
        if (Q_NULLPTR != desktop && img.logicalDpiX() != desktop->logicalDpiX()) {//图片Dpi值与屏幕会导致在图片上绘制位置错误
            img.setDotsPerMeterX(qRound(desktop->logicalDpiX() * 100 / 2.54));
            img.setDotsPerMeterY(qRound(desktop->logicalDpiY() * 100 / 2.54));
        }

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
struct STreePlatInfo {
    QByteArray headBytes;
    QByteArray itemsBytes;
    QByteArray md5;
    int bzItemCount = 0;
    int groupCount = 0;
};
void serializationTree_helper_1(QDataStream &countStream, int &bzItemCount, int &groupCount,
                                const CGroupBzItemsTreeInfo &tree, std::function<void(int, int)> f = nullptr)
{
    countStream << tree.childGroups.size();

    qDebug() << "save group count  = " << tree.childGroups.size();

    foreach (auto p, tree.childGroups) {
        qApp->processEvents();
        serializationTree_helper_1(countStream, bzItemCount, groupCount, p, f);
    }

    countStream << tree.bzItems.size();

    qDebug() << "save item count   = " << tree.bzItems.size();

    foreach (auto i, tree.bzItems) {
        qApp->processEvents();
        countStream << i;
        qDebug() << "save item info    = " << i.head.dataType;

        ++bzItemCount;
        if (f != nullptr) {
            f(bzItemCount, groupCount);
        }
    }

    //bzItemCount += tree.bzItems.size();

    countStream << tree.data;

    qDebug() << "save group info   = " << tree.data.head.dataType;

    ++groupCount;
    if (f != nullptr) {
        f(bzItemCount, groupCount);
    }
}
STreePlatInfo serializationTreeToBytes_1(const CGroupBzItemsTreeInfo &tree)
{
    STreePlatInfo info;

    QDataStream streamForCountBytes(&info.itemsBytes, QIODevice::WriteOnly);
    serializationTree_helper_1(streamForCountBytes, info.bzItemCount, info.groupCount, tree);

    return info;
}

QByteArray serializationHeadToBytes(const CGraphics &head)
{
    QByteArray  array;
    QDataStream streamForCountBytes(&array, QIODevice::WriteOnly);
    streamForCountBytes << head;

    return array;
}
int saveDdfWithCombinGroup(const QString &path, PageContext *contex, FileHander *hander)
{
    QString errorString;
    int error = FileHander::NoError;

    auto pDrawScen = contex->scene();

    CGroupBzItemsTreeInfo treeInfo = pDrawScen->getGroupTreeInfo(nullptr, ESaveToDDf, true);

    //0.开始之前设置当前要保存的文件名,并去掉针对该文件的监视(自身程序修改该文件不需要提示文件被修改了)

    //1.将treeInfo进行序列化到内存这个过程很快(很多图片的时候内存是否足够?)
    STreePlatInfo info = serializationTreeToBytes_1(treeInfo);

    //2.初始化文件的头信息(主要包含了ddf版本号,Qdatatream版本号，总的图元个数,场景的大小)并序列化到内存
    CGraphics head;
    QDataStream dataversion;
    head.version   = qint32(dataversion.version());    //存储datastream版本
    head.version   = (head.version << 8) | qint32(EDdfCurVersion);//ddf版本
    head.unitCount = info.groupCount + info.bzItemCount;         //在加载时可快速获知图元总数
    head.rect      = pDrawScen->sceneRect();                     //场景的大小
    info.headBytes = serializationHeadToBytes(head);

    //3.将树结构的二进制流数据进行md5加密
    info.md5 = QCryptographicHash::hash(info.headBytes + info.itemsBytes, QCryptographicHash::Md5);

    qDebug() << "save head size = " << info.headBytes.count() << "byte size = " << info.itemsBytes.count() << "md5 count = " << info.md5.count();

    //4.再进行磁盘检查(判断目标文件所处的磁盘分区空间是否足够),如果未通过,交互提示返回
    if (!hander->isVolumeSpaceAvailabel(path, info.itemsBytes.size() + info.md5.size())) {
        qWarning() << "volum space is not enough !!!";
        errorString = QObject::tr("Unable to save. There is not enough disk space.");
        error = FileHander::EInsufficientPartitionSpace;
    } else {
        //5.将数据保存到文件
        QFile writeFile(path);
        if (writeFile.open(QIODevice::WriteOnly/* | QIODevice::ReadOnly | QIODevice::Truncate*/)) {
            QDataStream out(&writeFile);

            //5.1 首先保存沿用的老的头信息(主要包含了ddf版本号,datastream版本，总的图元个数,场景的大小)
            int resultByteSize = out.writeRawData(info.headBytes.data(), info.headBytes.size());
            qDebug() << "write heads bytes: " << info.headBytes.size() << "success byteSize: " << resultByteSize;

            //5.2 写入图元的数据(安排一下进度?)
            int granularityByteSize = info.itemsBytes.size() > 3 * 32 * 1024 ? 32 * 1024 : qCeil(info.itemsBytes.size() / 3); //设置字节粒度 //一次写入不超过32 * 1024, 最少分3次保存
            resultByteSize = 0;
            while (resultByteSize < info.itemsBytes.size()) {
                qApp->processEvents();
                QByteArray bytes = info.itemsBytes.mid(resultByteSize, granularityByteSize);
                resultByteSize += out.writeRawData(bytes.data(), bytes.size());
                emit hander->progressChanged(resultByteSize, info.itemsBytes.size(), "");
            }
            qDebug() << "write items bytes: " << info.itemsBytes.size() << "success byteSize: " << resultByteSize;

            //5.3 写入md5数据
            resultByteSize = out.writeRawData(info.md5.data(), info.md5.size());
            qDebug() << "write  md5  bytes: " << info.md5.size() << "success byteSize: " << resultByteSize << "md5 = " << info.md5.toHex().toUpper();

            //5.4 关闭文件(执行后才会真的写入到磁盘)
            writeFile.close();

            if (writeFile.error() != QFileDevice::NoError) {
                error = writeFile.error();
                errorString = writeFile.errorString();
            } else {
                contex->setFile(path);
                contex->setDirty(false);
            }
            qDebug() << "save ddf end---------------- " << writeFile.error();
        } else {
            error = writeFile.error();
            qWarning() << "PermissionsError ==== " << error << QFile::PermissionsError;
            errorString = writeFile.errorString();
        }
    }

    PageScene::releaseBzItemsTreeInfo(treeInfo);
    hander->d_pri()->setError(error, errorString);
    return error;
}

/********************************************  SAVE STATIC FUNCTIONS ********************************************/



FileHander::FileHander(QObject *parent): QObject(parent)
{
    _pPrivate = new FileHander_private(this);
}

FileHander::~FileHander()
{
    delete _pPrivate;
}
QStringList FileHander::supPictureSuffix()
{
    auto allSupSuffix = drawApp->readableFormats();
    allSupSuffix.removeFirst();
    //static QStringList supPictureSuffixs = QStringList() << "png" << "jpg" << "bmp" << "tif" << "jpeg" ;
    return allSupSuffix;
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
bool FileHander::isLegalFile(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }

//    QRegExp regExp("[:\\*\\?\"<>\\|]");

//    if (path.contains(regExp)) {
//        return false;
//    }

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
    d_pri()->unsetError();

    emit progressBegin(tr("Opening..."));

    PageContext *result = nullptr;
    if (checkFileBeforeLoad(file, true)) {
        auto legalPath = toLegalFile(file);
        int ret = NoError;
        result = new PageContext(legalPath);
        result->moveToThread(qApp->thread());
        EDdfVersion ddfVersion = getDdfVersion(legalPath);
        if (ddfVersion >= EDdf5_9_0_3_LATER) {
            ret = loadDdfWithCombinGroup(legalPath, result, this);
        } else {
            ret = loadDdfWithNoCombinGroup(legalPath, result, this);
        }
        if (ret != NoError) {
            result->deleteLater();
            result = nullptr;
        }
    }
    emit progressEnd(d_pri()->lastError, d_pri()->lastErrorDescribe);
    return result;
}

bool FileHander::saveToDdf(PageContext *context, const QString &file)
{
    d_pri()->unsetError();

    emit progressBegin(tr("Saving..."));

    auto filePath = file.isEmpty() ? context->file() : file;
    if (checkFileBeforeSave(filePath, true)) {
        filePath = toLegalFile(filePath);

        if (context->page() != nullptr)
            context->page()->borad()->fileWatcher()->removePath(filePath);

        saveDdfWithCombinGroup(filePath, context, this);

        if (context->page() != nullptr)
            context->page()->borad()->fileWatcher()->addWather(filePath);
    }

    emit progressEnd(d_pri()->lastError, d_pri()->lastErrorDescribe);

    return  d_pri()->lastError == NoError;
}

QImage FileHander::loadImage(const QString &file)
{
    d_pri()->unsetError();
    if (checkFileBeforeLoad(file, false)) {
        auto legalPath = toLegalFile(file);
        QImage img = loadImage_helper(legalPath, this);
        if (img.isNull()) {
            d_pri()->setError(EDamagedImageFile, tr("Damaged file, unable to open it"));
        }
        img = img.convertToFormat(QImage::Format_ARGB32);
        return img;
    }
    return QImage();
}

bool FileHander::saveToImage(PageContext *context, const QString &file, const QSize &desImageSize, int imageQuility)
{
    d_pri()->unsetError();

    if (checkFileBeforeSave(file, false)) {

        QFileInfo info(file);
        auto stuff = info.suffix().toLower();
        //QColor bgColor = (stuff == "jpg" || stuff == "bmp" ? Qt::white : Qt::transparent);
        auto image = context->renderToImage(Qt::white, desImageSize);

        if (toLegalFile(file).isEmpty())
            return false;

        if (image.isNull())
            return false;

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
    return d_pri()->lastErrorDescribe;
}

int FileHander::lastError() const
{
    return d_pri()->lastError;
}

bool FileHander::checkFileBeforeLoad(const QString &file, bool isDdf)
{
    //1 check if name illegal.
    auto legalPath = toLegalFile(file);
    if (legalPath.isEmpty()) {
        d_pri()->setError(EFileNotExist, tr("The file does not exist"));
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
        d_pri()->setError(EUnSupportFile, tr("Unable to open \"%1\", unsupported file format").arg(info.fileName()));
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
        d_pri()->setError(EFileNameIllegal, tr("The file name must not contain \\/:*?\"<>|"));
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

EDdfVersion FileHander::getDdfVersion(const QString &file) const
{
    EDdfVersion ver = EDdfUnknowed;
    QFile f(file);
    if (f.exists()) {
        if (f.open(QFile::ReadOnly)) {
            QDataStream s(&f);
            //获取版本号
            ver = getVersion(s);
        }
    }
    return ver;
}

void FileHander::setQDataStreamVersion(const QString &path, QDataStream &data, const CGraphics &head)
{
    //1040以前版本用Qt_5_6
    if (getDdfVersion(path) <= EDdf5_8_0_84_LATER) {
        data.setVersion(QDataStream::Qt_5_6);
    } else {
        if (data.device() != nullptr) {
            int qdataversion = head.version >> 8;
            if (qdataversion)
                data.setVersion(qdataversion);
        } else {
            data.setVersion(QDataStream::Qt_5_6);
        }
    }
}

bool FileHander::isDdfFileDirty(const QString &filePath)const
{
    QFile file(filePath);
    if (file.exists()) {
        if (file.open(QFile::ReadOnly)) {
            QDataStream s(&file);
            // 先通过版本号判断是否ddf拥有md5校验值
            EDdfVersion ver = getVersion(s);
            if (ver >= EDdf5_8_0_20) {
                QByteArray allBins = file.readAll();
                QByteArray md5    = allBins.right(16);

                qDebug() << "load  head+bytes = " << (allBins.count() - md5.count()) << "md5 count = " << md5.count();
                qDebug() << "direct read MD5 form ddffile file = " << filePath << " MD5 = " << md5.toHex().toUpper();

                QByteArray contex = allBins.left(allBins.size() - md5.size());

                QByteArray nMd5 = QCryptographicHash::hash(contex, QCryptographicHash::Md5);

                qDebug() << "recalculate MD5 form ddffile file = " << filePath << " MD5 = " << nMd5.toHex().toUpper();

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

bool FileHander::checkFileExist(const QString &file) const
{
    //1 check if exist.
    QFileInfo info(file);
    if (!info.exists()) {
        d_pri()->lastError = EFileNotExist;
        d_pri()->lastErrorDescribe = tr("The file does not exist");
        return false;
    }
    return true;
}

bool FileHander::checkFileReadable(const QString &file) const
{
    //2 check if readable.
    QFileInfo info(file);
    if (!info.isReadable()) {
        d_pri()->setError(EUnReadableFile, tr("Unable to open the write-only file \"%1\"").arg(info.fileName()));
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
            d_pri()->setError(EUnWritableFile, tr("This file is read-only, please save with another name"));
            return false;
        }

    } else {
        QString dirPath = info.absolutePath();
        QFileInfo dir(dirPath);
        if (dir.isDir() && !dir.isWritable()) {
            d_pri()->setError(EUnWritableDir, tr("You do not have permission to save files here, please change and retry"));
            return false;
        }
        return true;
    }
    return true;
}

bool FileHander::checkDdfVersionIllegal(const QString &ddfFile) const
{
    //3.first to check ddf version.
    EDdfVersion ddfVersion = getDdfVersion(ddfFile);
    if (ddfVersion > EDdfCurVersion) {
        qWarning() << "The file is incompatible with the old app, please install the latest version.";


        //文件版本与当前应用不兼容，请安装最新版应用
//        DrawBoard::exeMessage(tr("The file is incompatible with the old app, please install the latest version"),
//                              DrawBoard::EWarningMsg, false);

        d_pri()->lastError = EExcessiveDdfVersion;
        d_pri()->lastErrorDescribe = tr("The file is incompatible with the old app, please install the latest version");
        return false;

    } else if (ddfVersion < 0) {
        qWarning() << "Cannot open unknown version file!";
        d_pri()->lastError = EUnKnowedDdfVersion;
        d_pri()->lastErrorDescribe = "unknowedDdfVersion";
        return false;
    }
    return true;
}

bool FileHander::checkDdfMd5(const QString &ddfFile) const
{
    if (isDdfFileDirty(ddfFile)) {
//        DrawBoard::exeMessage(tr("Unable to open the broken file \"%1\"").arg(QFileInfo(ddfFile).fileName()),
//                              DrawBoard::EWarningMsg, false);
        d_pri()->lastError = EDdfFileMD5Error;
        d_pri()->lastErrorDescribe = tr("Unable to open the broken file \"%1\"").arg(QFileInfo(ddfFile).fileName());
        return false;
    }
    return true;
}

