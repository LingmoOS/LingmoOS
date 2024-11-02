#include "musicDataBase.h"
#include<iostream>
#include<QSqlQuery>
#include<QMessageBox>
#include<QApplication>
#include<QSqlError>
#include<QFileInfo>
#include<QDebug>
#include<QSqlRecord>
#include<QTime>
#include<sys/time.h>

MusicDataBase::MusicDataBase(QObject *parent) : QObject(parent)
{
    qDebug() << QSqlDatabase::drivers();//当前环境支持哪些数据库
    QMutexLocker lockData( &m_mutex);  //加锁，函数执行完后自动解锁
    m_database=QSqlDatabase::addDatabase("QSQLITE");
    QString dirPath = QString(getenv("HOME")) + "/.config/.lingmo_music_ver1.2_";
//TODO
    QFileInfo oldVersion(QString(getenv("HOME")) + "/.config/.lingmo_music_ver1.0_" + "mymusic.db");
    if(oldVersion.exists())
    {
        //读取旧版本数据库内容，并添加至新版本数据库函数
        qDebug() << "存在旧版本数据库" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    }
    m_database.setDatabaseName(dirPath + "mymusic.db");
}

MusicDataBase::~MusicDataBase()
{
    qDebug() << "析构";
    if(true == m_databaseOpenFlag)
    {
        sqlite3_shutdown();
        m_database.close();
    }
}

MusicDataBase* MusicDataBase::getInstance()
{

    static MusicDataBase *dbInstance = nullptr;
    if (dbInstance == nullptr)
    {
        try
        {
            dbInstance = new MusicDataBase;
        }
        catch (const std::runtime_error &re)
        {
            qDebug() << "runtime_error:" << re.what();
        }

    }
    return dbInstance;
}

int MusicDataBase::initDataBase()
{
    if(!m_database.open())
    {
        m_databaseOpenFlag = true;

        QMessageBox::warning(0, QObject::tr("Database Error"),
                                     m_database.lastError().text());
        return DB_UNCONNECT;
    }

    QVariant v = m_database.driver()->handle(); // 获得低级handle包
    if(v.isValid() && qstrcmp(v.typeName(), "sqlite3*") == 0)
    {
        sqlite3_initialize();
        // 显式地初始化一下。
        sqlite3 *m_handle = *static_cast<sqlite3 **>(v.data());
        if(m_handle)
        {
            sqlite3_enable_load_extension(m_handle, 1); //允许加载扩展
            QSqlQuery loadExtension(m_database);
            bool loadRes = loadExtension.exec("SELECT load_extension('libsimple')"); // 使用sql函数加载libsimple
            if(!loadRes)
            {
                qDebug() << "无法加载分词器扩展" << loadExtension.lastError().text();
                // 加载不成功先不return
            }
        }
    }

    bool queryRes = true;
    QSqlQuery queryInit(m_database);
    //新建表:总表，历史表，我喜欢表
    queryRes &= queryInit.exec(QString("create table if not exists %1 ("
                                       "id integer primary key autoincrement,"
                                       "idIndex integer unique,"
                                       "filepath varchar unique not NULL,"
                                       "title varchar,"
                                       "singer varchar,"
                                       "album varchar,"
                                       "filetype varchar,"
                                       "size varchar,"
                                       "time varchar)"
                                       ).arg(ALLMUSIC));//创建音乐总表，自增id为主键，index为唯一值，插入歌曲时为空，获取自增id值后赋值，filepath为唯一值且不为空。

    queryRes &= queryInit.exec(QString("create table if not exists %1 ("
                                       "id integer primary key autoincrement,"
                                       "idIndex integer unique,"
                                       "filepath varchar unique not NULL,"
                                       "title varchar,"
                                       "singer varchar,"
                                       "album varchar,"
                                       "filetype varchar,"
                                       "size varchar,"
                                       "time varchar)"
                                       ).arg(HISTORY));//创建历史播放列表，自增id为主键，index为唯一值，插入歌曲时为空，获取自增id值后赋值，filepath为唯一值且不为空。

    queryRes &= queryInit.exec(QString("create table if not exists ListOfPlayList (title varchar primary key)"));//创建播放列表名称列表

    queryRes &= queryInit.exec(QString("create virtual table if not exists AuxIndexLocalMusicContent"
                                       " using fts5(id UNINDEXED, title, singer, album, filepath UNINDEXED, time UNINDEXED, tokenize='simple', prefix='1 2 3 4 5')"));
    // 为localMusic本地音乐表创建辅助的全文索引虚拟表fts5，设置前缀索引，设置非索引项，加载simple分词器包。
//    queryRes &= queryInit.exec(QString("create trigger local_music_add after insert on LocalMusic begin"
//                                       " insert into AuxIndexLocalMusicContent values(new.id, new.title); end"));
    // 存在问题：利用触发器插入时需要额外操作（包括对新值处理从中文到拼音、从编码格式到不编码格式）。即便表中存储的并非base64，我们也需要处理从中文到拼音，包括取old变量值到转换，
    // 废弃方案

    queryRes &= queryInit.exec(QString("create trigger if not exists local_music_delete before delete on LocalMusic begin"
                                       " delete from AuxIndexLocalMusicContent where id=old.id;"
                                       " end"));
    // 创建触发器，根据id删除虚拟表记录

    if(true == queryRes)
    {
        qDebug()<<"本地列表，历史列表，歌单表创建成功！";

        //创建我喜欢列表
        //先检查是否存在
        int checkRes;
        checkRes = checkPlayListExist(FAV);
        if(LIST_NOT_FOUND == checkRes)//我喜欢列表不存在才创建
        {
            int createRes;
            createRes = createNewPlayList(FAV);

            return createRes;
        }
        else
        {
            return checkRes;
        }
    }
    else
    {
        qDebug() << "初始化建表失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return CREATE_TABLE_FAILED;
    }
}

int MusicDataBase::addMusicToLocalMusic(const musicDataStruct &fileData)
{
    bool queryRes = true;

    if(fileData.title.isEmpty() || fileData.filepath.isEmpty())
    {
        qDebug() << "无效入参" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }
    else
    {
        if(true == m_database.isValid())
        {
            //查询历史列表中是否已有该歌曲，已有的话，返回添加失败
            int checkLocalRes = checkIfSongExistsInLocalMusic(fileData.filepath);

            //历史列表中已经有这首歌，重复添加了
            if(DB_OP_SUCC == checkLocalRes)
            {
                qDebug() << "添加失败，重复添加" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_OP_ADD_REPEAT;
            }

            QSqlQuery addSongToLocal(m_database);
            QString addSongString = QString("insert into %1 (%2) values('%3','%4','%5','%6','%7','%8','%9')").
                    arg(ALLMUSIC).
                    arg(SHOWCONTEXTS).
                    arg(inPutStringHandle(fileData.filepath)).
                    arg(inPutStringHandle(fileData.title)).
                    arg(inPutStringHandle(fileData.singer)).
                    arg(inPutStringHandle(fileData.album)).
                    arg(inPutStringHandle(fileData.filetype)).
                    arg(inPutStringHandle(fileData.size)).
                    arg(inPutStringHandle(fileData.time));
            queryRes &= addSongToLocal.exec(addSongString);
            //插入歌曲时自增id和idIndex无法赋值，插入后取得自增id，给idIndex赋值
            int tempIndex = addSongToLocal.lastInsertId().toInt();
            bool setRes = true;
            QSqlQuery setSongIDFromLocal(m_database);
            QString setIndex = QString("update %1 set idIndex='%2' WHERE filepath='%3'")
                    .arg(ALLMUSIC)
                    .arg(tempIndex)
                    .arg(inPutStringHandle(fileData.filepath));
            setRes &= setSongIDFromLocal.exec(setIndex);

            // 每插入一条记录，都要更新辅助的虚拟表AuxIndexLocalMusicContent
            QSqlQuery addSongToIndexTable(m_database);
            QString addSongIndex = QString("insert into AuxIndexLocalMusicContent(id,title,singer,album,filepath,time) values('%1', '%2', '%3', '%4', '%5', '%6')")
                    .arg(tempIndex)
                    .arg(preHandle(fileData.title),
                         preHandle(fileData.singer),
                         preHandle(fileData.album),
                         preHandle(fileData.filepath),
                         preHandle(fileData.time));
            bool setVTableRes = addSongToIndexTable.exec(addSongIndex);

            if(true == (queryRes&setRes&setVTableRes))
            {
                return DB_OP_SUCC;
            }
            else
            {
                qDebug() << "数据库操作失败，添加失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_OP_ADD_FAILED;
            }
        }
        else
        {
            return DB_UNCONNECT;
        }

    }
    //如果是先添加至新建列表
    //TODO
    //添加至新建列表
}

int MusicDataBase::delMusicFromLocalMusic(const QString& filePath)
{
    bool queryRes = true;

    if(filePath.isEmpty())
    {
        qDebug() << "无效入参" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }
    else
    {
        if(true == m_database.isValid())
        {
            int checkRes = checkIfSongExistsInLocalMusic(filePath);
            if(checkRes == DB_OP_SUCC)
            {
                QSqlQuery delSongFromLocal(m_database);
                QString delSongString = QString("delete from %1 where filepath = '%2'").
                        arg(ALLMUSIC).
                        arg(inPutStringHandle(filePath));
                queryRes = delSongFromLocal.exec(delSongString);

                if(true == queryRes)
                {
                    return DB_OP_SUCC;
                }
                else
                {
                    qDebug() << "数据库打开，删除失败！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    return DB_OP_DEL_FAILED;
                }
            }
            else
            {
                return INVALID_INPUT;
            }
        }
        else
        {
            qDebug() << "数据库无法打开，请重试！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
            return DB_UNCONNECT;
        }

    }
}

int MusicDataBase::createNewPlayList(const QString& playListName)
{
    if(playListName.isEmpty())
    {
        qDebug() << "无效入参" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }

    if(true == m_database.isValid())
    {
        bool createRes = true;

//        QMutexLocker lockData( &m_mutex);  //加锁，函数执行完后自动解锁
        QSqlQuery createNewPlayList(m_database);
        QString createPlayListString = QString("create table if not exists 'playlist_%1' ("
                                               "id integer primary key autoincrement,"
                                               "idIndex integer unique,"
                                               "filepath varchar unique not NULL,"
                                               "title varchar,"
                                               "singer varchar,"
                                               "album varchar,"
                                               "filetype varchar,"
                                               "size varchar,"
                                               "time varchar)").
                arg(inPutStringHandle(playListName));//创建新建播放列表，自增id为主键，index为唯一值，插入歌曲时为空，获取自增id值后赋值，filepath为唯一值且不为空。
        createRes &= createNewPlayList.exec(createPlayListString);

        if(true != createRes)
        {
            return CREATE_TABLE_FAILED;
        }

        QSqlQuery addPlayListToList(m_database);
        QString addPlayListToListString = QString("insert into ListOfPlayList (title) values('%1')").
                arg(inPutStringHandle(playListName));
        createRes &= addPlayListToList.exec(addPlayListToListString);

        if(true == createRes)
        {
            return DB_OP_SUCC;
        }
        else
        {
            qDebug() << "创建新歌单失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
            return CREATE_TABLE_FAILED;
        }

    }
    else
    {
        return DB_UNCONNECT;
    }

}

int MusicDataBase::delPlayList(const QString& playListName)
{
    //入参检查
    if(playListName.isEmpty() || playListName == FAV)
    {
        qDebug() << "无效入参" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }

    if(true == m_database.isValid())
    {
        bool delRes = true;
        QSqlQuery delPlayListFromList(m_database);
        QString delPlayListFromListString = QString("delete from ListOfPlayList where title = '%1'").
                arg(inPutStringHandle(playListName));
        delRes &= delPlayListFromList.exec(delPlayListFromListString);

        QSqlQuery delPlayList(m_database);
        QString delPlayListString = QString("drop table if exists 'playlist_%1'"
                                    ).arg(inPutStringHandle(playListName));
        delRes &= delPlayList.exec(delPlayListString);

        if(true == delRes)
        {
            return DB_OP_SUCC;
        }
        else
        {
            return DEL_TABLE_FAILED;
        }

    }
    else
    {
        return DB_UNCONNECT;
    }

}

int MusicDataBase::getSongInfoFromLocalMusic(const QString& filePath, musicDataStruct &fileData)
{

    if(filePath.isEmpty())
    {
        qDebug() << "入参无效" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }

    if(true == m_database.isValid())
    {
        bool getRes = true;
        QSqlQuery getSongFromLocalMusic(m_database);
        QString getSongString = QString("select %1 from %2 where filepath = '%3'")
                .arg(SHOWCONTEXTS)
                .arg(ALLMUSIC)
                .arg(inPutStringHandle(filePath));
        getRes = getSongFromLocalMusic.exec(getSongString);

        if(true == getRes)
        {
            if(getSongFromLocalMusic.next())
            {
                fileData.filepath    = outPutStringHandle(getSongFromLocalMusic.value(0).toString());
                fileData.title       = outPutStringHandle(getSongFromLocalMusic.value(1).toString());
                fileData.singer      = outPutStringHandle(getSongFromLocalMusic.value(2).toString());
                fileData.album       = outPutStringHandle(getSongFromLocalMusic.value(3).toString());
                fileData.filetype    = outPutStringHandle(getSongFromLocalMusic.value(4).toString());
                fileData.size        = outPutStringHandle(getSongFromLocalMusic.value(5).toString());
                fileData.time        = outPutStringHandle(getSongFromLocalMusic.value(6).toString());
            }
            else
            {
                qDebug() << "数据库中查无此歌！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_DISORDERD;
            }
            return DB_OP_SUCC;
        }
        else
        {
            return DB_OP_GET_FAILED;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::getSongInfoListFromLocalMusic(QList<musicDataStruct>& resList)
{
    if(true == m_database.isValid())
    {
        bool getRes = true;

        QSqlQuery getSongListFromLocalMusic(m_database);
        QString getSongListString = QString("select filepath from %1 order by idIndex").arg(ALLMUSIC);//按index排序返回给前端，而非添加歌曲时的顺序
        getRes = getSongListFromLocalMusic.exec(getSongListString);

        if(true == getRes)
        {
            while(getSongListFromLocalMusic.next())
            {
                musicDataStruct temp;
                temp.filepath = outPutStringHandle(getSongListFromLocalMusic.value(0).toString());
                int curRes = getSongInfoFromLocalMusic(temp.filepath, temp);

                if(DB_OP_SUCC == curRes)
                {
                    resList.append(temp);
                }
                else
                {
                    return curRes;
                }
            }

            return DB_OP_SUCC;
        }
        else
        {
            return DB_OP_GET_FAILED;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::changeSongOrderInLocalMusic(const QString& selectFilePath, const QString& destinationFilePath)
{
    //入参检查
    if(selectFilePath.isEmpty() || destinationFilePath.isEmpty())
    {
        qDebug() << "入参为空" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;//入参为空
    }
    if(DB_OP_SUCC != checkIfSongExistsInLocalMusic(selectFilePath) || DB_OP_SUCC != checkIfSongExistsInLocalMusic(destinationFilePath))
    {
        qDebug() << "歌曲在歌曲总表中不存在" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;//歌曲在歌曲总表中不存在
    }
    if(selectFilePath == destinationFilePath)//位置没变化
    {
        qDebug() << "位置没变化，不用处理" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return DB_OP_SUCC;//位置没变化，不用处理
    }
    //获取歌曲的index信息
    int oldIndex = 0, newIndex = 0;
    int getRes1 = getSongIndexFromLocalMusic(selectFilePath, oldIndex);
    int getRes2 = getSongIndexFromLocalMusic(destinationFilePath, newIndex);

    if(getRes1 != DB_OP_SUCC)
    {
        qDebug() << "歌曲原INDEX获取失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return getRes1;
    }
    if(getRes2 != DB_OP_SUCC)
    {
        qDebug() << "目标歌曲INDEX获取失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return getRes2;
    }

    if(oldIndex < newIndex)//靠前的位置拖拽到了靠后的位置
    {
        bool setRes = true;
        QSqlQuery setSongIndexFromLocal(m_database);
        QString setIndex = QString("update %1 set idIndex=0 WHERE filepath='%2'")
                                    .arg(ALLMUSIC)
                                    .arg(inPutStringHandle(selectFilePath));
        setRes &= setSongIndexFromLocal.exec(setIndex);

        if(true == setRes)
        {
            bool getRes = true;
            QSqlQuery getBetweenSongIndexFromLocal(m_database);
            QString setIndexs = QString("select idIndex,filepath from %1 WHERE idIndex between '%2' and '%3'")
                                        .arg(ALLMUSIC)
                                        .arg(oldIndex)
                                        .arg(newIndex);
            getRes &= getBetweenSongIndexFromLocal.exec(setIndexs);

            if(true == getRes)
            {
                bool updateIndexRes = true;
                while(getBetweenSongIndexFromLocal.next())
                {
                    int tempIndex           = getBetweenSongIndexFromLocal.value(0).toInt();
                    QString tempFilepath    = outPutStringHandle(getBetweenSongIndexFromLocal.value(1).toString());

                    QSqlQuery updateSongIndexFromLocal(m_database);
                    QString updateIndex = QString("update %1 set idIndex='%2' WHERE filepath='%3'")
                                                    .arg(ALLMUSIC)
                                                    .arg(tempIndex-1)
                                                    .arg(inPutStringHandle(tempFilepath));
                    updateIndexRes &= updateSongIndexFromLocal.exec(updateIndex);
                }

                if(true == updateIndexRes)
                {
                    bool setRes2 = true;
                    QSqlQuery setSongIndexFromLocal2(m_database);
                    QString setIndex2 = QString("update %1 set idIndex='%2' WHERE filepath='%3'")
                                                .arg(ALLMUSIC)
                                                .arg(newIndex)
                                                .arg(inPutStringHandle(selectFilePath));
                    setRes2 &= setSongIndexFromLocal2.exec(setIndex2);

                    if(true == setRes2)
                    {
                        return DB_OP_SUCC;
                    }
                    else
                    {
                        return LIST_REORDER_ERR;
                    }
                }
                else
                {
                    return LIST_REORDER_ERR;
                }
            }
        }
    }
    else if(oldIndex > newIndex)//靠后的位置拖拽到了靠前的位置
    {
        bool setRes = true;
        QSqlQuery setSongIndexFromLocal(m_database);
        QString setIndex = QString("update %1 set idIndex=0 WHERE filepath='%2'")
                                    .arg(ALLMUSIC)
                                    .arg(inPutStringHandle(selectFilePath));
        setRes &= setSongIndexFromLocal.exec(setIndex);

        if(true == setRes)
        {
            bool getRes = true;
            QSqlQuery getBetweenSongIndexFromLocal(m_database);
            QString setIndexs = QString("select idIndex,filepath from %1 WHERE idIndex between '%2' and '%3' order by idIndex desc")
                                        .arg(ALLMUSIC)
                                        .arg(newIndex+1)
                                        .arg(oldIndex);
            getRes &= getBetweenSongIndexFromLocal.exec(setIndexs);

            if(true == getRes)
            {
                bool updateIndexRes = true;
                while(getBetweenSongIndexFromLocal.next())
                {
                    int tempIndex           = getBetweenSongIndexFromLocal.value(0).toInt();
                    QString tempFilepath    = outPutStringHandle(getBetweenSongIndexFromLocal.value(1).toString());

                    QSqlQuery updateSongIndexFromLocal(m_database);
                    QString updateIndex = QString("update %1 set idIndex='%2' WHERE filepath='%3'")
                                                    .arg(ALLMUSIC)
                                                    .arg(tempIndex+1)
                                                    .arg(inPutStringHandle(tempFilepath));
                    updateIndexRes &= updateSongIndexFromLocal.exec(updateIndex);
                }

                if(true == updateIndexRes)
                {
                    bool setRes2 = true;
                    QSqlQuery setSongIndexFromLocal2(m_database);
                    QString setIndex2 = QString("update %1 set idIndex='%2' WHERE filepath='%3'")
                                                .arg(ALLMUSIC)
                                                .arg(newIndex+1)
                                                .arg(inPutStringHandle(selectFilePath));
                    setRes2 &= setSongIndexFromLocal2.exec(setIndex2);

                    if(true == setRes2)
                    {
                        return DB_OP_SUCC;
                    }
                    else
                    {
                        return LIST_REORDER_ERR;
                    }
                }
                else
                {
                    return LIST_REORDER_ERR;
                }
            }
        }
    }
    else//位置没变
    {
        return DB_OP_SUCC;
    }
}

int MusicDataBase::getSongInfoListFromLocalMusicByKeyword(QList<musicDataStruct> &resList, const QString &keyword)
{
    // ph-code
    if(true == keyword.isEmpty())
    {
        // 空keyword不处理
        return INVALID_INPUT;
    }
    if(true == m_database.isValid())
    {
        bool getRes = true;

        QSqlQuery getSongListFromLocalMusicByKeyword(m_database);
//        QString getSongListStringByKeyword = QString("select * from LocalMusic where `id` in ("
//                                                     "select id from AuxIndexLocalMusicContent where AuxIndexLocalMusicContent match simple_query('%1') order by rank) ")
//                                                    .arg(keyword);
        // 直接在虚表中存取，不再根据id回表。
        QString getSongListStringByKeyword = QString("select * from AuxIndexLocalMusicContent"
                                                     " where AuxIndexLocalMusicContent match simple_query('%1') order by rank")
                                                    .arg(keyword);

        getRes = getSongListFromLocalMusicByKeyword.exec(getSongListStringByKeyword);

        if(true == getRes)
        {
            while(getSongListFromLocalMusicByKeyword.next())
            {
                musicDataStruct temp;
                temp.title      = getSongListFromLocalMusicByKeyword.value(1).toString();
                temp.singer     = getSongListFromLocalMusicByKeyword.value(2).toString();
                temp.album      = getSongListFromLocalMusicByKeyword.value(3).toString();
                temp.filepath   = getSongListFromLocalMusicByKeyword.value(4).toString();
                temp.time       = getSongListFromLocalMusicByKeyword.value(5).toString();

                resList.append(temp);
            }

            return DB_OP_SUCC;
        }
        else
        {
            qDebug() << "执行错误信息：" << getSongListFromLocalMusicByKeyword.lastError().text();
            return DB_OP_GET_FAILED;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::getCurtEstimatedListByKeyword(const QString& keyword,
                                                 int Number,
                                                 QList<musicDataStruct>& titleSongsList,
                                                 QList<musicDataStruct>& singersList,
                                                 QList<musicDataStruct>& albumsList)
{
//    qDebug() << "\n" << "====================" << "\n"
//             << keyword
//             << Number
//             << "\n" << "====================" << "\n";
//    Number = 99;
    if(true == keyword.isEmpty())
    {
        // 空keyword不处理
        return INVALID_INPUT;
    }
    if(true == m_database.isValid())
    {
        bool getTitle = true;
        bool getSinger = true;
        bool getAlbum = true;

        QSqlQuery getCurtEstimatedTitleList(m_database);

        QString getCurtEstimatedTitleListString = QString("select * from AuxIndexLocalMusicContent"
                                                     " where title match simple_query('%1') order by rank limit %2")
                                                    .arg(keyword, QString::number(Number));
        getTitle &= getCurtEstimatedTitleList.exec(getCurtEstimatedTitleListString);

        QSqlQuery getCurtEstimatedSingerList(m_database);
        QString getCurtEstimatedSingerListString = QString("select * from AuxIndexLocalMusicContent"
                                                     " where singer match simple_query('%1') order by rank limit %2")
                                                    .arg(keyword, QString::number(Number));
        getSinger &= getCurtEstimatedSingerList.exec(getCurtEstimatedSingerListString);

        QSqlQuery getCurtEstimatedAlbumList(m_database);
        QString getCurtEstimatedAlbumListString = QString("select * from AuxIndexLocalMusicContent"
                                                     " where album match simple_query('%1') order by rank limit %2")
                                                    .arg(keyword, QString::number(Number));
        getAlbum &= getCurtEstimatedAlbumList.exec(getCurtEstimatedAlbumListString);

        if(getTitle)
        {
            while(getCurtEstimatedTitleList.next())
            {
                musicDataStruct temp;
                temp.title      = getCurtEstimatedTitleList.value(1).toString();
                temp.singer     = getCurtEstimatedTitleList.value(2).toString();
                temp.album      = getCurtEstimatedTitleList.value(3).toString();
                temp.filepath   = getCurtEstimatedTitleList.value(4).toString();
                temp.time       = getCurtEstimatedTitleList.value(5).toString();
                titleSongsList.append(temp);
            }
        }
        else
        {
            qDebug() << "执行错误信息：" << getCurtEstimatedTitleList.lastError().text();
            qDebug() << getCurtEstimatedTitleList.lastQuery();
        }

        if(getSinger)
        {
            while(getCurtEstimatedSingerList.next())
            {
                musicDataStruct temp;
                temp.title      = getCurtEstimatedSingerList.value(1).toString();
                temp.singer     = getCurtEstimatedSingerList.value(2).toString();
                temp.album      = getCurtEstimatedSingerList.value(3).toString();
                temp.filepath   = getCurtEstimatedSingerList.value(4).toString();
                temp.time       = getCurtEstimatedSingerList.value(5).toString();
                singersList.append(temp);
            }
        }
        else
        {
            qDebug() << "执行错误信息：" << getCurtEstimatedSingerList.lastError().text();
        }

        if(getAlbum)
        {
            while(getCurtEstimatedAlbumList.next())
            {
                musicDataStruct temp;
                temp.title      = getCurtEstimatedAlbumList.value(1).toString();
                temp.singer     = getCurtEstimatedAlbumList.value(2).toString();
                temp.album      = getCurtEstimatedAlbumList.value(3).toString();
                temp.filepath   = getCurtEstimatedAlbumList.value(4).toString();
                temp.time       = getCurtEstimatedAlbumList.value(5).toString();
                albumsList.append(temp);
            }
        }
        else
        {
            qDebug() << "执行错误信息：" << getCurtEstimatedAlbumList.lastError().text();
        }

        if(getTitle && getSinger && getAlbum)
        {
            return DB_OP_SUCC;
        }
        else
        {
            return DB_OP_GET_FAILED;
        }
    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::getSongInfoListByAlbum(QList<musicDataStruct> &resList, const QString &album)
{
    if(true == album.isEmpty())
    {
        return INVALID_INPUT;
    }
    if(true == m_database.isValid())
    {
        bool getRes = true;

        QSqlQuery getSongsListByAlbum(m_database);
        QString getSongsListByAlbumString = QString("select * from LocalMusic where album = '%1'").arg(inPutStringHandle(album));
        getRes = getSongsListByAlbum.exec(getSongsListByAlbumString);

        if(true == getRes)
        {
            while(getSongsListByAlbum.next())
            {
                musicDataStruct fileData;
                fileData.filepath    = outPutStringHandle(getSongsListByAlbum.value(2).toString());
                fileData.title       = outPutStringHandle(getSongsListByAlbum.value(3).toString());
                fileData.singer      = outPutStringHandle(getSongsListByAlbum.value(4).toString());
                fileData.album       = outPutStringHandle(getSongsListByAlbum.value(5).toString());
                fileData.filetype    = outPutStringHandle(getSongsListByAlbum.value(6).toString());
                fileData.size        = outPutStringHandle(getSongsListByAlbum.value(7).toString());
                fileData.time        = outPutStringHandle(getSongsListByAlbum.value(8).toString());

                resList.append(fileData);
            }

            return DB_OP_SUCC;
        }
        else
        {
            qDebug() << "执行错误信息：" << getSongsListByAlbum.lastError().text();
            return DB_OP_GET_FAILED;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::getSongInfoListBySinger(QList<musicDataStruct> &resList, const QString &singer)
{
    if(true == singer.isEmpty())
    {
        return INVALID_INPUT;
    }
    if(true == m_database.isValid())
    {
        bool getRes = true;

        QSqlQuery getSongsListBySinger(m_database);
        QString getSongsListBySingerString = QString("select * from LocalMusic where singer = '%1'").arg(inPutStringHandle(singer));
        getRes = getSongsListBySinger.exec(getSongsListBySingerString);

        if(true == getRes)
        {
            while(getSongsListBySinger.next())
            {
                musicDataStruct fileData;
                fileData.filepath    = outPutStringHandle(getSongsListBySinger.value(2).toString());
                fileData.title       = outPutStringHandle(getSongsListBySinger.value(3).toString());
                fileData.singer      = outPutStringHandle(getSongsListBySinger.value(4).toString());
                fileData.album       = outPutStringHandle(getSongsListBySinger.value(5).toString());
                fileData.filetype    = outPutStringHandle(getSongsListBySinger.value(6).toString());
                fileData.size        = outPutStringHandle(getSongsListBySinger.value(7).toString());
                fileData.time        = outPutStringHandle(getSongsListBySinger.value(8).toString());

                resList.append(fileData);
            }

            return DB_OP_SUCC;
        }
        else
        {
            qDebug() << "执行错误信息：" << getSongsListBySinger.lastError().text();
            return DB_OP_GET_FAILED;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::getSongInfoFromPlayList(musicDataStruct &fileData, const QString& filePath,const QString& playListName)
{

    if(filePath.isEmpty() || playListName.isEmpty())
    {
        qDebug() << "无效入参" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }

    if(true == m_database.isValid())
    {
        int checkRes = checkPlayListExist(playListName);
        if(checkRes == DB_OP_SUCC)
        {
            bool getRes = true;
            QSqlQuery getplayList(m_database);
            QString getplayListString = QString("select title from ListOfPlayList where title = '%1'").
                    arg(inPutStringHandle(playListName));
            getRes = getplayList.exec(getplayListString);

            if(false == getRes)
            {
                qDebug() << "歌单表中查询歌单失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_OP_GET_FAILED;
            }

            if(getplayList.next())
            {
                QSqlQuery getSongFromPlayList(m_database);
                QString getSongFromPlayListString = QString("select %1 from 'playlist_%2' where filepath = '%3'")
                        .arg(SHOWCONTEXTS)
                        .arg(inPutStringHandle(playListName))
                        .arg(inPutStringHandle(filePath));
                getRes = getSongFromPlayList.exec(getSongFromPlayListString);

                if(false == getRes)
                {
                    return DB_OP_GET_FAILED;
                }
                if(getSongFromPlayList.next())
                {
                    fileData.filepath    = outPutStringHandle(getSongFromPlayList.value(0).toString());
                    fileData.title       = outPutStringHandle(getSongFromPlayList.value(1).toString());
                    fileData.singer      = outPutStringHandle(getSongFromPlayList.value(2).toString());
                    fileData.album       = outPutStringHandle(getSongFromPlayList.value(3).toString());
                    fileData.filetype    = outPutStringHandle(getSongFromPlayList.value(4).toString());
                    fileData.size        = outPutStringHandle(getSongFromPlayList.value(5).toString());
                    fileData.time        = outPutStringHandle(getSongFromPlayList.value(6).toString());
                }
                else
                {
                    qDebug() << "歌单中未查询到歌曲" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    return DB_OP_GET_FAILED;
                }
                return DB_OP_SUCC;
            }
            else
            {
                qDebug() << "歌单表中未查询到该歌单" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return PLAYLIST_UNFOUND;
            }
        }
        else
        {
            return checkRes;
        }

    }
    else
    {
        return DB_UNCONNECT;
    }
}

int MusicDataBase::getSongInfoListFromPlayList(QList<musicDataStruct>& resList,const QString& playListName)
{
    if(playListName.isEmpty())
    {
        qDebug() << "输入歌单名为空" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }

    if(true == m_database.isValid())
    {
        int checkRes = checkPlayListExist(playListName);
        if(checkRes == DB_OP_SUCC)
        {
            bool getRes = true;
            resList.clear();

            QSqlQuery getplayList(m_database);
            QString getplayListString = QString("select title from ListOfPlayList where title = '%1'").
                    arg(inPutStringHandle(playListName));
            getRes = getplayList.exec(getplayListString);

            if(false == getRes)
            {
                qDebug() << "歌单表中查询歌单失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_OP_GET_FAILED;
            }

            if(getplayList.next())
            {
                QSqlQuery getSongFromPlayList(m_database);
                QString getSongFromPlayListString = QString("select %1 from 'playlist_%2' order by idIndex")
                        .arg(SHOWCONTEXTS)
                        .arg(inPutStringHandle(playListName));//按index排序返回给前端，而非添加歌曲时的顺序
                getRes = getSongFromPlayList.exec(getSongFromPlayListString);

                if(true == getRes)
                {
                    while(getSongFromPlayList.next())
                    {
                        musicDataStruct temp;
                        temp.filepath      = outPutStringHandle(getSongFromPlayList.value(0).toString());
                        temp.title         = outPutStringHandle(getSongFromPlayList.value(1).toString());
                        temp.singer        = outPutStringHandle(getSongFromPlayList.value(2).toString());
                        temp.album         = outPutStringHandle(getSongFromPlayList.value(3).toString());
                        temp.filetype      = outPutStringHandle(getSongFromPlayList.value(4).toString());
                        temp.size          = outPutStringHandle(getSongFromPlayList.value(5).toString());
                        temp.time          = outPutStringHandle(getSongFromPlayList.value(6).toString());

                        resList.append(temp);
                    }

                    return DB_OP_SUCC;
                }
                else
                {
                    return DB_OP_GET_FAILED;
                }

            }
            else
            {
                return DB_DISORDERD;
            }
        }
        else
        {
            return checkRes;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::changeSongOrderInPlayList(const QString& selectFilePath, const QString& destinationFilePath, const QString& playListName)
{
    //入参检查
    if(selectFilePath.isEmpty() || destinationFilePath.isEmpty() || playListName.isEmpty())
    {
        qDebug() << "入参为空" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;//入参为空
    }
    if(DB_OP_SUCC != checkPlayListExist(playListName))
    {
        qDebug() << "歌单在歌单表中不存在" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;//歌单在歌单表中不存在
    }
    if(DB_OP_SUCC != checkIfSongExistsInPlayList(selectFilePath, playListName)
    || DB_OP_SUCC != checkIfSongExistsInPlayList(destinationFilePath, playListName))
    {
        qDebug() << "歌曲在歌单表中不存在" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;//歌曲在歌单表中不存在
    }
    if(selectFilePath == destinationFilePath)//位置没变化
    {
        qDebug() << "位置没变化，不用处理" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return DB_OP_SUCC;//位置没变化，不用处理
    }

    //获取歌曲的index信息
    int oldIndex = 0, newIndex = 0;
    int getRes1 = getSongIndexFromPlayList(selectFilePath, playListName, oldIndex);
    int getRes2 = getSongIndexFromPlayList(destinationFilePath, playListName, newIndex);

    if(getRes1 != DB_OP_SUCC)
    {
        qDebug() << "歌曲原INDEX获取失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return getRes1;
    }
    if(getRes2 != DB_OP_SUCC)
    {
        qDebug() << "目标歌曲INDEX获取失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return getRes2;
    }

    if(oldIndex < newIndex)//靠前的位置拖拽到了靠后的位置
    {
        bool setRes = true;
        QSqlQuery setSongIndexFromLocal(m_database);
        QString setIndex = QString("update 'playlist_%1' set idIndex=0 WHERE filepath='%2'")
                                    .arg(inPutStringHandle(playListName))
                                    .arg(inPutStringHandle(selectFilePath));
        setRes &= setSongIndexFromLocal.exec(setIndex);

        if(true == setRes)
        {
            bool getRes = true;
            QSqlQuery getBetweenSongIndexFromLocal(m_database);
            QString setIndexs = QString("select idIndex,filepath from 'playlist_%1' WHERE idIndex between '%2' and '%3'")
                                        .arg(inPutStringHandle(playListName)).arg(oldIndex).arg(newIndex);
            getRes &= getBetweenSongIndexFromLocal.exec(setIndexs);

            if(true == getRes)
            {
                bool updateIndexRes = true;
                while(getBetweenSongIndexFromLocal.next())
                {
                    int tempIndex           = getBetweenSongIndexFromLocal.value(0).toInt();
                    QString tempFilepath    = outPutStringHandle(getBetweenSongIndexFromLocal.value(1).toString());

                    QSqlQuery updateSongIndexFromLocal(m_database);
                    QString updateIndex = QString("update 'playlist_%1' set idIndex='%2' WHERE filepath='%3'")
                                                  .arg(inPutStringHandle(playListName)).arg(tempIndex-1).arg(inPutStringHandle(tempFilepath));
                    updateIndexRes &= updateSongIndexFromLocal.exec(updateIndex);
                }

                if(true == updateIndexRes)
                {
                    bool setRes2 = true;
                    QSqlQuery setSongIndexFromLocal2(m_database);
                    QString setIndex2 = QString("update 'playlist_%1' set idIndex='%2' WHERE filepath='%3'")
                                                  .arg(inPutStringHandle(playListName)).arg(newIndex).arg(inPutStringHandle(selectFilePath));
                    setRes2 &= setSongIndexFromLocal2.exec(setIndex2);

                    if(true == setRes2)
                    {
                        return DB_OP_SUCC;
                    }
                    else
                    {
                        return LIST_REORDER_ERR;
                    }
                }
                else
                {
                    return LIST_REORDER_ERR;
                }
            }
        }
    }
    else if(oldIndex > newIndex)//靠后的位置拖拽到了靠前的位置
    {
        bool setRes = true;
        QSqlQuery setSongIndexFromLocal(m_database);
        QString setIndex = QString("update 'playlist_%1' set idIndex=0 WHERE filepath='%2'")
                                   .arg(inPutStringHandle(playListName)).arg(inPutStringHandle(selectFilePath));
        setRes &= setSongIndexFromLocal.exec(setIndex);

        if(true == setRes)
        {
            bool getRes = true;
            QSqlQuery getBetweenSongIndexFromLocal(m_database);
            QString setIndexs = QString("select idIndex,filepath from 'playlist_%1' WHERE idIndex between '%2' and '%3' order by idIndex desc")
                                        .arg(inPutStringHandle(playListName)).arg(newIndex+1).arg(oldIndex);
            getRes &= getBetweenSongIndexFromLocal.exec(setIndexs);

            if(true == getRes)
            {
                bool updateIndexRes = true;
                while(getBetweenSongIndexFromLocal.next())
                {
                    int tempIndex           = getBetweenSongIndexFromLocal.value(0).toInt();
                    QString tempFilepath    = outPutStringHandle(getBetweenSongIndexFromLocal.value(1).toString());

                    QSqlQuery updateSongIndexFromLocal(m_database);
                    QString updateIndex = QString("update 'playlist_%1' set idIndex='%2' WHERE filepath='%3'")
                                                  .arg(inPutStringHandle(playListName)).arg(tempIndex+1).arg(inPutStringHandle(tempFilepath));
                    updateIndexRes &= updateSongIndexFromLocal.exec(updateIndex);
                }

                if(true == updateIndexRes)
                {
                    bool setRes2 = true;
                    QSqlQuery setSongIndexFromLocal2(m_database);
                    QString setIndex2 = QString("update 'playlist_%1' set idIndex='%2' WHERE filepath='%3'")
                                                .arg(inPutStringHandle(playListName)).arg(newIndex+1).arg(inPutStringHandle(selectFilePath));
                    setRes2 &= setSongIndexFromLocal2.exec(setIndex2);

                    if(true == setRes2)
                    {
                        return DB_OP_SUCC;
                    }
                    else
                    {
                        return LIST_REORDER_ERR;
                    }
                }
                else
                {
                    return LIST_REORDER_ERR;
                }
            }
        }
    }
    else//位置没变
    {
        return DB_OP_SUCC;
    }
}

int MusicDataBase::addMusicToHistoryMusic(const QString& filePath)
{
    if(filePath.isEmpty())
    {
        return INVALID_INPUT;
    }
    else
    {

        musicDataStruct temp;
        if(true == m_database.isValid())
        {
            bool queryRes = true;
            //检查歌曲在总表中是否存在
            int checkLocalRes = getSongInfoFromLocalMusic(filePath, temp);

            if(DB_OP_SUCC != checkLocalRes)
            {
                return checkLocalRes;
            }

            //查询历史列表中是否已有该歌曲，已有的话，返回添加失败
            int checkHistoryRes = checkIfSongExistsInHistoryMusic(filePath);

            //历史列表中没有这首歌，直接添加
            if(SONG_NOT_FOUND == checkHistoryRes)
            {
                //历史列表中不存在该歌曲，添加该歌曲
                QSqlQuery addSongToHistory(m_database);
                QString addSongString = QString("insert into %1 (%2) values('%3','%4','%5','%6','%7','%8','%9')").
                        arg(HISTORY).
                        arg(SHOWCONTEXTS).
                        arg(inPutStringHandle(temp.filepath)).
                        arg(inPutStringHandle(temp.title)).
                        arg(inPutStringHandle(temp.singer)).
                        arg(inPutStringHandle(temp.album)).
                        arg(inPutStringHandle(temp.filetype)).
                        arg(inPutStringHandle(temp.size)).
                        arg(inPutStringHandle(temp.time));
                queryRes = addSongToHistory.exec(addSongString);
                //插入歌曲时自增id和idIndex无法赋值，插入后取得自增id，给idIndex赋值
                int tempIndex = addSongToHistory.lastInsertId().toInt();
                bool setRes = true;
                QSqlQuery setSongIDFromLocal(m_database);
                QString setIndex = QString("update %1 set idIndex='%2' WHERE filepath='%3'").
                        arg(HISTORY).arg(tempIndex).arg(inPutStringHandle(temp.filepath));
                setRes &= setSongIDFromLocal.exec(setIndex);

                if(true == (queryRes&setRes))
                {
                    return DB_OP_SUCC;
                }
                else
                {
                    qDebug() << "数据库打开，添加失败！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    return DB_OP_ADD_FAILED;
                }
            }
            else//历史列表中已存在该，歌曲，需要先删除再添加
            {
                int delHistoryRes = delMusicFromHistoryMusic(filePath);
                if(DB_OP_SUCC == delHistoryRes)
                {
                    //历史列表中不存在该歌曲，添加该歌曲
                    QSqlQuery addSongToHistory(m_database);
                    QString addSongString = QString("insert into %1 (%2) values('%3','%4','%5','%6','%7','%8','%9')").
                            arg(HISTORY).
                            arg(SHOWCONTEXTS).
                            arg(inPutStringHandle(temp.filepath)).
                            arg(inPutStringHandle(temp.title)).
                            arg(inPutStringHandle(temp.singer)).
                            arg(inPutStringHandle(temp.album)).
                            arg(inPutStringHandle(temp.filetype)).
                            arg(inPutStringHandle(temp.size)).
                            arg(inPutStringHandle(temp.time));
                    queryRes = addSongToHistory.exec(addSongString);
                    //插入歌曲时自增id和idIndex无法赋值，插入后取得自增id，给idIndex赋值
                    int tempIndex = addSongToHistory.lastInsertId().toInt();
                    bool setRes = true;
                    QSqlQuery setSongIDFromLocal(m_database);
                    QString setIndex = QString("update %1 set idIndex='%2' WHERE filepath='%3'").
                                                arg(HISTORY).
                                                arg(tempIndex).
                                                arg(inPutStringHandle(temp.filepath));
                    setRes &= setSongIDFromLocal.exec(setIndex);

                    if(true == (queryRes&setRes))
                    {
                        return DB_OP_SUCC;
                    }
                    else
                    {
                        qDebug() << "数据库打开，添加失败！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                        return DB_OP_ADD_FAILED;
                    }
                }
                else
                {
                    return delHistoryRes;
                }
            }


        }
        else
        {
            qDebug() << "数据库无法打开，请重试！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
            return DB_UNCONNECT;
        }

    }
}

int MusicDataBase::delMusicFromHistoryMusic(const QString& filePath)
{
    bool delRes = true;

    if(filePath.isEmpty())
    {
        return INVALID_INPUT;
    }
    else
    {
        if(m_database.isValid())
        {
            //查询历史列表中是否已有该歌曲，没有的话，返回删除失败
            int checkHistoryRes = checkIfSongExistsInHistoryMusic(filePath);

            if(DB_OP_SUCC == checkHistoryRes)
            {

                QSqlQuery delSongFromHistoryPlayList(m_database);

                QString delSongString = QString("delete from %1 where filepath = '%2'").
                                                arg(HISTORY).
                                                arg(inPutStringHandle(filePath));
                delRes &= delSongFromHistoryPlayList.exec(delSongString);

                if(true == delRes)
                {
                    return DB_OP_SUCC;
                }
                else
                {
                    qDebug() << "数据库打开，删除失败！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    return DB_OP_DEL_FAILED;
                }
            }
            else
            {
                qDebug() << "歌曲不存在！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return checkHistoryRes;
            }
        }
        else
        {
            qDebug() << "数据库无法打开，请重试！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
            return DB_UNCONNECT;
        }
    }
}
int MusicDataBase::getSongInfoFromHistoryMusic(const QString& filePath, musicDataStruct &fileData)
{
    bool getRes = true;

    if(filePath.isEmpty())
    {
        return INVALID_INPUT;
    }
    else
    {
        if(true == m_database.isValid())
        {
            int checkRes = checkIfSongExistsInHistoryMusic(filePath);

            if(checkRes == SONG_NOT_FOUND)
            {
                return SONG_NOT_FOUND;
            }
            else
            {
                QSqlQuery getSongInfoFromHistoryPlayList(m_database);

                QString getSongString = QString("select %1 from %2 where filepath = '%3'")
                        .arg(SHOWCONTEXTS)
                        .arg(HISTORY)
                        .arg(inPutStringHandle(filePath));
                getRes &= getSongInfoFromHistoryPlayList.exec(getSongString);
                if(false == getRes)
                {
                    return DB_OP_GET_FAILED;
                }

                //判断是否找到了对应的歌曲
                if(getSongInfoFromHistoryPlayList.next())
                {
                    fileData.filepath    = outPutStringHandle(getSongInfoFromHistoryPlayList.value(0).toString());
                    fileData.title       = outPutStringHandle(getSongInfoFromHistoryPlayList.value(1).toString());
                    fileData.singer      = outPutStringHandle(getSongInfoFromHistoryPlayList.value(2).toString());
                    fileData.album       = outPutStringHandle(getSongInfoFromHistoryPlayList.value(3).toString());
                    fileData.filetype    = outPutStringHandle(getSongInfoFromHistoryPlayList.value(4).toString());
                    fileData.size        = outPutStringHandle(getSongInfoFromHistoryPlayList.value(5).toString());
                    fileData.time        = outPutStringHandle(getSongInfoFromHistoryPlayList.value(6).toString());

                    return DB_OP_SUCC;
                }
                else
                {
                    return DB_DISORDERD;
                }

            }

        }
        else
        {
            qDebug() << "数据库无法打开，请重试！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
            return DB_UNCONNECT;
        }
    }

}

//使用歌曲的path值，查询歌曲信息(因为所有歌曲都在本地歌单，所以只用歌曲路径)
int MusicDataBase::getSongInfoFromDB(const QString& filePath, musicDataStruct &fileData)
{
    bool getRes = true;

    if(filePath.isEmpty())
    {
        qDebug() << "入参错误" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }
    else
    {
        getRes = getSongInfoFromLocalMusic(filePath, fileData);

        return getRes;
    }
}

int MusicDataBase::getSongInfoListFromDB(QList<musicDataStruct>& resList,const QString& playListName)
{
    bool getRes = true;

    if(playListName.isEmpty())
    {
        qDebug() << "输入歌单名为空" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }
    else
    {
        if(ALLMUSIC == playListName)
        {
            getRes = getSongInfoListFromLocalMusic(resList);
        }
        else if(HISTORY == playListName)
        {
            getRes = getSongInfoListFromHistoryMusic(resList);
        }
        else if(FAV == playListName)
        {
            getRes = getSongInfoListFromPlayList(resList,FAV);
        }
        else
        {
            getRes = getSongInfoListFromPlayList(resList,playListName);
        }
        return getRes;
    }
}

bool MusicDataBase::checkSongIsInFav(const QString& filePath)
{
    int isExist = OUT_OF_RESULT;
    isExist = checkIfSongExistsInPlayList(filePath, FAV);
    if(isExist == DB_OP_SUCC)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int MusicDataBase::getSongInfoListFromHistoryMusic(QList<musicDataStruct>& resList)
{
    if(true == m_database.isValid())
    {
        bool getRes = true;
        QSqlQuery getSongListFromHistoryMusic(m_database);
        QString getSongListString = QString("select filepath from %1 order by idIndex desc").arg(HISTORY);
        getRes = getSongListFromHistoryMusic.exec(getSongListString);

        if(true == getRes)
        {
            while(getSongListFromHistoryMusic.next())
            {
                musicDataStruct temp;
                temp.filepath = outPutStringHandle(getSongListFromHistoryMusic.value(0).toString());
                int curRes = getSongInfoFromHistoryMusic(temp.filepath, temp);

                if(DB_OP_SUCC == curRes)
                {
                    resList.append(temp);
                }
                else
                {
                    return curRes;
                }
            }

            return DB_OP_SUCC;
        }
        else
        {
            return DB_OP_GET_FAILED;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

//清空历史歌单
int MusicDataBase::emptyHistoryMusic()
{
    if(true == m_database.isValid())
    {
        bool delRes = true;

        QSqlQuery delPlayList(m_database);
        QString delPlayListString = QString("DROP TABLE %1").arg(HISTORY);
        delRes &= delPlayList.exec(delPlayListString);

        if(true == delRes)
        {
            bool queryRes = true;
            QSqlQuery queryInit(m_database);

            queryRes &= queryInit.exec(QString("create table if not exists %1 ("
                                               "id integer primary key autoincrement,"
                                               "idIndex integer unique,"
                                               "filepath varchar unique not NULL,"
                                               "title varchar,"
                                               "singer varchar,"
                                               "album varchar,"
                                               "filetype varchar,"
                                               "size varchar,"
                                               "time varchar)"
                                               ).arg(HISTORY));//创建历史播放列表，自增id为主键，index为唯一值，插入歌曲时为空，获取自增id值后赋值，filepath为唯一值且不为空。
            if(true == queryRes)
            {
                return DB_OP_SUCC;
            }
            else
            {
                return CREATE_HIS_FAILED;
            }
        }
        else
        {
            return EMPTYT_HIS_FAILED;
        }

    }
    else
    {
        qDebug() << "数据库无法打开，请重试！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return DB_UNCONNECT;
    }

}

int MusicDataBase::checkIfSongExistsInLocalMusic(const QString& filePath)
{
    bool queryRes = true;

    QString filePathHash = inPutStringHandle(filePath);

    QSqlQuery getSongFromLocalMusic(m_database);
    QString getSongString = QString("select id from %1 where filepath = '%2'")
                                    .arg(ALLMUSIC)
                                    .arg(filePathHash);
    queryRes = getSongFromLocalMusic.exec(getSongString);

    if(false == queryRes)
    {
        return DB_OP_GET_FAILED;
    }

    //判断总表中是否存在此歌曲，不存在返回错误！
    if(getSongFromLocalMusic.next())
    {
        return DB_OP_SUCC;
    }
    else
    {
        qDebug() << "歌曲未找到" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return SONG_NOT_FOUND;
    }
}

int MusicDataBase::checkIfSongExistsInHistoryMusic(const QString& filePath)
{
    bool queryRes = true;

    QString filePathHash = inPutStringHandle(filePath);

    QSqlQuery getSongFromHistoryMusic(m_database);
    QString getSongString = QString("select id from %1 where filepath = '%2'")
                                    .arg(HISTORY)
                                    .arg(filePathHash);
    queryRes = getSongFromHistoryMusic.exec(getSongString);

    if(false == queryRes)
    {
        return DB_OP_GET_FAILED;
    }

    //判断历史歌单中是否存在此歌曲，不存在返回错误！
    if(getSongFromHistoryMusic.next())
    {
        return DB_OP_SUCC;
    }
    else
    {
        //歌曲未找到
        return SONG_NOT_FOUND;
    }
}
int MusicDataBase::checkIfSongExistsInPlayList(const QString& filePath, const QString& playListName)
{
    if(filePath.isEmpty() || playListName.isEmpty())
    {
        return INVALID_INPUT;
    }

    QString filePathHash = inPutStringHandle(filePath);
    QString playListNameHash = inPutStringHandle(playListName);

    int checkRes = checkPlayListExist(playListName);

    if(DB_OP_SUCC == checkRes)
    {
        bool queryRes = true;

        QSqlQuery getSongFromPlayListMusic(m_database);
        QString getSongString = QString("select id from 'playlist_%1' where filepath = '%2'").arg(playListNameHash).arg(filePathHash);
        queryRes = getSongFromPlayListMusic.exec(getSongString);

        if(false == queryRes)
        {
            return DB_OP_GET_FAILED;
        }

        //判断歌曲列表中是否存在此歌曲，不存在返回错误！
        if(getSongFromPlayListMusic.next())
        {
            return DB_OP_SUCC;
        }
        else
        {
            //歌曲不存在
            return SONG_NOT_FOUND;
        }
    }
    else
    {
        return checkRes;
    }

}

int MusicDataBase::getPlayList(QStringList& playListNameList)
{
    bool getRes = true;

    if(true == m_database.isValid())
    {
//        QMutexLocker lockData( &m_mutex);  //加锁，函数执行完后自动解锁

        QSqlQuery getplayListFromLocalMusic(m_database);
        QString getplayListFromLocalMusicString = QString("select title from ListOfPlayList");
        getRes = getplayListFromLocalMusic.exec(getplayListFromLocalMusicString);

        if(false == getRes)
        {
            return DB_OP_GET_FAILED;
        }

        while(getplayListFromLocalMusic.next())
        {
            playListNameList.append(outPutStringHandle(getplayListFromLocalMusic.value(0).toString()));
        }

        return DB_OP_SUCC;
    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::renamePlayList(const QString& oldPlayListName, const QString& newPlayListName)
{
    if(oldPlayListName.isEmpty() || newPlayListName.isEmpty())
    {
        return INVALID_INPUT;
    }

    if(FAV == oldPlayListName || FAV == newPlayListName)
    {
        return INVALID_INPUT;
    }
    int oldCheckRes = checkPlayListExist(oldPlayListName);
    if(DB_OP_SUCC == oldCheckRes)
    {
        int newCheckRes = checkPlayListExist(newPlayListName);
        if(LIST_NOT_FOUND == newCheckRes)
        {
            //sql支持table重命名
            bool queryRes = true;
            QSqlQuery queryRename(m_database);
            //重命名歌单
            QString renameListString = QString("ALTER TABLE 'playlist_%1' RENAME TO 'playlist_%2'").
                    arg(inPutStringHandle(oldPlayListName)).
                    arg(inPutStringHandle(newPlayListName));
            queryRes &= queryRename.exec(renameListString);
            if(true == queryRes)
            {
                //要把歌单列表中的对应旧歌单名删除，添加新歌单名
                int createRes = createNewPlayList(newPlayListName);
                if(DB_OP_SUCC == createRes)
                {
                    int delRes = delPlayList(oldPlayListName);
                    if(DB_OP_SUCC == delRes)
                    {
                        return DB_OP_SUCC;//歌单名表名和歌单列表中的名称全部改变
                    }
                    else
                    {
                        return LIST_RENAME_ERR;//歌单名已改变，但歌单表中同时存在新歌单名和旧歌单名
                    }
                }
                else
                {
                    return LIST_RENAME_ERR;//歌单名已改变，但歌单表中名未改变
                }
            }
            else
            {
                return LIST_RENAME_FAILED;
            }
        }
        else
        {
            if(DB_OP_SUCC == newCheckRes)
            {
                return LIST_EXISTS;
            }
            else
            {
                return newCheckRes;
            }
        }
    }
    else
    {
        return oldCheckRes;
    }
}

int MusicDataBase::checkPlayListExist(const QString& playListName)
{
    bool getRes = true;

    QStringList playListNameList;

    if(m_database.isValid())
    {
        QSqlQuery getplayListFromLocalMusic(m_database);
        QString getplayListFromLocalMusicString = QString("select title from ListOfPlayList");
        getRes = getplayListFromLocalMusic.exec(getplayListFromLocalMusicString);

        if(false == getRes)
        {
            return DB_OP_GET_FAILED;
        }

        playListNameList.clear();
        while(getplayListFromLocalMusic.next())
        {
            playListNameList.append(outPutStringHandle(getplayListFromLocalMusic.value(0).toString()));
        }

        if(playListNameList.contains(playListName))
        {
            return DB_OP_SUCC;
        }
        else
        {
            //该列表不存在
            return LIST_NOT_FOUND;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

//添加歌曲到新建歌单，使用歌曲的歌名值,歌单名值，输入数据必须有效，
int MusicDataBase::addMusicToPlayList(const QString& filePath,const QString& playListName)
{
    if(filePath.isEmpty() || playListName.isEmpty())
    {
        return INVALID_INPUT;
    }

    int checkRes;
    checkRes = checkPlayListExist(playListName);

    if(DB_OP_SUCC == checkRes)
    {
        int checkLocalRes = checkIfSongExistsInLocalMusic(filePath);

        if(DB_OP_SUCC == checkLocalRes)
        {
            musicDataStruct temp;
            if(true == m_database.isValid())
            {
                //从总表中添加的，故要从总表中取歌曲信息！！！
                //TODO缺少直接添加文件到播放列表的接口
                int getLocalRes = getSongInfoFromLocalMusic(filePath, temp);

                if(DB_OP_SUCC != getLocalRes)
                {
                    return getLocalRes;
                }
                //查询歌单列表中是否已有该歌曲，已有的话，返回添加失败
                int checkPlayListRes = checkIfSongExistsInPlayList(temp.filepath,playListName);

                //歌单列表中已经有这首歌，重复添加了
                if(DB_OP_SUCC == checkPlayListRes)
                {
                    return DB_OP_ADD_REPEAT;
                }

                bool addRes;
                //歌单列表中不存在该歌曲，添加该歌曲
                QSqlQuery addSongToHistory(m_database);
                QString addSongString = QString("insert into 'playlist_%1' (%2) values('%3','%4','%5','%6','%7','%8','%9')").
                        arg(inPutStringHandle(playListName)).
                        arg(SHOWCONTEXTS).
                        arg(inPutStringHandle(temp.filepath)).
                        arg(inPutStringHandle(temp.title)).
                        arg(inPutStringHandle(temp.singer)).
                        arg(inPutStringHandle(temp.album)).
                        arg(inPutStringHandle(temp.filetype)).
                        arg(inPutStringHandle(temp.size)).
                        arg(inPutStringHandle(temp.time));
                addRes = addSongToHistory.exec(addSongString);

                int tempIndex = addSongToHistory.lastInsertId().toInt();
                bool setRes = true;
                QSqlQuery setSongIDFromLocal(m_database);
                QString setIndex = QString("update 'playlist_%1' set idIndex='%2' WHERE filepath='%3'").
                        arg(inPutStringHandle(playListName)).arg(tempIndex).arg(inPutStringHandle(temp.filepath));
                setRes &= setSongIDFromLocal.exec(setIndex);

                if(true == (addRes&setRes))
                {
                    return DB_OP_SUCC;
                }
                else
                {
                    qDebug() << "数据库打开，添加失败！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    return DB_OP_ADD_FAILED;
                }

            }
            else
            {
                qDebug() << "数据库无法打开，请重试！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_UNCONNECT;
            }

        }
        else
        {
            //本地歌单中检查该歌曲失败
            return checkLocalRes;
        }
    }
    else
    {
        //该歌单检查是否存在失败
        return checkRes;
    }

}
//从歌单中添加歌曲到新建歌单，如果本地歌单中没有，同样插入本地歌单，使用musicDataStruct结构,输入数据必须有效，
int MusicDataBase::addNewSongToPlayList(const musicDataStruct& fileData,const QString& playListName)
{
    if(playListName.isEmpty())
    {
        return INVALID_INPUT;
    }
    if(fileData.title.isEmpty() || fileData.filepath.isEmpty())
    {
        qDebug() << "无效入参" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return INVALID_INPUT;
    }

    int checkListRes = checkIfSongExistsInPlayList(fileData.filepath,playListName);
    if(SONG_NOT_FOUND != checkListRes)
    {
        if(DB_OP_SUCC == checkListRes)
        {
            qDebug() << "歌曲重复添加";
            return DB_OP_ADD_REPEAT;
        }
        else
        {
            qDebug() << "检查歌曲是否存在失败，失败码为：" << checkListRes;
            return checkListRes;
        }
    }
    else
    {
        int checkLocalRes = checkIfSongExistsInLocalMusic(fileData.filepath);
        if(checkLocalRes == SONG_NOT_FOUND)
        {
            int addLocalRes;
            addLocalRes = addMusicToLocalMusic(fileData);
            if(DB_OP_SUCC == addLocalRes)
            {
                int addListRes;
                addListRes = addMusicToPlayList(fileData.filepath,playListName);
                return addListRes;
            }
            else
            {
                return addLocalRes;
            }
        }
        else if(checkLocalRes == DB_OP_SUCC)
        {
            int addListRes;
            addListRes = addMusicToPlayList(fileData.filepath,playListName);
            return addListRes;
        }
        else
        {
            return checkLocalRes;
        }
    }



}
//从新建歌单中删除歌曲，使用歌曲的路径值,输入数据必须有效，
int MusicDataBase::delMusicFromPlayList(const QString& filePath,const QString& playListName)
{
    if(filePath.isEmpty() || playListName.isEmpty())
    {
        return INVALID_INPUT;
    }

    int checkRes;
    checkRes = checkPlayListExist(playListName);

    if(DB_OP_SUCC == checkRes)
    {
        //暂时把歌单和本地歌曲分离
//        int checkLocalRes = checkIfSongExistsInLocalMusic(filePath);

//        if(DB_OP_SUCC == checkLocalRes)
//        {
            if(m_database.isValid())
            {
                //查询歌单列表中是否已有该歌曲，没有的话，返回删除失败
                int checkPlayListRes = checkIfSongExistsInPlayList(filePath,playListName);

                //歌单列表中已经有这首歌，可以删除
                if(DB_OP_SUCC == checkPlayListRes)
                {
                    bool delRes;
                    //歌单列表中存在该歌曲，删除该歌曲
                    QSqlQuery delSongToHistory(m_database);
                    QString delSongString = QString("delete from 'playlist_%1' where filepath = '%2'").
                            arg(inPutStringHandle(playListName)).
                            arg(inPutStringHandle(filePath));
                    delRes = delSongToHistory.exec(delSongString);

                    if(true == delRes)
                    {
                        return DB_OP_SUCC;
                    }
                    else
                    {
                        qDebug() << "数据库打开， 删除失败！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                        return DB_OP_DEL_FAILED;
                    }
                }
                else
                {
                    return DB_OP_DEL_FAILED;
                }


            }
            else
            {
                qDebug() << "数据库无法打开，请重试！！！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_UNCONNECT;
            }

//        }
//        else
//        {
//            //本地歌单中检查该歌曲失败
//            return checkLocalRes;
//        }
    }
    else
    {
        //该歌单检查是否存在失败
        return checkRes;
    }
}

int MusicDataBase::getSongIndexFromLocalMusic(const QString& filePath, int &songIndex)
{
    if(filePath.isEmpty())
    {
        return INVALID_INPUT;
    }

    if(true == m_database.isValid())
    {
        bool getRes = true;
        QSqlQuery getSongFromLocalMusic(m_database);
        QString getSongString = QString("select idIndex from %1 where filepath = '%2'")
                                        .arg(ALLMUSIC)
                                        .arg(inPutStringHandle(filePath));
        getRes = getSongFromLocalMusic.exec(getSongString);

        if(true == getRes)
        {
            if(getSongFromLocalMusic.next())
            {
                songIndex    = getSongFromLocalMusic.value(0).toInt();
            }
            else
            {
                qDebug() << "数据库中查无此歌！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                return DB_DISORDERD;
            }
            return DB_OP_SUCC;
        }
        else
        {
            return DB_OP_GET_FAILED;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::getSongIndexFromPlayList(const QString& filePath,const QString& playListName, int &songIndex)
{
    if(filePath.isEmpty())
    {
        return INVALID_INPUT;
    }

    if(true == m_database.isValid())
    {
        int checkRes = checkPlayListExist(playListName);
        if(checkRes == DB_OP_SUCC)
        {
            bool getRes = true;
            QSqlQuery getSongFromPlayList(m_database);
            QString getSongString = QString("select idIndex from 'playlist_%1' where filepath = '%2'").
                    arg(inPutStringHandle(playListName)).arg(inPutStringHandle(filePath));
            getRes = getSongFromPlayList.exec(getSongString);

            if(true == getRes)
            {
                if(getSongFromPlayList.next())
                {
                    songIndex    = getSongFromPlayList.value(0).toInt();
                }
                else
                {
                    qDebug() << "数据库中查无此歌！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    return DB_DISORDERD;
                }
                return DB_OP_SUCC;
            }
            else
            {
                return DB_OP_GET_FAILED;
            }
        }
        else
        {
            return checkRes;
        }

    }
    else
    {
        return DB_DISORDERD;
    }
}

int MusicDataBase::delSongFromEveryWhere(const QString& filePath)
{
    //从历史歌单中删除该歌
    int checkHistoryRes = checkIfSongExistsInHistoryMusic(filePath);
    if(DB_OP_SUCC == checkHistoryRes)
    {
        int delHistoryRes = delMusicFromHistoryMusic(filePath);
        if(DB_OP_SUCC != delHistoryRes)
        {
            return delHistoryRes;
        }
        else
        {}
    }
    else
    {
        if(SONG_NOT_FOUND != checkHistoryRes)
        {
            return checkHistoryRes;
        }
        else
        {}
    }

    //从总歌单中删除该歌
    int checkLocalRes = checkIfSongExistsInLocalMusic(filePath);
    if(DB_OP_SUCC == checkLocalRes)
    {
        int delLocalRes = delMusicFromLocalMusic(filePath);
        if(DB_OP_SUCC != delLocalRes)
        {
            return delLocalRes;
        }
        else
        {}
    }
    else
    {
        if(SONG_NOT_FOUND != checkLocalRes)
        {
            return checkLocalRes;
        }
        else
        {}
    }

    QStringList temp;
    //查询当前已有歌单列表
    int getPlayListRes = getPlayList(temp);
    if(DB_OP_SUCC == getPlayListRes)
    {
        foreach (auto tempList, temp)
        {
            //检查歌曲是否在歌单列表中存在
            int checkPlayListRes = checkIfSongExistsInPlayList(filePath, tempList);
            if(DB_OP_SUCC == checkPlayListRes)
            {
                //从新建歌单中删除歌曲，使用歌曲filePath,歌单名title值，输入数据必须有效，
                int delMusicFromPlayListRes = delMusicFromPlayList(filePath, tempList);
                if(DB_OP_SUCC != delMusicFromPlayListRes)
                {
                    return delMusicFromPlayListRes;
                }
                else
                {}
            }
            else
            {
                if(SONG_NOT_FOUND != checkPlayListRes)
                {
                    return checkPlayListRes;
                }
                else
                {}
            }
        }
    }
    else
    {
        return getPlayListRes;
    }
    //从历史歌单，总歌单，各新建歌单中都删除后，返回成功。
    return DB_OP_SUCC;
}

//删除多首歌曲
int MusicDataBase::delMultiSongs(const QString &playListName, const QStringList &songsList)
{
    //入参检查
    if(playListName.isEmpty())
    {
        return INVALID_INPUT;
    }
    if(0 == songsList.size())
    {
        return INVALID_INPUT;
    }
    foreach (auto songName, songsList)
    {
        if(songName.isEmpty())
        {
            return INVALID_INPUT;
        }
    }
    if(ALLMUSIC == playListName || HISTORY == playListName)
    {

    }
    else
    {
        int playListCheckRes = checkPlayListExist(playListName);
        if(DB_OP_SUCC != playListCheckRes)
        {
            return playListCheckRes;
        }
    }

    foreach (auto songName, songsList)
    {
        int checkIfSongExists = OUT_OF_RESULT;
        if(ALLMUSIC == playListName)
        {
            checkIfSongExists = checkIfSongExistsInLocalMusic(songName);
            if(SONG_NOT_FOUND == checkIfSongExists)
            {
                continue;
            }
        }
        else if(HISTORY == playListName)
        {
            checkIfSongExists = checkIfSongExistsInHistoryMusic(songName);
            if(SONG_NOT_FOUND == checkIfSongExists)
            {
                continue;
            }
        }
        else
        {
            checkIfSongExists = checkIfSongExistsInPlayList(songName, playListName);
            if(SONG_NOT_FOUND == checkIfSongExists)
            {
                continue;
            }
        }


        if(DB_OP_SUCC == checkIfSongExists)
        {
            int delRes = OUT_OF_RESULT;
            if(ALLMUSIC == playListName)
            {
                delRes = delMusicFromLocalMusic(songName);
            }
            else if(HISTORY == playListName)
            {
                delRes = delMusicFromHistoryMusic(songName);
            }
            else
            {
                delRes = delMusicFromPlayList(songName, playListName);
            }

            if(delRes != DB_OP_SUCC)
            {
                return delRes;
            }
            else
            {
                continue;
            }
        }
        else
        {
            return checkIfSongExists;
        }
    }
    return DB_OP_SUCC;
}

int MusicDataBase::checkIfPlayListIsEmpty(const QString& playListName)
{
    if(playListName.isEmpty())
    {
        return INVALID_INPUT;
    }
    int playListCheckRes = checkPlayListExist(playListName);
    if(DB_OP_SUCC != playListCheckRes)
    {
        return playListCheckRes;
    }
    else
    {
        if(true == m_database.isValid())
        {
            bool getRes = true;
            int count = 0;
            QSqlQuery getSongFromPlayList(m_database);
            QString getSongString = QString("select count(title) from 'playlist_%1'").arg(inPutStringHandle(playListName));
            getRes = getSongFromPlayList.exec(getSongString);
            if(true == getRes)
            {
                if(getSongFromPlayList.next())
                {
                    count = getSongFromPlayList.value(0).toInt();
                    if(count > 0)
                    {
                        return LIST_NOT_EMPTY;
                    }
                    else
                    {
                        return LIST_IS_EMPTY;
                    }
                }
                else
                {
                    qDebug() << "数据库中查无此歌！" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    return DB_DISORDERD;
                }
            }
            else
            {
                return DB_OP_GET_FAILED;
            }
        }
    }
}

QString MusicDataBase::inPutStringHandle(const QString& input)
{
    QString temp = input;
    QByteArray text = temp.toUtf8();

    QString output = text.toBase64();
    return output;
}

QString MusicDataBase::outPutStringHandle(const QString& output)
{
    QString temp = output;
    QByteArray text = temp.toUtf8();
    text = QByteArray::fromBase64(text);
    QString outOrigin;
    outOrigin.prepend(text.data());
    return outOrigin;
}

QString MusicDataBase::preHandle(const QString &input)
{
    // 将'替换成''
    QString res = input;
    int iPos = res.indexOf("'", 0);
    while(-1 != iPos){
        res.insert(iPos, QString("'"));
        iPos = res.indexOf("'", iPos + 2);
    }
    return res;
}
