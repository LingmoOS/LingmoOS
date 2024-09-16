// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transferjob.h"
#include "co/log.h"
#include "co/fs.h"
#include "co/path.h"
#include "co/co.h"
#include "common/constant.h"
#include "common/commonstruct.h"
#include "service/fsadapter.h"
#include "service/rpc/sendrpcservice.h"
#include "service/ipc/sendipcservice.h"
#include "utils/config.h"
#include "service/comshare.h"

#include "ipc/bridge.h"

#include <QPointer>
#include <QElapsedTimer>
#include <QStorageInfo>

TransferJob::TransferJob(QObject *parent)
    : QObject(parent)
{
    _status = NONE;
}

TransferJob::~TransferJob()
{
    _status = STOPED;
    if (fx != nullptr) {
        // 主动释放文件句柄，否则取消或异常时在win上有可能导致一直被占用
        LOG << "release fd for file:" << fx->path();
        fx->close();
        delete fx;
        fx = nullptr;
    }
}

bool TransferJob::initRpc(fastring target, uint16 port)
{
    _tar_ip = target;
    _tar_port = port;
    // ip是空，并且是发送那就出现错误了
    if (target.empty() && !_writejob) {
        ELOG << "TransferJob initRpc ip is empty and is push job!! app = " << _app_name;
        return false;
    }
    _remote.reset(new RemoteServiceSender(_app_name.c_str(), _tar_ip.c_str(), _tar_port, true));
    if (!_writejob) {
        FileTransJob req_job;
        req_job.job_id = _jobid;
        req_job.save_path = _savedir.c_str();
        req_job.include_hidden = false;
        req_job.path = _path;
        req_job.sub = _sub;
        req_job.write = (!_writejob);
        req_job.app_who = _tar_app_name;
        req_job.targetAppname = _app_name;
        req_job.ip = Util::getFirstIp();
        SendResult res;
        // 必须等待对方回复了才执行后面的流程
        {
            QMutexLocker g(&_send_mutex);
            res = _remote->doSendProtoMsg(TRANSJOB, req_job.as_json().str().c_str(), QByteArray());
        }

        if (res.errorType < INVOKE_OK) {
            SendStatus st;
            st.type = res.errorType;
            st.msg = res.as_json().str();

            // notifySendStatus
            auto targetAppname = QString(_app_name.c_str());
            QString jsonMsg = st.as_json().str().c_str();
            SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SEND_STATUS, jsonMsg);

            this->_init_success = false;
            return false;
        }
    }
    return true;
}

// 先调用initrpc, 在调用initjob
void TransferJob::initJob(fastring appname, fastring targetappname, int id, fastring path, bool sub, fastring savedir, bool write)
{
    _app_name = appname;
    _tar_app_name = targetappname;
    _jobid = id;
    _path = path;
    _sub = sub;
    _savedir = savedir;
    _writejob = write;
    _status = INIT;
    _save_fulldir = path::join(DaemonConfig::instance()->getStorageDir(_app_name), _savedir);
    if (_writejob) {
        Comshare::instance()->updateStatus(CURRENT_STATUS_TRAN_FILE_RCV);
        fastring fullpath = _save_fulldir;
        FSAdapter::newFileByFullPath(fullpath.c_str(), true);
    } else {
        Comshare::instance()->updateStatus(CURRENT_STATUS_TRAN_FILE_SEN);
    }
}

void TransferJob::setFileName(const fastring &name, const fastring &acName)
{
    QWriteLocker lk(&_file_name_maps_lock);
    _file_name_maps.remove(name);
    _file_name_maps.insert(name, acName);
}

fastring TransferJob::acName(const fastring &name)
{
    QReadLocker lk(&_file_name_maps_lock);
    return _file_name_maps.value(name);
}

fastring TransferJob::getSaveFullpath(const fastring &rootdir, const fastring &filename)
{
    // 第一层子目录已存在则尝试获取已重命名后的名字，比如 abc/ddd/eee.txt -> abc(1)/ddd/eee.txt
    auto acfilename = filename;
    if (acfilename.contains("/")) {
        auto first = acfilename.substr(0, acfilename.find_first_of("/"));
        auto acFirst = acName(first);
        if (!acFirst.empty()) {
            //已经重命名，组合为新的保存路径
            acfilename = acFirst.c_str() + acfilename.substr(acfilename.find_first_of("/"));
        }
    } else {
        auto reName = acName(filename);
        if (!reName.empty()) {
            //已经重命名
            acfilename = reName;
        }
    }

    fastring fullpath = path::join(rootdir, acfilename);
    return fullpath;
}

bool TransferJob::createFile(const fastring fullpath, const bool isDir)
{
    if (fullpath.empty()) {
        ELOG << "Try create file with empty path: " << fullpath;
        return false;
    }

    return FSAdapter::newFileByFullPath(fullpath.c_str(), isDir);
}

void TransferJob::start()
{
    atomic_store(&_status, STARTED);
    _mark_canceled = false;

    if (_writejob) {
        DLOG << "start write job: " << _savedir << " fullpath = " << _save_fulldir;
        handleJobStatus(JOB_TRANS_DOING);
        QStorageInfo info(_save_fulldir.c_str());
        _device_free_size = info.bytesFree();
    } else {
        // 读取所有文件的信息
        DLOG << "doTransfileJob path to save:" << _savedir;
        //并行读取文件数据
        UNIGO([this]() {
            co::Json pathJson;
            pathJson.parse_from(_path);
            for (uint32 i = 0; i < pathJson.array_size(); i++) {
                fastring jobpath = pathJson[i].as_string();
                std::pair<fastring, fastring> pairs = path::split(jobpath);
                fastring rootpath = pairs.first.c_str();
                scanPath(rootpath, jobpath, true);
            }
            QSharedPointer<FSDataBlock> blocks(new FSDataBlock);
            blocks->job_id = _jobid;
            blocks->data_size = _total_size;
            // copy binrary data
            blocks->flags = JobTransFileOp::FILE_COUNTED;
            pushQueque(blocks);

            DLOG << "read job start path: " << pathJson;
            for (uint32 i = 0; i < pathJson.array_size(); i++) {
                fastring jobpath = pathJson[i].as_string();
                std::pair<fastring, fastring> pairs = path::split(jobpath);
                fastring rootpath = pairs.first.c_str();
                scanPath(rootpath, jobpath, false);
            }
            QSharedPointer<FSDataBlock> block(new FSDataBlock);
            block->job_id = _jobid;
            block->data_size = 0;
            // copy binrary data
            block->flags = JobTransFileOp::FILE_TRANS_OVER;
            pushQueque(block);
        });
    }

    // 开始循环处理数据块
    handleBlockQueque();

    // 自己完成了
    co::sleep(100);
    emit notifyJobFinished(_jobid);
}

void TransferJob::stop(const bool notify)
{
    _not_notify = !notify;
    DLOG << "(" << _jobid << ") stop now!";
    atomic_store(&_status, STOPED);
}

void TransferJob::waitFinish()
{
    DLOG << "(" << _jobid << ") wait write finish!";
    atomic_store(&_status, WAIT_DONE);
}

bool TransferJob::ended()
{
    return unlikely(_status == STOPED);
}

bool TransferJob::isRunning()
{
    return unlikely(_status == STARTED);
}

bool TransferJob::isWriteJob()
{
    return _writejob;
}

fastring TransferJob::getAppName()
{
    return _app_name;
}

void TransferJob::cancel(bool notify)
{
    DLOG << "job cancel: " << notify;
    _mark_canceled = true;
    if (notify) {
        atomic_cas(&_status, STARTED, CANCELING);   // 如果运行，则赋值取消
    } else {
        handleJobStatus(JOB_TRANS_CANCELED);   // 通知前端应用作业已取消
        atomic_store(&_status, STOPED);
    }
}

void TransferJob::pushQueque(const QSharedPointer<FSDataBlock> block)
{
    QWriteLocker g(&_queque_mutex);
    if (_status == CANCELING || _status == STOPED) {
        DLOG << "This job has mark cancel or stoped, stop handle data.";
        return;
    }

    if (_writejob) {
        // 更新对端发送文件根目录为本地保存目录
        block->rootdir.clear();
        block->rootdir = (_save_fulldir);
    }

    _block_queue.enqueue(block);
}

qint64 TransferJob::freeBytes() const
{
    return _device_free_size;
}

bool TransferJob::offlineCancel(const QString &ip)
{
    if (_offlined || ip.isEmpty() || ip != QString(_tar_ip.c_str()))
        return false;
    _offlined = true;
    handleJobStatus(JOB_TRANS_FAILED);
    return true;
}

fastring TransferJob::getSubdir(const char *path, const char *root)
{
    fastring indir = "";
    std::pair<fastring, fastring> pairs = path::split(fastring(path));
    fastring filedir = pairs.first;
    indir = filedir.size() > strlen(root) ? filedir.remove_prefix(root) : "";
    fastring subdir = path::join(indir.c_str(), "");
    return subdir;
}

void TransferJob::handleBlockQueque()
{
    bool exception = false;
    bool counted = false;
    QElapsedTimer time;
    time.start();
    int64 timeold = 0;
    if (!_writejob)
        createSendCounting();
    // 发送当前统计文件中block
    while (_status != STOPED) {
        if (_status == CANCELING) {
            exception = true;
            break;
        }
        // 计时器处理
        bool timeout = false;
        if (time.elapsed() - timeold >= 500) {
            timeold = time.elapsed();
            timeout = true;
        }
        auto block = popQueue();
        if (block.isNull() && (_writejob || (!timeout && !counted))) {
            co::sleep(10);
            continue;
        }

        if (block.isNull()) {
            block.reset(new FSDataBlock);
            block->flags = JobTransFileOp::FILE_COUNTING;
        }

        if (counted == false)
            counted = block->flags & JobTransFileOp::FILE_COUNTED;

        fastring fullpath = "";
        bool needReacquire = (_jobid == 1000) && ((block->flags & JobTransFileOp::FIlE_DIR_CREATE)
                                                  || (block->flags & JobTransFileOp::FIlE_CREATE));
        bool reacquired = false;
        //加解密文件名，防止路径特殊导致序列化卡住
        if (!block->filename.empty()) {
            fastring filename = _writejob ? Util::decodeBase64(block->filename.c_str())
                                          : Util::encodeBase64(block->filename.c_str());

            //真实全路径，写文件和通知
            fullpath = getSaveFullpath(block->rootdir, _writejob ? filename : block->filename);
            if (needReacquire) {
                // 跨端：需重新命名已存在文件
                fastring newPath;
                reacquired = FSAdapter::reacquirePath(fullpath, &newPath);
                if (reacquired) {
                    fullpath = newPath;
                }
            }

            // 重新赋加密解密名字
            block->filename = (filename);
        }
        if (_writejob) {
            if (block->flags & JobTransFileOp::FILE_COUNTING)
                continue;
            // 写入失败，怎么处理，继续尝试
            exception = !writeAndCreateFile(block, fullpath);

            if (reacquired && !exception) {
                //创建文件成功，记录文件名对应的新名称
                auto ft = fullpath.replace(block->rootdir + "/", "");
                DLOG << "record: " << block->filename << " to:" << ft;
                setFileName(block->filename, ft);
            }
        } else {
            // 发送失败，怎么处理
            exception = !sendToRemote(block);
            if (exception)
                WLOG << "sendToRemote exception!!!!";
        }

        if (exception || _offlined) {
            DLOG << "trans job exception hanpend: " << _jobid << " exception? " << exception << " offlined? " << _offlined;
            handleJobStatus(JOB_TRANS_FAILED);
            break;
        }

        if (block->flags & JobTransFileOp::FILE_TRANS_OVER) {
            DLOG << "transfer end ::: all file read or write over !!!";
            break;
        }

        // 通知前端进度
        {
            FileInfo info;
            info.job_id = _jobid;
            info.file_id = _notify_fileid;
            info.total_size = _total_size;
            info.current_size = _cur_size;
            info.name = fullpath;
            info.time_spended = time.elapsed();

            if (block->flags & JobTransFileOp::FILE_CLOSE) {
                handleTransStatus(FILE_TRANS_END, info);
            } else if (block->flags & JobTransFileOp::FIlE_CREATE) {
                handleTransStatus(FILE_TRANS_IDLE, info);
            }

            if (timeout && counted) {
                handleTransStatus(FILE_TRANS_SPEED, info);
            }
        }
    };
    if (!_not_notify && !exception && !_mark_canceled && !_offlined) {
        handleJobStatus(JOB_TRANS_FINISHED);
    }
    LOG << "trans job end: " << _jobid << " freebytes = " << _device_free_size
        << "  not enought = " << _device_not_enough;
    atomic_store(&_status, STOPED);
}

void TransferJob::handleUpdate(FileTransRe result, const char *path, const char *emsg)
{
    FileTransJobReport report;
    report.job_id = (_jobid);
    report.path = (path);
    report.result = (result);
    report.error = (emsg);
    SendResult res;
    // 必须等待对方回复了才执行后面的流程
    {
        QMutexLocker g(&_send_mutex);
        res = _remote->doSendProtoMsg(FS_REPORT,
                                      report.as_json().str().c_str(), QByteArray());
    }
}

void TransferJob::handleJobStatus(int status)
{
    QString appname(_app_name.c_str());
    fastring fullpath = path::join(
                DaemonConfig::instance()->getStorageDir(_app_name), _savedir);
    QString savepath(fullpath.c_str());
    if (_device_not_enough)
        savepath += "::not enough";

    if (_offlined)
        savepath += "::off line";

    emit notifyJobResult(appname, _jobid, status, savepath);
}

void TransferJob::handleTransStatus(int status, const FileInfo &info)
{
    co::Json filejson = info.as_json();
    QString appname(_app_name.c_str());
    QString fileinfo(filejson.str().c_str());

    // FileInfo > FileStatus in handle func
    // DLOG << "handleTransStatus  =  " << appname.toStdString() << " status = " << status
    //      << " file info ===== " << filejson;
    emit notifyFileTransStatus(appname, status, fileinfo);
}

QSharedPointer<FSDataBlock> TransferJob::popQueue()
{
    QWriteLocker g(&_queque_mutex);
    if (_block_queue.empty())
        return nullptr;
    return _block_queue.dequeue();
}

int TransferJob::queueCount() const
{
    QReadLocker g(&_queque_mutex);
    return _block_queue.count();
}

void TransferJob::scanPath(const fastring root, const fastring path, const bool acTotal)
{
#ifdef linux
    // 链接文件不拷贝
    if (fs::isSymlink(path.c_str()))
        return;
#endif
    _fileid++;
    fastring subdir = getSubdir(path.c_str(), root.c_str());
    FileEntry *entry = new FileEntry();
    if (FSAdapter::getFileEntry(path.c_str(), &entry) < 0) {
        ELOG << "get file entry error !!!!";
        cancel();
        return;
    }

    if (_status >= STOPED)
        return;
    if (fs::isdir(path.c_str())) {
        _total_size += 4096;
        if (!acTotal) {
            QSharedPointer<FSDataBlock> block(new FSDataBlock);
            block->job_id = _jobid;
            block->rootdir = root;
            auto file = path;
            block->filename = file.replace(root, "");
            block->blk_id = 0;
            block->flags = JobTransFileOp::FIlE_DIR_CREATE;
            block->data_size = 0;
            pushQueque(block);
        }
        // 发送文件创建操作
        readPath(path, root, acTotal);
    } else {
        readFile(path, _fileid, subdir.c_str(), acTotal);
    }
}

void TransferJob::readPath(fastring path, fastring root, const bool acTotal)
{
    if (_status >= STOPED)
        return;

    fastring dirpath = path::join(path, "");
    fs::dir d(dirpath);
    auto v = d.all();   // 读取所有子项
    for (const fastring &file : v) {
        if (_status >= STOPED)
            return;
        fastring file_path = path::join(d.path(), file.c_str());
        scanPath(root, file_path, acTotal);
    }
}

bool TransferJob::readFile(fastring filepath, int fileid, fastring subdir, const bool acTotal)
{
    if (_status >= STOPED)
        return false;

    std::pair<fastring, fastring> pairs = path::split(fastring(filepath));

    fastring filename = pairs.second;

    fastring subname = path::join(subdir, filename.c_str());
    readFileBlock(filepath, fileid, subname, acTotal);
    return true;
}

void TransferJob::readFileBlock(fastring filepath, int fileid, const fastring subname, const bool acTotal)
{
    if (filepath.empty() || fileid < 0 || !fs::exists(filepath)) {
        ELOG << "readFileBlock file is invaild" << filepath;
        return;
    }

    if (_status >= STOPED)
        return;

    size_t block_size = BLOCK_SIZE;
    int64 file_size = fs::fsize(filepath);

    if (acTotal) {
        _total_size += static_cast<int64>(file_size <= 0 ? 4096 : file_size);
        return;
    }

    // 文件所在根目录
    auto file = filepath;
    auto root = file.replace(subname, "");

    if (file_size <= 0) {
        // error file or 0B file.
        QSharedPointer<FSDataBlock> block(new FSDataBlock);
        block->job_id = _jobid;
        block->file_id = fileid;
        block->rootdir = root;
        block->filename = subname;
        block->blk_id = 0;
        block->data_size = 0;
        block->flags = JobTransFileOp::FIlE_CREATE;
        pushQueque(block);
        return;
    }
    QPointer<TransferJob> self = this;
    int64 read_size = 0;
    uint32 block_id = 0;

    fs::file fd(filepath, 'r');
    if (!fd) {
        ELOG << "open file failed: " << filepath;
        return;
    }

    size_t block_len = block_size * sizeof(char);
    char *buf = reinterpret_cast<char *>(malloc(block_len));
    size_t resize = 0;
    bool open = true;
    do {
        // 最多100个数据块->100M 限制内存使用
        if (self && self->queueCount() > 100) {
            co::sleep(10);
            continue;
        }

        if (self.isNull() || self->_status >= STOPED)
            break;

        memset(buf, 0, block_len);
        resize = fd.read(buf, block_size);
        if (resize > block_size) {
            LOG << "read file ERROR  resize = " << resize;
            break;
        }

        QSharedPointer<FSDataBlock> block(new FSDataBlock);
        if (self)
            block->job_id = self->_jobid;
        block->file_id = fileid;
        block->rootdir = root;
        block->filename = subname;
        block->blk_id = block_id;
        // 判断文件刚开始读取
        block->flags = open ? JobTransFileOp::FIlE_CREATE : JobTransFileOp::FIlE_NONE;
        // 判断文件是否读取完成
        block->flags = (resize == 0 || read_size + static_cast<int64>(resize) >= file_size) ? block->flags | JobTransFileOp::FILE_CLOSE : block->flags;
        block->data_size = static_cast<int64>(resize);
        // copy binrary data
        fastring bufdata(buf, resize);
        block->data = bufdata;
        if (self)
            self->pushQueque(block);
        open = false;

        if (resize == 0 || read_size + static_cast<int64>(resize) >= file_size) {
            LOG << "read file End resize = " << resize;
            break;
        }

        read_size += resize;
        block_id++;
    } while (read_size < file_size || (resize > 0 && resize == block_size));

    free(buf);
    fd.close();
}

bool TransferJob::writeAndCreateFile(const QSharedPointer<FSDataBlock> block, const fastring fullpath)
{
    _notify_fileid = block->file_id;
    // 创建文件
    if (block->flags & JobTransFileOp::FIlE_DIR_CREATE) {
        if (!createFile(fullpath, true))
            return false;
        _cur_size += 4096;
        return true;
    } else if (block->flags & JobTransFileOp::FILE_COUNTED) {
        _total_size = block->data_size;
        // 磁盘空间不足判断
        if (_total_size >= _device_free_size) {
            // 通知发送端磁盘空间不足
            _device_not_enough = true;
            return false;
        }
        return true;
    } else if (block->flags & JobTransFileOp::FILE_COUNTING || block->flags & JobTransFileOp::FILE_TRANS_OVER) {
        return true;
    }

    fastring buffer = block->data;
    size_t len = buffer.size();
    int64 offset = static_cast<int64>(block->blk_id * BLOCK_SIZE);
    // ELOG << "file : " << name << " write : " << len << " totol = " << _total_size << " curent " <<  _cur_size
    //      << "  flags !!! " << block->flags;
    int count = 3;
    bool good = false;
    do {
        good = FSAdapter::writeBlock(fullpath.c_str(), offset, buffer.c_str(), len, block->flags, &fx);
        count--;
    } while(!good && count > 0);


    if (!good) {
        ELOG << "file : " << fullpath << " write BLOCK error";
    } else {
        if (len == 0 && block->flags & JobTransFileOp::FIlE_CREATE) {
            _cur_size += 4096;
        } else {
            _cur_size += static_cast<int64>(len);
        }
    }
    return good;
}

bool TransferJob::sendToRemote(const QSharedPointer<FSDataBlock> block)
{
    if (_device_not_enough)
        return false;
    FileTransBlock file_block;
    file_block.job_id = (_jobid);
    file_block.file_id = (block->file_id);
    file_block.rootdir = ""; // 重置文件根目录
    file_block.filename = (block->filename.c_str());
    file_block.blk_id = (static_cast<uint>(block->blk_id));
    file_block.flags = block->flags;
    file_block.data_size = block->data_size;
    _notify_fileid = block->file_id;
    fastring buffer = block->data;
    QByteArray data(buffer.c_str(), static_cast<int>(block->data.empty() ? 0 : block->data_size));
    // DLOG << "( ==== " << _jobid << ") send block " << block->filename << " size: " << block->data_size
    //     << " ----- = " << queueCount() << "  flags  == " << block->flags;
    SendResult res;
    // 必须等待对方回复了才执行后面的流程
    {
        res.errorType = 0;
        QMutexLocker g(&_send_mutex);
        res = _remote->doSendProtoMsg(FS_DATA, file_block.as_json().str().c_str(), data);
    }
    co::Json resJson;
    if (res.protocolType == FS_DATA && resJson.parse_from(res.data)) {
        FileTransResponse transres;
        transres.from_json(resJson);
        if (transres.result == IO_ERROR) {
            DLOG << "remote return: IO_ERROR!";
            _device_not_enough = block->flags & JobTransFileOp::FILE_COUNTED;
            return false;
        }
    } else {
        WLOG << "remote return type: " << res.protocolType << " data: \n" << res.data;
    }

    if (res.errorType < INVOKE_OK && !_offlined) {
        WLOG << "sendToRemote invoke fail";

        SendStatus st;
        st.type = res.errorType;
        st.msg = res.as_json().str();

        // notifySendStatus
        auto targetAppname = QString(_app_name.c_str());
        QString jsonMsg = st.as_json().str().c_str();
        SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SEND_STATUS, jsonMsg);


        offlineCancel(_tar_ip.c_str());
        return false;
    }

    if (block->data_size == 0 && block->flags & JobTransFileOp::FIlE_CREATE) {
        _cur_size += 4096;
    } else {
        if (block->flags & JobTransFileOp::FILE_COUNTED) {
            // 跳过计算结果的total_size.
            DLOG << "FILE_COUNTED: skip + " << block->data_size;
        } else {
            _cur_size += static_cast<int64>(block->data_size);
        }
    }
    return true;
}

void TransferJob::createSendCounting()
{
    QSharedPointer<FSDataBlock> block(new FSDataBlock);
    block->job_id = _jobid;
    block->flags = JobTransFileOp::FILE_COUNTING;
    block->data_size = 0;
    pushQueque(block);
}
