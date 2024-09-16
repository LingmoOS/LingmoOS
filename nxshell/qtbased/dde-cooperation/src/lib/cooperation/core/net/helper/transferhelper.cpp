// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transferhelper.h"
#include "transferhelper_p.h"
#include "configs/settings/configmanager.h"
#include "utils/cooperationutil.h"
#include "utils/historymanager.h"
#include "net/networkutil.h"
#include "net/cooconstrants.h"

#include "common/constant.h"
#include "common/commonutils.h"

#include <QDesktopServices>
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTime>
#include <QProcess>

#ifdef linux
#    include "base/reportlog/reportlogmanager.h"
#    include <QDBusInterface>
#    include <QDBusReply>
#endif

using ButtonStateCallback = std::function<bool(const QString &, const DeviceInfoPointer)>;
using ClickedCallback = std::function<void(const QString &, const DeviceInfoPointer)>;
Q_DECLARE_METATYPE(ButtonStateCallback)
Q_DECLARE_METATYPE(ClickedCallback)

inline constexpr int TransferJobStartId = 1000;

inline constexpr char HistoryButtonId[] { "history-button" };
inline constexpr char TransferButtonId[] { "transfer-button" };

inline constexpr char NotifyCancelAction[] { "cancel" };
inline constexpr char NotifyRejectAction[] { "reject" };
inline constexpr char NotifyAcceptAction[] { "accept" };
inline constexpr char NotifyCloseAction[] { "close" };
inline constexpr char NotifyViewAction[] { "view" };

using TransHistoryInfo = QMap<QString, QString>;
Q_GLOBAL_STATIC(TransHistoryInfo, transHistory)

#ifdef linux
inline constexpr char Khistory[] { "history" };
inline constexpr char Ksend[] { "send" };
#else
inline constexpr char Khistory[] { ":/icons/deepin/builtin/texts/history_18px.svg" };
inline constexpr char Ksend[] { ":/icons/deepin/builtin/texts/send_18px.svg" };
#endif

using namespace deepin_cross;
using namespace cooperation_core;

TransferHelperPrivate::TransferHelperPrivate(TransferHelper *qq)
    : QObject(qq),
      q(qq)
{
    *transHistory = HistoryManager::instance()->getTransHistory();
    connect(HistoryManager::instance(), &HistoryManager::transHistoryUpdated, q,
            [] {
                *transHistory = HistoryManager::instance()->getTransHistory();
            });

    initConnect();
}

TransferHelperPrivate::~TransferHelperPrivate()
{
}

void TransferHelperPrivate::initConnect()
{
#ifdef __linux__
    notice = new NoticeUtil(q);
    connect(notice, &NoticeUtil::onConfirmTimeout, q, &TransferHelper::onVerifyTimeout);
    connect(notice, &NoticeUtil::ActionInvoked, q, &TransferHelper::onActionTriggered);
#endif

    confirmTimer.setInterval(10 * 1000);
    confirmTimer.setSingleShot(true);
    connect(&confirmTimer, &QTimer::timeout, q, &TransferHelper::onVerifyTimeout);
}

CooperationTransDialog *TransferHelperPrivate::transDialog()
{
    if (!dialog) {
        dialog = new CooperationTransDialog(qApp->activeWindow());
        dialog->setModal(true);

        connect(dialog, &CooperationTransDialog::cancelApply, q, &TransferHelper::cancelTransferApply);
        connect(dialog, &CooperationTransDialog::cancel, q, [this] { q->onActionTriggered(NotifyCancelAction); });
        connect(dialog, &CooperationTransDialog::rejected, q, [this] { q->onActionTriggered(NotifyRejectAction); });
        connect(dialog, &CooperationTransDialog::accepted, q, [this] { q->onActionTriggered(NotifyAcceptAction); });
        connect(dialog, &CooperationTransDialog::viewed, q, [this] { q->onActionTriggered(NotifyViewAction); });
    }

    return dialog;
}

void TransferHelperPrivate::reportTransferResult(bool result)
{
#ifdef __linux__
    QVariantMap map;
    map.insert("deliveryResult", result);
    ReportLogManager::instance()->commit(ReportAttribute::FileDelivery, map);
#endif
}

void TransferHelperPrivate::notifyMessage(const QString &body, const QStringList &actions, int expireTimeout)
{
#ifdef __linux__
    notice->notifyMessage(tr("File transfer"), body, actions, QVariantMap(), expireTimeout);
#endif
}

TransferHelper::TransferHelper(QObject *parent)
    : QObject(parent),
      d(new TransferHelperPrivate(this))
{
}

TransferHelper::~TransferHelper()
{
}

TransferHelper *TransferHelper::instance()
{
    static TransferHelper ins;
    return &ins;
}

void TransferHelper::registBtn()
{
    ClickedCallback clickedCb = TransferHelper::buttonClicked;
    ButtonStateCallback visibleCb = TransferHelper::buttonVisible;
    ButtonStateCallback clickableCb = TransferHelper::buttonClickable;
    QVariantMap historyInfo { { "id", HistoryButtonId },
                              { "description", tr("View transfer history") },
                              { "icon-name", Khistory },
                              { "location", 2 },
                              { "button-style", 0 },
                              { "clicked-callback", QVariant::fromValue(clickedCb) },
                              { "visible-callback", QVariant::fromValue(visibleCb) },
                              { "clickable-callback", QVariant::fromValue(clickableCb) } };

    QVariantMap transferInfo { { "id", TransferButtonId },
                               { "description", tr("Send files") },
                               { "icon-name", Ksend },
                               { "location", 3 },
                               { "button-style", 1 },
                               { "clicked-callback", QVariant::fromValue(clickedCb) },
                               { "visible-callback", QVariant::fromValue(visibleCb) },
                               { "clickable-callback", QVariant::fromValue(clickableCb) } };

    CooperationUtil::instance()->registerDeviceOperation(historyInfo);
    CooperationUtil::instance()->registerDeviceOperation(transferInfo);
}

void TransferHelper::sendFiles(const QString &ip, const QString &devName, const QStringList &fileList)
{
    d->who = devName;
    d->readyToSendFiles = fileList;
    if (fileList.isEmpty())
        return;

    if (!d->status.testAndSetRelease(TransferHelper::Idle, TransferHelper::Connecting)) {
        d->status.storeRelease(TransferHelper::Idle);
        return;
    }

    // send the transfer file RPC request
    NetworkUtil::instance()->tryTransApply(ip);

    waitForConfirm();
}

TransferHelper::TransferStatus TransferHelper::transferStatus()
{
    return static_cast<TransferStatus>(d->status.loadAcquire());
}

void TransferHelper::buttonClicked(const QString &id, const DeviceInfoPointer info)
{
    QString ip = info->ipAddress();
    QString name = info->deviceName();
    LOG << "button clicked, button id: " << id.toStdString()
        << " ip: " << ip.toStdString()
        << " device name: " << name.toStdString();

    if (id == TransferButtonId) {
        QStringList selectedFiles = qApp->property("sendFiles").toStringList();
        if (selectedFiles.isEmpty())
            selectedFiles = QFileDialog::getOpenFileNames(qApp->activeWindow());

        if (selectedFiles.isEmpty())
            return;

        if (qApp->property("onlyTransfer").toBool()) {
            // send command to local socket.
            QStringList msgs;
            // must be in the format of: <ip> <name> <files>
            msgs << "-f" << ip << name << selectedFiles;
            emit TransferHelper::instance()->deliverMessage(MainAppName, msgs);
            qApp->exit(0);
        } else {
            TransferHelper::instance()->sendFiles(ip, name, selectedFiles);
        }
    } else if (id == HistoryButtonId) {
        if (!transHistory->contains(ip))
            return;

        QDesktopServices::openUrl(QUrl::fromLocalFile(transHistory->value(ip)));
    }
}

bool TransferHelper::buttonVisible(const QString &id, const DeviceInfoPointer info)
{
    if (id == TransferButtonId) {
        switch (info->transMode()) {
        case DeviceInfo::TransMode::Everyone:
            return info->connectStatus() != DeviceInfo::Offline;
        case DeviceInfo::TransMode::OnlyConnected:
            return info->connectStatus() == DeviceInfo::Connected;
        default:
            return false;
        }
    }

    if (id == HistoryButtonId) {
        if (qApp->property("onlyTransfer").toBool())
            return false;

        if (!transHistory->contains(info->ipAddress()))
            return false;

        bool exists = QFile::exists(transHistory->value(info->ipAddress()));
        if (!exists)
            HistoryManager::instance()->removeTransHistory(info->ipAddress());

        return exists;
    }

    return true;
}

bool TransferHelper::buttonClickable(const QString &id, const DeviceInfoPointer info)
{
    Q_UNUSED(info)

    if (id == TransferButtonId)
        return TransferHelper::instance()->transferStatus() == Idle;

    return true;
}

void TransferHelper::onVerifyTimeout()
{
    d->isTransTimeout = true;
    if (d->status.loadAcquire() != TransferHelper::Confirming)
        return;

    d->status.storeRelease(Idle);
    d->transDialog()->showResultDialog(false, tr("The other party did not receive, the files failed to send"));
}

void TransferHelper::transferResult(bool result, const QString &msg)
{
#ifdef __linux__
    if (d->role != Server) {
        QStringList actions;
        if (result)
            actions << NotifyViewAction << tr("View");
        d->notifyMessage(msg, actions, 3 * 1000);
        return;
    }
#endif

    d->transDialog()->showResultDialog(result, msg, d->role != Server);
    d->reportTransferResult(result);
}

void TransferHelper::updateProgress(int value, const QString &remainTime)
{
#ifdef __linux__
    if (d->role != Server) {
        // 在通知中心中，如果通知内容包含“%”且actions中存在“cancel”，则不会在通知中心显示
        QStringList actions { NotifyCancelAction, tr("Cancel") };
        // dde-session-ui 5.7.2.2 版本后，支持设置该属性使消息不进通知中心
        QVariantMap hitMap { { "x-deepin-ShowInNotifyCenter", false } };
        QString msg(tr("File receiving %1% | Remaining time %2").arg(QString::number(value), remainTime));

        d->notifyMessage(msg, actions, 15 * 1000);
        return;
    }
#endif

    QString title = d->role == Server ? tr("Sending files to \"%1\"") : tr("Receiving files from \"%1\"");

    title = title.arg(CommonUitls::elidedText(d->who, Qt::ElideMiddle, 15));
    d->transDialog()->showProgressDialog(title);
    d->transDialog()->updateProgress(value, remainTime);
}

void TransferHelper::onActionTriggered(const QString &action)
{
    // clear transfer info
    d->transferInfo.clear();
    if (action == NotifyCancelAction) {
        cancelTransfer(true); // do UI first
        NetworkUtil::instance()->cancelTrans();
    } else if (action == NotifyRejectAction) {
        NetworkUtil::instance()->replyTransRequest(false);
    } else if (action == NotifyAcceptAction) {
        d->role = Client;
        NetworkUtil::instance()->replyTransRequest(true);
    } else if (action == NotifyCloseAction) {
#ifdef __linux__
        d->notice->closeNotification();
#endif
    } else if (action == NotifyViewAction) {
        auto fileurl = d->recvFilesSavePath;
        if (fileurl.isEmpty()) {
            auto value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::StoragePathKey);
            fileurl = value.isValid() ? value.toString() : QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        }

        openFileLocation(fileurl);
    }
}

void TransferHelper::openFileLocation(const QString &path)
{
#ifdef __linux__
    QProcess::execute("dde-file-manager", QStringList() << path);
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    d->transDialog()->close();
#endif
}

void TransferHelper::notifyTransferRequest(const QString &nick, const QString &ip)
{
    DLOG << "request info: " << nick.toStdString() << ip.toStdString();
    auto storageFolder = nick + "(" + ip + ")";
    NetworkUtil::instance()->setStorageFolder(storageFolder);

    static QString msg(tr("\"%1\" send some files to you"));
    d->who = nick;
#ifdef __linux__

    QStringList actions { NotifyRejectAction, tr("Reject"),
                          NotifyAcceptAction, tr("Accept"),
                          NotifyCloseAction, tr("Close") };

    d->notifyMessage(msg.arg(CommonUitls::elidedText(nick, Qt::ElideMiddle, 25)), actions, 10 * 1000);
#else
    d->transDialog()->showConfirmDialog(nick);
#endif
}

void TransferHelper::notifyTransferResult(bool result, const QString &msg)
{
    QStringList actions;
    if (result)
        actions << NotifyViewAction << tr("View");

    d->notifyMessage(msg, actions, 3 * 1000);
}

void TransferHelper::handleCancelTransferApply()
{
    static QString body(tr("The other party has cancelled the transfer request !"));
#ifdef __linux__
    d->notifyMessage(body, {}, 3 * 1000);
#else
    d->transDialog()->showResultDialog(false, body);
#endif
}

void TransferHelper::onConnectStatusChanged(int result, const QString &msg, const bool isself)
{
    LOG << "connect status: " << result << " msg:" << msg.toStdString();
    if (result > 0) {
        if (!isself)
            return;

        d->status.storeRelease(Confirming);
    } else {
        if (Idle == d->status.loadAcquire())
            return;

        d->status.storeRelease(Idle);
        transferResult(false, tr("Connect to \"%1\" failed").arg(msg));
    }
}

void TransferHelper::onTransChanged(int status, const QString &path, quint64 size)
{
#ifdef QT_DEBUG
    DLOG << "status: " << status << " path=" << path.toStdString();
#endif
    switch (status) {
    case TRANS_CANCELED:
        cancelTransfer(false);
        break;
    case TRANS_EXCEPTION:
        // exception reason: "io_error" "net_error" "not_found" "fs_exception"
        d->status.storeRelease(Idle);
        if (path == "io_error") {
            transferResult(false, tr("Insufficient storage space, file delivery failed this time. Please clean up disk space and try again!"));
        } else if (path == "net_error") {
            transferResult(false, tr("Network not connected, file delivery failed this time. Please connect to the network and try again!"));
        } else {
            transferResult(false, tr("File read/write exception"));
        }
        break;
    case TRANS_COUNT_SIZE:
        // only update the total size while rpc notice
        d->transferInfo.totalSize = size;
        break;
    case TRANS_WHOLE_START:
        d->status.storeRelease(Transfering);
        updateTransProgress(0);
        break;
    case TRANS_WHOLE_FINISH:
        d->status.storeRelease(Idle);
        if (d->role == Client) {
            d->recvFilesSavePath = NetworkUtil::instance()->getStorageFolder();
            HistoryManager::instance()->writeIntoTransHistory(NetworkUtil::instance()->getConfirmTargetAddress(), d->recvFilesSavePath);
        }
        transferResult(true, tr("File sent successfully"));
        break;
    case TRANS_INDEX_CHANGE:
        break;
    case TRANS_FILE_CHANGE:
        break;
    case TRANS_FILE_SPEED: {
        d->transferInfo.transferSize += size;
        d->transferInfo.maxTimeS += 1;   // 每1秒收到此信息
        updateTransProgress(d->transferInfo.transferSize);

        // double speed = (static_cast<double>(size)) / (1024 * 1024); // 计算下载速度，单位为兆字节/秒
        // QString formattedSpeed = QString::number(speed, 'f', 2); // 格式化速度为保留两位小数的字符串
        // DLOG << "Transfer speed: " << formattedSpeed.toStdString() << " M/s";

    } break;
    case TRANS_FILE_DONE:
        break;
    }
}

void TransferHelper::updateTransProgress(uint64_t current)
{
    QTime time(0, 0, 0);
    if (d->transferInfo.totalSize < 1) {
        // the total has not been set.
        updateProgress(0, time.toString("hh:mm:ss"));
        return;
    }

    // 计算整体进度和预估剩余时间
    double value = static_cast<double>(current) / d->transferInfo.totalSize;
    int progressValue = static_cast<int>(value * 100);

    int remain_time;
    if (progressValue <= 0) {
        return;
    } else if (progressValue >= 100) {
        progressValue = 100;
        remain_time = 0;
    } else {
        remain_time = (d->transferInfo.maxTimeS * 100 / progressValue - d->transferInfo.maxTimeS);
    }
    time = time.addSecs(remain_time);

#ifdef QT_DEBUG
    // DLOG << "progressbar: " << progressValue << " remain_time=" << remain_time;
    // DLOG << "totalSize: " << d->transferInfo.totalSize << " transferSize=" << current;
#endif

    updateProgress(progressValue, time.toString("hh:mm:ss"));
}

void TransferHelper::waitForConfirm()
{
    d->isTransTimeout = false;
    d->transferInfo.clear();
    d->recvFilesSavePath.clear();

    // 超时处理
    d->confirmTimer.start();
    d->transDialog()->showWaitConfirmDialog();
}

void TransferHelper::accepted()
{
    if (!d->status.testAndSetRelease(Confirming, Transfering)) {
        d->status.storeRelease(Idle);
        return;
    }
    d->role = Server;
    d->status.storeRelease(Transfering);
    updateProgress(1, tr("calculating"));
    NetworkUtil::instance()->doSendFiles(d->readyToSendFiles);
}

void TransferHelper::rejected()
{
    DLOG << "file transfer rejected >>> ";
    d->status.storeRelease(Idle);
    if (!d->isTransTimeout)
        transferResult(false, tr("The other party rejects your request"));
    d->transDialog()->hide();
}

void TransferHelper::cancelTransfer(bool click)
{
    if (Idle == d->status.loadAcquire()) {
        WLOG << "Transfer Idle, ignore cancel again!";
        return;
    }

    d->status.storeRelease(Idle);
    if (click) {
        // just hide dislog if user click cancel button
        d->transDialog()->hide();
    } else {
        transferResult(false, tr("The other party has canceled the file transfer"));
    }
}

void TransferHelper::cancelTransferApply()
{
    d->status.storeRelease(Idle);
    d->confirmTimer.stop();
    d->transDialog()->hide();
    NetworkUtil::instance()->cancelApply("transfer");
}

// ----------compat the old protocol-----------

void TransferHelper::compatTransJobStatusChanged(int id, int result, const QString &msg)
{
    LOG << "id: " << id << " result: " << result << " msg: " << msg.toStdString();
    switch (result) {
    case JOB_TRANS_FAILED:
        if (Server == d->role) {
            // Cancel by self clicked
            d->status.storeRelease(Idle);
            d->transDialog()->hide();
            return;
        }
        if (msg.contains("::not enough")) {
            transferResult(false, tr("Insufficient storage space, file delivery failed this time. Please clean up disk space and try again!"));
        } else if (msg.contains("::off line")) {
            transferResult(false, tr("Network not connected, file delivery failed this time. Please connect to the network and try again!"));
        } else {
            transferResult(false, tr("File read/write exception"));
        }
        break;
    case JOB_TRANS_DOING:
        d->status.storeRelease(Transfering);
        break;
    case JOB_TRANS_FINISHED: {
        d->status.storeRelease(Idle);
        d->recvFilesSavePath = msg;
        transferResult(true, tr("File sent successfully"));
    } break;
    case JOB_TRANS_CANCELED:
        d->status.storeRelease(Idle);
        transferResult(false, tr("The other party has canceled the file transfer"));
        break;
    default:
        break;
    }
}

void TransferHelper::compatFileTransStatusChanged(quint64 total, quint64 current, quint64 millisec)
{
    d->transferInfo.totalSize = total;
    d->transferInfo.transferSize = current;
    d->transferInfo.maxTimeS = millisec / 1000;

    // 计算整体进度和预估剩余时间
    double value = static_cast<double>(d->transferInfo.transferSize) / d->transferInfo.totalSize;
    int progressValue = static_cast<int>(value * 100);
    QTime time(0, 0, 0);
    int remain_time;
    if (progressValue <= 0) {
        return;
    } else if (progressValue >= 100) {
        progressValue = 100;
        remain_time = 0;
    } else {
        remain_time = (d->transferInfo.maxTimeS * 100 / progressValue - d->transferInfo.maxTimeS);
    }
    time = time.addSecs(remain_time);

    // LOG << "progressbar: " << progressValue << " remain_time=" << remain_time;
    // LOG << "totalSize: " << d->transferInfo.totalSize << " transferSize=" << d->transferInfo.transferSize;

    updateProgress(progressValue, time.toString("hh:mm:ss"));
}

void TransferHelper::onTransferExcepted(int type, const QString &remote)
{
    if (Idle == d->status.loadAcquire()) {
        WLOG << "Transfer Idle, ignore exception:" << type << " " << remote.toStdString();
        return;
    }

    cancelTransfer(true); // hide dialog first and show exception
    // cancel transfer and hide progress
    NetworkUtil::instance()->cancelTrans();

    switch (type) {
    case EX_NETWORK_PINGOUT:
        transferResult(false, tr("Network not connected, file delivery failed this time. Please connect to the network and try again!"));
        break;
    case EX_SPACE_NOTENOUGH:
        transferResult(false, tr("Insufficient storage space, file delivery failed this time. Please clean up disk space and try again!"));
        break;
    case EX_FS_RWERROR: {
        transferResult(false, tr("File read/write exception"));
    } break;
    case EX_OTHER:
    default:
        break;
    }
}
