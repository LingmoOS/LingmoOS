extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <QTimer>
#include <QAbstractButton>
#include <QDirIterator>

#include "musicfileinformation.h"
#include "UI/mainwidget.h"

#include "lingmo-log4qt.h"

MusicFileInformation::MusicFileInformation(QObject *parent) : QObject(parent)
{
    m_musicType << "*.voc" << "*.aiff" << "*.au" << "*.dts" << "*.flv" << "*.m4r"
                << "*.mka" << "*.mmf" << "*.mp2" << "*.mp4" << "*.mpa" << "*.wv"
                << "*.mp3" << "*.ogg" << "*.wma" << "*.amr" << "*.flac"
                << "*.wav" << "*.ape" << "*.m4a" << "*.ac3" << "*.aac";
}

QStringList MusicFileInformation::getMusicType()
{
    return m_musicType;
}

void MusicFileInformation::addFile(const QStringList &addFile)
{
    qDebug() << "addFile";

    int count = 0;
    int failCount = 0;
    resList.clear();

    for(auto &filepath : addFile)
    {
        if(filepath.isEmpty())
        {
            continue;
        }
        //过滤掉CD和远程目录
//        if(filepath.startsWith("/run/user/"))
//        {
//            continue;
//        }
        //过滤U盘路径
//        if(filepath.startsWith("/media/"))
//        {
//            continue;
//        }
        QFileInfo fileInfo(filepath);
        if(fileInfo.isDir())
        {
            //适合用于大目录
            QDirIterator iter(filepath,m_musicType,QDir::Files,QDirIterator::Subdirectories);
            while (iter.hasNext())
            {
                count++;
                QString strpath = iter.next();
                musicdataStruct.filepath = strpath;
                fileInfo.setFile(musicdataStruct.filepath);
                fileType(fileInfo);          //文件类型
                fileSize(fileInfo);      //文件大小
                if(m_musicType.contains("*." + musicdataStruct.filetype))
                {
                    fileInformation(musicdataStruct.filepath);//获取歌曲文件信息
                    if(musicdataStruct.time != "")
                    {
                        resList.append(musicdataStruct);
                    } else {
                        failCount++;
                    }
                }
                else
                {
                    failCount++;
                }
            }
        }
        else if(fileInfo.isFile())
        {
            count++;
            musicdataStruct.filepath = filepath;
            fileInfo.setFile(musicdataStruct.filepath);
            fileType(fileInfo);          //文件类型
            fileSize(fileInfo);      //文件大小
            if(m_musicType.contains("*." + musicdataStruct.filetype))
            {
                fileInformation(musicdataStruct.filepath);//获取歌曲文件信息
                if(musicdataStruct.time != "")
                {
                    resList.append(musicdataStruct);
                } else {
                    failCount++;
                }
            }
            else
            {
                failCount++;
            }
        }
    }
    musicCount = count;
    m_failCount = failCount;
//    resList.clear();
//    if(!addFile.isEmpty())
//    {
//        for(int i = 0; i < addFile.size(); i++)
//        {
//            musicdataStruct.filepath = addFile.at(i);
//            fileInfo.setFile(musicdataStruct.filepath);
//            fileType(fileInfo);          //文件类型
//            fileSize(fileInfo);      //文件大小
//            if(musicType.indexOf(musicdataStruct.filetype) != -1)
//            {
//                QStringList list = fileInformation(musicdataStruct.filepath);//获取歌曲文件信息
//                if(musicdataStruct.time == "")
//                {
//                    m_failCount++;
//                }
//                else
//                {
//                    m_successCount++;
//                }
//                resList.append(musicdataStruct);
//            }
//            else
//            {
//                qDebug() << "添加文件失败";
//            }
//        }
//    }
//    slotImportFinished(m_successCount, m_failCount);
}

int MusicFileInformation::getCount()
{
    return musicCount;
}

int MusicFileInformation::getFailCount()
{
    return m_failCount;
}

QString MusicFileInformation::filterTextCode(QString str)
{
    QByteArray strdata =str.toLocal8Bit();
    int len = strdata.length();
    int num = 0;
    int i = 0;
    while (i < len) {
        if ((strdata[i] & 0x80) == 0x00) {
            // 0XXX_XXXX
            i++;
            continue;
        }
        else if ((num = preNum(strdata[i])) > 2) {
            // 110X_XXXX 10XX_XXXX
            // 1110_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // preNUm() 返回首个字节8个bits中首�?0bit前面1bit的个数，该数量也是该字符所使用的字节数
            i++;
            for(int j = 0; j < num - 1; j++) {
                //判断后面num - 1 个字节是不是都是10开
                if ((strdata[i] & 0xc0) != 0x80) {
                    return "";
                }
                i++;
            }
        } else {
            //其他情况说明不是utf-8
            return "";
        }
    }
    return str;
}

inline int MusicFileInformation::preNum(unsigned char byte) {
    unsigned char mask = 0x80;
    int num = 0;
    for (int i = 0; i < 8; i++) {
        if ((byte & mask) == mask) {
            mask = mask >> 1;
            num++;
        } else {
            break;
        }
    }
    return num;
}

QStringList MusicFileInformation::fileInformation(QString filepath)
{
    QFileInfo fileInfo(filepath);
    QStringList information;
    QByteArray byteArray = filepath.toLocal8Bit();
    TagLib::FileRef f(byteArray.data());
    if(f.isNull())
    {
        musicdataStruct.time = "";
        AVFormatContext *pFormatCtx = NULL;
        avformat_open_input(&pFormatCtx, filepath.toStdString().c_str(), nullptr, nullptr);
        if(pFormatCtx)
        {
            avformat_find_stream_info(pFormatCtx, nullptr);
            qint64 duration = pFormatCtx->duration / 1000;
            if(duration > 0)
            {
                duration = duration / 1000;
//                QTime me(0,(duration/60000) % 60,(duration / 1000) % 60);
//                QString length = me.toString("mm:ss");
                int h = duration / 3600;
                QString hour = QString("%1").arg(duration / 3600, 2, 10, QChar('0'));
                QString minutes = QString("%1").arg(duration % 3600 / 60, 2, 10, QChar('0'));
                QString seconds = QString("%1").arg(duration % 60, 2, 10, QChar('0'));

                if(h > 0) {
                    musicdataStruct.time = QString("%1:%2:%3").arg(hour).arg(minutes).arg(seconds);
                }
                else {
                    musicdataStruct.time = QString("%1:%2").arg(minutes).arg(seconds);
                }

                musicdataStruct.title = fileInfo.completeBaseName();
                musicdataStruct.singer = tr("Unknown singer");
                musicdataStruct.album  = tr("Unknown album");
                information << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
                return information;
            }
        }
        else
        {
            QStringList str;
            musicdataStruct.singer = "";
            musicdataStruct.album = "";
            musicdataStruct.title  = "";
            musicdataStruct.time  = "";
            str << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
            return str;
        }
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
    }
    if (f.file() == nullptr) {
        KyInfo() << "f.file == nullptr";
        QStringList str;
        musicdataStruct.singer = "";
        musicdataStruct.album = "";
        musicdataStruct.title  = "";
        musicdataStruct.time  = "";
        str << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
        return str;
    }

    TagLib::PropertyMap propertyMap = f.file() ->properties();
    QString musicName = propertyMap["TITLE"].toString().toCString(true);
    if(filterTextCode(musicName).isEmpty())
        musicName = fileInfo.completeBaseName();
    QString musicSinger = propertyMap["ARTIST"].toString().toCString(true);
    if(filterTextCode(musicSinger).isEmpty())
        musicSinger = tr("Unknown singer");
    QString musicAlbum = propertyMap["ALBUM"].toString().toCString(true);
    if(filterTextCode(musicAlbum).isEmpty())
        musicAlbum = tr("Unknown album");
    TagLib::AudioProperties *properties = f.audioProperties();

    if(properties == nullptr)
    {
        QStringList str;
        musicdataStruct.singer = "";
        musicdataStruct.album = "";
        musicdataStruct.title  = "";
        musicdataStruct.time  = "";
        str << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
        return str;
    }

    int h = properties->length() / 3600;
    if( h <= 0)
    {
        musicdataStruct.time = "";
        AVFormatContext *pFormatCtx = NULL;
        avformat_open_input(&pFormatCtx, filepath.toStdString().c_str(), nullptr, nullptr);
        if(pFormatCtx)
        {
            avformat_find_stream_info(pFormatCtx, nullptr);
            qint64 duration = pFormatCtx->duration / 1000;
            if(duration > 0)
            {
                duration = duration / 1000;
                int h = duration / 3600;
                QString hour = QString("%1").arg(duration / 3600, 2, 10, QChar('0'));
                QString minutes = QString("%1").arg(duration % 3600 / 60, 2, 10, QChar('0'));
                QString seconds = QString("%1").arg(duration % 60, 2, 10, QChar('0'));

                if(h > 0) {
                    musicdataStruct.time = QString("%1:%2:%3").arg(hour).arg(minutes).arg(seconds);
                }
                else {
                    musicdataStruct.time = QString("%1:%2").arg(minutes).arg(seconds);
                }

                musicdataStruct.title = musicName;
                musicdataStruct.singer = musicSinger;
                musicdataStruct.album  = musicAlbum;
                information << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
                return information;
            }
        }
        else
        {
            QStringList str;
            musicdataStruct.singer = "";
            musicdataStruct.album = "";
            musicdataStruct.title  = "";
            musicdataStruct.time  = "";
            str << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
            return str;
        }
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
    }
    QString hour = QString("%1").arg(properties->length() / 3600, 2, 10, QChar('0'));
    QString minutes = QString("%1").arg(properties->length() % 3600 / 60, 2, 10, QChar('0'));
    QString seconds = QString("%1").arg(properties->length() % 60, 2, 10, QChar('0'));

    if(h > 0) {
        musicdataStruct.time = QString("%1:%2:%3").arg(hour).arg(minutes).arg(seconds);
    }
    else {
        musicdataStruct.time = QString("%1:%2").arg(minutes).arg(seconds);
    }

//    int seconds = properties->length() % 60;
//    int minutes = (properties->length() - seconds) / 60;
//    QTime time(0,minutes,seconds);
////    QString musicTime = QString::number(minutes)+":"+QString("%1").arg(seconds, 2, 10, QChar('0'));
//    QString musicTime = time.toString("mm:ss");
    musicdataStruct.title = musicName;
    musicdataStruct.singer = musicSinger;
    musicdataStruct.album = musicAlbum;
    QStringList audioFileInformation;
    audioFileInformation << musicdataStruct.title << musicdataStruct.singer
                         << musicdataStruct.album << musicdataStruct.time;
    return audioFileInformation;
}

QPixmap MusicFileInformation::getCoverPhotoPixmap(QString filepath)
{
    AVFormatContext *pFormatCtx = NULL;
    avformat_open_input(&pFormatCtx, filepath.toUtf8().data(), nullptr, nullptr);

    QPixmap pixmap;
    QImage image;
    if(pFormatCtx)
    {
        if(pFormatCtx->iformat != nullptr && pFormatCtx->iformat->read_header(pFormatCtx) >= 0)
        {
            for(unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
            {
                if(pFormatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC)
                {
                    AVPacket pkt = pFormatCtx->streams[i]->attached_pic;
                    image = QImage::fromData(static_cast<uchar *>(pkt.data), pkt.size);
                    break;
                }
            }
        }
    }

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    pixmap = QPixmap::fromImage(image);
    return pixmap;
}

QStringList MusicFileInformation::updateSongInfoFromLocal(QString filepath)
{
    QFileInfo fileInfo(filepath);
    QStringList lists;
    int importFailCount = 0;
    musicdataStruct.time = "";
    AVFormatContext *pFormatCtx = NULL;
    avformat_open_input(&pFormatCtx, filepath.toStdString().c_str(), nullptr, nullptr);
    if(pFormatCtx)
    {
        avformat_find_stream_info(pFormatCtx, nullptr);
        qint64 duration = pFormatCtx->duration / 1000;
        if(duration > 0)
        {
            QTime me(0,(duration/60000) % 60,(duration / 1000) % 60);
            QString length = me.toString("mm:ss");
            musicdataStruct.time = length;
        }
    }
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    if(musicdataStruct.time == "")
    {
        importFailCount++;
        QEventLoop loop;
        MMediaContent media(QUrl::fromLocalFile(filepath));
        MMediaPlayer *music = new MMediaPlayer();
        music->setMedia(media);
        connect(music,SIGNAL(durationChanged(qint64)),this,SLOT(durationChange(qint64)));
        connect(this,SIGNAL(durations()),&loop,SLOT(quit()));
        QTimer::singleShot(1000,&loop,&QEventLoop::quit);
        loop.exec();
        if(success)
        {
            QStringList list;
            QString name = fileInfo.completeBaseName();
            QString singer = "未知歌手";
            QString album = "未知专辑";
            musicdataStruct.singer = singer;
            musicdataStruct.album = album;
            musicdataStruct.title  = name;
            QTime me(0,(dur/60000) % 60,(dur / 1000) % 60);
            QString m_str = me.toString("mm:ss");
            musicdataStruct.time = m_str;
            list << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
            return list;
        }
        else
        {
            QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("Add failed, no valid music file found"),QMessageBox::Ok);
            QStringList str;
            musicdataStruct.singer = "";
            musicdataStruct.album = "";
            musicdataStruct.title  = "";
            musicdataStruct.time  = "";
            str << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
            return str;
        }
        music->deleteLater();
    }
    if(importFailCount <= 0)
    {
        QString name = fileInfo.completeBaseName();
        QString singer = "未知歌手";
        QString album = "未知专辑";
        musicdataStruct.singer = singer;
        musicdataStruct.album = album;
        musicdataStruct.title  = name;
        lists << musicdataStruct.title << musicdataStruct.singer << musicdataStruct.album << musicdataStruct.time;
        return lists;
    }
}

QString MusicFileInformation::fileSize(QFileInfo fileInfo)
{
    QString musicSize;
    qint64 size = fileInfo.size();   //文件大小
    if(size/1024)
    {
        if(size/1024/1024)
        {
            if(size/1024/1024/1024)
            {
                musicSize = QString::number(size/1024/1024/1024,10)+"GB";
            }
            else
                musicSize = QString::number(size/1024/1024,10)+"MB";
        }
        else
            musicSize = QString::number(size/1024,10)+"KB";
    }
    else
        musicSize = QString::number(size,10)+"B";
    musicdataStruct.size = musicSize;
    return musicdataStruct.size;
}

bool MusicFileInformation::checkFileIsDamaged(QString filepath)
{
    QProcess *ffmpegCheckProcess = new QProcess(this);
    QString cmd = "/usr/bin/ffmpeg";
    QStringList args;
    args.append("-i");
    args.append(filepath);
    ffmpegCheckProcess->start(cmd, args);
    ffmpegCheckProcess->waitForFinished();
    ffmpegCheckProcess->waitForReadyRead();

    QString results = QString::fromLocal8Bit(ffmpegCheckProcess->readAllStandardOutput());
    KyInfo() << "ffmpeg info: "<< results;

    return true;

}

QString MusicFileInformation::fileType(QFileInfo fileInfo)
{
    QString musicType = fileInfo.suffix().toLower();        //文件类型
    musicdataStruct.filetype = musicType;
    return musicdataStruct.filetype;
}

void MusicFileInformation::durationChange(qint64 duration)
{
    if(duration <= 0)
    {
        return;
    }
    dur = duration;
    success = true;
    Q_EMIT durations();
}

int MusicFileInformation::findIndexFromPlayList(QString listname,QString filepath)
{
    int index = -1;
    int ret;
    QList<musicDataStruct> musicDateList;
    ret = g_db->getSongInfoListFromDB(musicDateList,listname);
    if(ret == DB_OP_SUCC) {
        for(int i = 0; i < musicDateList.size(); i++) {
            if(musicDateList[i].filepath == filepath) {
                index = i;
                break;
            }
        }
    }
    return index;
}
