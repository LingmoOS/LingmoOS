// Copyright © 2018 Red Hat, Inc
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include "utils.h"
#include <QDBusArgument>
#include <QDBusMetaType>

#include <QEventLoop>
#include <QLoggingCategory>
#include <QThread>
#include <QTimer>
#include <QFile>
#include <QPainter>
#include <QImage>
#include <QtConcurrent>
// KWayland
#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/event_queue.h>
#include <DWayland/Client/registry.h>
#include <DWayland/Client/output.h>
#include <DWayland/Client/remote_access.h>
#include <DWayland/Client/fakeinput.h>
#include <DWayland/Client/outputdevice_v2.h>

// system
#include <fcntl.h>
#include <unistd.h>

#include <KLocalizedString>


// system
#include <sys/mman.h>
#include <qdir.h>
#include "recordadmin.h"

#include <string.h>
#include <QMutexLocker>


Q_LOGGING_CATEGORY(XdgDesktopPortalKdeWaylandIntegration, "xdp-kde-wayland-integration")

//取消自动析构机制，此处后续还需优化
//Q_GLOBAL_STATIC(WaylandIntegration::WaylandIntegrationPrivate, globalWaylandIntegration)
static WaylandIntegration::WaylandIntegrationPrivate *globalWaylandIntegration = new  WaylandIntegration::WaylandIntegrationPrivate();

void WaylandIntegration::init(QStringList list)
{
    globalWaylandIntegration->initWayland(list);
}

void WaylandIntegration::init(QStringList list, GstRecordX *gstRecord)
{
    globalWaylandIntegration->initWayland(list, gstRecord);
}

bool WaylandIntegration::isEGLInitialized()
{
    return globalWaylandIntegration->isEGLInitialized();
}

void WaylandIntegration::stopStreaming()
{
    qDebug() << "stop recording!";
    globalWaylandIntegration->stopVideoRecord();
    globalWaylandIntegration->stopStreaming();
}

bool WaylandIntegration::WaylandIntegrationPrivate::stopVideoRecord()
{
    if (Utils::isFFmpegEnv) {
        if (m_recordAdmin) {
            return  m_recordAdmin->stopStream();
        } else {
            qWarning() << "m_recordAdmin is not init!";
            return false;
        }
    } else {
        setBGetFrame(false);
        isGstWriteVideoFrame = false;
        return true;
    }
}

QMap<quint32, WaylandIntegration::WaylandOutput> WaylandIntegration::screens()
{
    return globalWaylandIntegration->screens();
}

WaylandIntegration::WaylandIntegration *WaylandIntegration::waylandIntegration()
{
    return globalWaylandIntegration;
}

// Thank you kscreen
void WaylandIntegration::WaylandOutput::setOutputType(const QString &type)
{
    const auto embedded = { QLatin1String("LVDS"),
                            QLatin1String("IDP"),
                            QLatin1String("EDP"),
                            QLatin1String("LCD")
                          };

    for (const QLatin1String &pre : embedded) {
        if (type.toUpper().startsWith(pre)) {
            m_outputType = OutputType::Laptop;
            return;
        }
    }

    if (type.contains(QLatin1String("VGA")) || type.contains(QLatin1String("DVI")) || type.contains(QLatin1String("HDMI")) || type.contains(QLatin1String("Panel")) ||
            type.contains(QLatin1String("DisplayPort")) || type.startsWith(QLatin1String("DP")) || type.contains(QLatin1String("unknown"))) {
        m_outputType = OutputType::Monitor;
    } else if (type.contains(QLatin1String("TV"))) {
        m_outputType = OutputType::Television;
    } else {
        m_outputType = OutputType::Monitor;
    }
}

const QDBusArgument &operator >> (const QDBusArgument &arg, WaylandIntegration::WaylandIntegrationPrivate::Stream &stream)
{
    arg.beginStructure();
    arg >> stream.nodeId;

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant map;
        arg.beginMapEntry();
        arg >> key >> map;
        arg.endMapEntry();
        stream.map.insert(key, map);
    }
    arg.endMap();
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator << (QDBusArgument &arg, const WaylandIntegration::WaylandIntegrationPrivate::Stream &stream)
{
    arg.beginStructure();
    arg << stream.nodeId;
    arg << stream.map;
    arg.endStructure();

    return arg;
}

Q_DECLARE_METATYPE(WaylandIntegration::WaylandIntegrationPrivate::Stream)
Q_DECLARE_METATYPE(WaylandIntegration::WaylandIntegrationPrivate::Streams)

WaylandIntegration::WaylandIntegrationPrivate::WaylandIntegrationPrivate()
    : WaylandIntegration()
    , m_eglInitialized(false)
    , m_registryInitialized(false)
    , m_connection(nullptr)
    , m_queue(nullptr)
    , m_registry(nullptr)
    , m_remoteAccessManager(nullptr)
{
    m_bInit = true;
#if defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__) || defined (__loongarch__)
    m_bufferSize = 60;
#elif defined (__aarch64__)
    m_bufferSize = 60;
#else
    m_bufferSize = 200;
#endif
    m_ffmFrame = nullptr;
    qDBusRegisterMetaType<WaylandIntegrationPrivate::Stream>();
    qDBusRegisterMetaType<WaylandIntegrationPrivate::Streams>();
    m_recordAdmin = nullptr;
    m_gstRecordX = nullptr;
    //m_writeFrameThread = nullptr;
    m_bInitRecordAdmin = true;
    m_bGetFrame = true;
    //m_recordTIme = -1;
    m_screenCount = 1;
}

WaylandIntegration::WaylandIntegrationPrivate::~WaylandIntegrationPrivate()
{
    QMutexLocker locker(&m_mutex);
    for (int i = 0; i < m_freeList.size(); i++) {
        if (m_freeList[i]) {
            delete m_freeList[i];
        }
    }
    m_freeList.clear();
    for (int i = 0; i < m_waylandList.size(); i++) {
        delete m_waylandList[i]._frame;
    }
    m_waylandList.clear();
    if (nullptr != m_ffmFrame) {
        delete []m_ffmFrame;
        m_ffmFrame = nullptr;
    }
    if (nullptr != m_recordAdmin) {
        delete m_recordAdmin;
        m_recordAdmin = nullptr;
    }
}


bool WaylandIntegration::WaylandIntegrationPrivate::isEGLInitialized() const
{
    return m_eglInitialized;
}

void WaylandIntegration::WaylandIntegrationPrivate::bindOutput(int outputName, int outputVersion)
{
    KWayland::Client::Output *output = new KWayland::Client::Output(this);
    output->setup(m_registry->bindOutput(static_cast<uint32_t>(outputName), static_cast<uint32_t>(outputVersion)));
    m_bindOutputs << output;
}

void WaylandIntegration::WaylandIntegrationPrivate::stopStreaming()
{
    if (m_streamingEnabled) {
        m_appendFrameToListFlag = false;
        m_streamingEnabled = false;

        // First unbound outputs and destroy remote access manager so we no longer receive buffers
        if (m_remoteAccessManager) {
            m_remoteAccessManager->release();
            m_remoteAccessManager->destroy();
        }
        qDeleteAll(m_bindOutputs);
        m_bindOutputs.clear();
        //        if (m_stream) {
        //            delete m_stream;
        //            m_stream = nullptr;
        //        }
    }
}

QMap<quint32, WaylandIntegration::WaylandOutput> WaylandIntegration::WaylandIntegrationPrivate::screens()
{
    return m_outputMap;
}

void WaylandIntegration::WaylandIntegrationPrivate::initWayland(QStringList list)
{
    //通过wayland底层接口获取图片的方式不相同，需要获取电脑的厂商，hw的需要特殊处理
    //m_boardVendorType = getBoardVendorType();
    m_boardVendorType = getProductType();
    qDebug() << "m_boardVendorType: " << m_boardVendorType;
    //由于性能问题部分非hw的arm机器编码效率低，适当调大视频帧的缓存空间（防止调整的过大导致保存时间延长），且在下面添加视频到缓冲区时进行了降低帧率的处理。
    if (QSysInfo::currentCpuArchitecture().startsWith("ARM", Qt::CaseInsensitive) && !m_boardVendorType) {
        m_bufferSize = 200;
    }
    m_fps = list[5].toInt();
    m_recordAdmin = new RecordAdmin(list, this);
    m_recordAdmin->setBoardVendor(m_boardVendorType);
    //初始化wayland服务链接
    initConnectWayland();
}

void WaylandIntegration::WaylandIntegrationPrivate::initWayland(QStringList list, GstRecordX *gstRecord)
{
    //通过wayland底层接口获取图片的方式不相同，需要获取电脑的厂商，hw的需要特殊处理
    //m_boardVendorType = getBoardVendorType();
    m_boardVendorType = getProductType();
    qDebug() << "m_boardVendorType: " << m_boardVendorType;
    //由于性能问题部分非hw的arm机器编码效率低，适当调大视频帧的缓存空间（防止调整的过大导致保存时间延长），且在下面添加视频到缓冲区时进行了降低帧率的处理。
    if (QSysInfo::currentCpuArchitecture().startsWith("ARM", Qt::CaseInsensitive) && !m_boardVendorType) {
        m_bufferSize = 200;
    }
    m_fps = list[5].toInt();
    m_gstRecordX = gstRecord;
    m_gstRecordX->setBoardVendorType(m_boardVendorType);
    //初始化wayland服务链接
    initConnectWayland();
}

void WaylandIntegration::WaylandIntegrationPrivate::initConnectWayland()
{
    //设置获取视频帧
    setBGetFrame(true);

    m_thread = new QThread(this);
    m_connection = new KWayland::Client::ConnectionThread;
    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandIntegrationPrivate::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied, this, [this] {
        if (m_queue)
        {
            delete m_queue;
            m_queue = nullptr;
        }
        m_connection->deleteLater();
        m_connection = nullptr;
        if (m_thread)
        {
            m_thread->quit();
            if (!m_thread->wait(3000)) {
                m_thread->terminate();
                m_thread->wait();
            }
            delete m_thread;
            m_thread = nullptr;
        }
    });
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, this, [this] {
        m_thread->quit();
        m_thread->wait();
    });
    m_thread->start();
    m_connection->moveToThread(m_thread);
    m_connection->initConnection();
}

int WaylandIntegration::WaylandIntegrationPrivate::getBoardVendorType()
{
    QFile file("/sys/class/dmi/id/board_vendor");
    bool flag = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!flag) {
        return 0;
    }
    QByteArray t = file.readAll();
    QString result = QString(t);
    if (result.isEmpty()) {
        return 0;
    }
    file.close();
    qDebug() << "/sys/class/dmi/id/board_vendor: " << result;
    qDebug() << "cpu: " << Utils::getCpuModelName();
    if (result.contains("HUAWEI")) {
        if (!Utils::getCpuModelName().contains("Kunpeng", Qt::CaseSensitivity::CaseInsensitive)) {
            return 1;
        }
    }
    return 0;
}
//根据命令行 dmidecode -s system-product-name|awk '{print SNF}' 返回的结果判断是否是华为电脑
int WaylandIntegration::WaylandIntegrationPrivate::getProductType()
{
    QStringList options;
    options << QString(QStringLiteral("-c"));
    options << QString(QStringLiteral("dmidecode -s system-product-name|awk '{print $NF}'"));
    QProcess process;
    process.start(QString(QStringLiteral("bash")), options);
    process.waitForFinished();
    process.waitForReadyRead();
    QByteArray tempArray =  process.readAllStandardOutput();
    char *charTemp = tempArray.data();
    QString str_output = QString(QLatin1String(charTemp));
    process.close();
    qInfo() << "system-product-name: " << str_output;
    if (str_output.contains("KLVV", Qt::CaseInsensitive) ||
            str_output.contains("KLVU", Qt::CaseInsensitive) ||
            str_output.contains("PGUV", Qt::CaseInsensitive) ||
            str_output.contains("PGUW", Qt::CaseInsensitive))
        return 1;
    return 0;
}
void WaylandIntegration::WaylandIntegrationPrivate::addOutput(quint32 name, quint32 version)
{
    KWayland::Client::Output *output = new KWayland::Client::Output(this);
    output->setup(m_registry->bindOutput(name, version));

    connect(output, &KWayland::Client::Output::changed, this, [this, name, version, output]() {
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Adding output:";
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    manufacturer: " << output->manufacturer();
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    model: " << output->model();
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    resolution: " << output->pixelSize();

        WaylandOutput portalOutput;
        portalOutput.setManufacturer(output->manufacturer());
        portalOutput.setModel(output->model());
        portalOutput.setOutputType(output->model());
        portalOutput.setResolution(output->pixelSize());
        portalOutput.setWaylandOutputName(static_cast<int>(name));
        portalOutput.setWaylandOutputVersion(static_cast<int>(version));

        if (m_registry->hasInterface(KWayland::Client::Registry::Interface::RemoteAccessManager)) {
            KWayland::Client::Registry::AnnouncedInterface interface = m_registry->interface(KWayland::Client::Registry::Interface::RemoteAccessManager);
            if (!interface.name && !interface.version) {
                qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to start streaming: remote access manager interface is not initialized yet";
                return ;
            }
            return ;
        }
        m_outputMap.insert(name, portalOutput);

        //        delete output;
    });
}

void WaylandIntegration::WaylandIntegrationPrivate::removeOutput(quint32 name)
{
    WaylandOutput output = m_outputMap.take(name);
    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Removing output:";
    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    manufacturer: " << output.manufacturer();
    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    model: " << output.model();
}

void WaylandIntegration::WaylandIntegrationPrivate::onDeviceChanged(quint32 name, quint32 version)
{
    KWayland::Client::OutputDeviceV2 *devT = m_registry->createOutputDeviceV2(name, version);
    if (devT && devT->isValid()) {
        connect(devT, &KWayland::Client::OutputDeviceV2::changed, this, [ = ]() {
            qDebug() << devT->uuid() << devT->geometry();
            // 保存屏幕id和对应位置大小
            m_screenId2Point.insert(devT->uuid(), devT->geometry());
            m_screenCount = m_screenId2Point.size();

            // 更新屏幕大小
            m_screenSize.setWidth(0);
            m_screenSize.setHeight(0);
            for (auto p = m_screenId2Point.begin(); p != m_screenId2Point.end(); ++p) {
                if (p.value().x() + p.value().width() > m_screenSize.width()) {
                    m_screenSize.setWidth(p.value().x() + p.value().width());
                }
                if (p.value().y() + p.value().height() > m_screenSize.height()) {
                    m_screenSize.setHeight(p.value().y() + p.value().height());
                }
            }
            qDebug() << "屏幕大小:" << m_screenSize;
        });
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::processBuffer(const KWayland::Client::RemoteBuffer *rbuf, const QRect rect)
{
    //qDebug() << Q_FUNC_INFO;
    QScopedPointer<const KWayland::Client::RemoteBuffer> guard(rbuf);
    auto dma_fd = rbuf->fd();
    quint32 width = rbuf->width();
    quint32 height = rbuf->height();
    quint32 stride = rbuf->stride();
    //    if(!bGetFrame())
    //        return;
    if (m_bInitRecordAdmin) {
        m_bInitRecordAdmin = false;
        if (Utils::isFFmpegEnv) {
            m_recordAdmin->init(static_cast<int>(m_screenSize.width()), static_cast<int>(m_screenSize.height()));
            frameStartTime = avlibInterface::m_av_gettime();
        } else {
            frameStartTime = QDateTime::currentMSecsSinceEpoch();
            isGstWriteVideoFrame = true;
            if (m_gstRecordX) {
                m_gstRecordX->waylandGstStartRecord();
            } else {
                qWarning() << "m_gstRecordX is nullptr!";
            }
            QtConcurrent::run(this, &WaylandIntegrationPrivate::gstWriteVideoFrame);
        }
        m_appendFrameToListFlag = true;
        QtConcurrent::run(this, &WaylandIntegrationPrivate::appendFrameToList);
    }
    unsigned char *mapData = static_cast<unsigned char *>(mmap(nullptr, stride * height, PROT_READ, MAP_SHARED, dma_fd, 0));
    if (MAP_FAILED == mapData) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "dma fd " << dma_fd << " mmap failed - ";
    }
//appendBuffer(mapData, static_cast<int>(width), static_cast<int>(height), static_cast<int>(stride), avlibInterface::m_av_gettime() - frameStartTime);
    if (m_screenCount == 1) {
        m_curNewImage.first = 0;//avlibInterface::m_av_gettime() - frameStartTime;
        {
            QMutexLocker locker(&m_bGetScreenImageMutex);
            m_curNewImage.second = QImage(mapData, width, height, QImage::Format_RGBA8888).copy();
        }
    } else {
        //QString pngName = QDateTime::currentDateTime().toString(QLatin1String("hh:mm:ss.zzz ") + QString("%1_").arg(rect.x()));
        //QImage(mapData, width, height, QImage::Format_RGBA8888).copy().save(pngName + ".png");
        m_ScreenDateBuf.append(QPair<QRect, QImage>(rect, QImage(mapData, width, height, QImage::Format_RGBA8888).copy()));
        if (m_ScreenDateBuf.size() ==  m_screenCount) {
            QMutexLocker locker(&m_bGetScreenImageMutex);
            m_curNewImageScreen.clear();
            for (auto itr = m_ScreenDateBuf.begin(); itr != m_ScreenDateBuf.end(); ++itr) {
                m_curNewImageScreen.append(*itr);
            }
            m_ScreenDateBuf.clear();
        }
    }
    munmap(mapData, stride * height);
    close(dma_fd);
}

void WaylandIntegration::WaylandIntegrationPrivate::processBufferX86(const KWayland::Client::RemoteBuffer *rbuf, const QRect rect)
{
    QScopedPointer<const KWayland::Client::RemoteBuffer> guard(rbuf);
    auto dma_fd = rbuf->fd();
    quint32 width = rbuf->width();
    quint32 height = rbuf->height();
    quint32 stride = rbuf->stride();
    if (m_bInitRecordAdmin) {
        m_bInitRecordAdmin = false;
        if (Utils::isFFmpegEnv) {
            m_recordAdmin->init(static_cast<int>(m_screenSize.width()), static_cast<int>(m_screenSize.height()));
            frameStartTime = avlibInterface::m_av_gettime();
        } else {
            frameStartTime = QDateTime::currentMSecsSinceEpoch();
            isGstWriteVideoFrame = true;
            if (m_gstRecordX) {
                m_gstRecordX->waylandGstStartRecord();
            } else {
                qWarning() << "m_gstRecordX is nullptr!";
            }
            QtConcurrent::run(this, &WaylandIntegrationPrivate::gstWriteVideoFrame);
        }
        m_appendFrameToListFlag = true;
        QtConcurrent::run(this, &WaylandIntegrationPrivate::appendFrameToList);
    }
    QImage img(m_screenSize, QImage::Format_RGBA8888);
    if (m_screenCount == 1) {
        m_curNewImage.first = 0;//avlibInterface::m_av_gettime() - frameStartTime;
        {
            QMutexLocker locker(&m_bGetScreenImageMutex);
            m_curNewImage.second = getImage(dma_fd, width, height, stride, rbuf->format());
        }
    } else {
        m_ScreenDateBuf.append(QPair<QRect, QImage>(rect, getImage(dma_fd, width, height, stride, rbuf->format())));
        if (m_ScreenDateBuf.size() ==  m_screenCount) {
            QMutexLocker locker(&m_bGetScreenImageMutex);
            m_curNewImageScreen.clear();
            for (auto itr = m_ScreenDateBuf.begin(); itr != m_ScreenDateBuf.end(); ++itr) {
                m_curNewImageScreen.append(*itr);
            }
            m_ScreenDateBuf.clear();
        }
    }
    close(dma_fd);
}
//通过线程每30ms钟向数据池中取出一张图片添加到环形缓冲区，以便后续视频编码
void WaylandIntegration::WaylandIntegrationPrivate::appendFrameToList()
{
    int64_t delayTime = 1000 / m_fps + 1;
#ifdef __mips__
    //delayTime = 150;
#endif

    while (m_appendFrameToListFlag) {
        if (m_screenCount == 1) {
            QImage tempImage;
            {
                QMutexLocker locker(&m_bGetScreenImageMutex);
                tempImage = m_curNewImage.second.copy();
            }
            if (!tempImage.isNull()) {
                int64_t temptime = 0;
                if (Utils::isFFmpegEnv) {
                    temptime = avlibInterface::m_av_gettime();

                } else {
                    temptime = QDateTime::currentMSecsSinceEpoch();

                }
                appendBuffer(tempImage.bits(), static_cast<int>(tempImage.width()), static_cast<int>(tempImage.height()),
                             static_cast<int>(tempImage.width() * 4), temptime);

            }
            QThread::msleep(static_cast<unsigned long>(delayTime));
        } else {
            //多屏录制
            int64_t curFramTime = 0;
            if (Utils::isFFmpegEnv) {
                curFramTime = avlibInterface::m_av_gettime();

            } else {
                curFramTime = QDateTime::currentMSecsSinceEpoch();

            }
#if 0
            cv::Mat res;
            res.create(cv::Size(m_screenSize.width(), m_screenSize.height()), CV_8UC4);
            res = cv::Scalar::all(0);
            for (auto itr = m_curNewImageScreen.begin(); itr != m_curNewImageScreen.end(); ++itr) {
                cv::Mat temp = cv::Mat(itr->second.height(), itr->second.width(), CV_8UC4,
                                       const_cast<uchar *>(itr->second.bits()), static_cast<size_t>(itr->second.bytesPerLine()));
                temp.copyTo(res(cv::Rect(itr->first.x(), itr->first.y(), itr->first.width(), itr->first.height())));
            }
            m_curNewImageScreen.clear();
            QImage img = QImage(res.data, res.cols, res.rows, static_cast<int>(res.step), QImage::Format_RGBA8888);
#else

            QVector<QPair<QRect, QImage>> tempImageVec;
            {
                QMutexLocker locker(&m_bGetScreenImageMutex);
                if (m_curNewImageScreen.size() != m_screenCount) continue;
                for (auto itr = m_curNewImageScreen.begin(); itr != m_curNewImageScreen.end(); ++itr) {
                    tempImageVec.append(*itr);
                }
            }
            QImage img(m_screenSize, QImage::Format_RGBA8888);
            img.fill(Qt::GlobalColor::black);
            QPainter painter(&img);
            for (auto itr = tempImageVec.begin(); itr != tempImageVec.end(); ++itr) {
                painter.drawImage(itr->first.topLeft(), itr->second);
            }
            tempImageVec.clear();
#endif
            appendBuffer(img.bits(), img.width(), img.height(), img.width() * 4, curFramTime - frameStartTime);
            // 计算拼接的时的耗时
            int64_t t = 0;
            if (Utils::isFFmpegEnv) {
                t = (avlibInterface::m_av_gettime() - curFramTime) / 1000;

            } else {
                t = (QDateTime::currentMSecsSinceEpoch() - curFramTime) / 1000;

            }
            qDebug() << delayTime - t;
            if (t < delayTime) {
                QThread::msleep(static_cast<unsigned long>(delayTime - t));
            }
        }
    }
}

//通过线程循环向gstreamer管道写入视频帧数据
void WaylandIntegration::WaylandIntegrationPrivate::gstWriteVideoFrame()
{
    waylandFrame frame;
    while (isWriteVideo()) {
        if (getFrame(frame)) {
            if (m_gstRecordX) {
                m_gstRecordX->waylandWriteVideoFrame(frame._frame, frame._width, frame._height);
            } else {
                qWarning() << "m_gstRecordX is nullptr!";
            }
        } else {
            //qDebug() << "视频缓冲区无数据！";
        }
    }
    //等待wayland视频环形队列中的视频帧，全部写入管道
    if (m_gstRecordX) {
        m_gstRecordX->waylandGstStopRecord();
    } else {
        qWarning() << "m_gstRecordX is nullptr!";
    }
}

QImage WaylandIntegration::WaylandIntegrationPrivate::getImage(int32_t fd, uint32_t width, uint32_t height, uint32_t stride, uint32_t format)
{
    QImage tempImage;
    tempImage = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGBA8888);
    eglMakeCurrent(m_eglstruct.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, m_eglstruct.ctx);
    EGLint importAttributes[] = {EGL_WIDTH,
                                 static_cast<int>(width),
                                 EGL_HEIGHT,
                                 static_cast<int>(height),
                                 EGL_LINUX_DRM_FOURCC_EXT,
                                 static_cast<int>(format),
                                 EGL_DMA_BUF_PLANE0_FD_EXT,
                                 fd,
                                 EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                                 0,
                                 EGL_DMA_BUF_PLANE0_PITCH_EXT,
                                 static_cast<int>(stride),
                                 EGL_NONE
                                };
    EGLImageKHR image = eglCreateImageKHR(m_eglstruct.dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, importAttributes);
    if (image == EGL_NO_IMAGE_KHR) {
        qDebug() << "未获取到图片！！！";
        return tempImage;
    }

    // create GL 2D texture for framebuffer
    GLuint texture;
    glGenTextures(1, &texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tempImage.bits());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    eglDestroyImageKHR(m_eglstruct.dpy, image);

    return tempImage;
}
void WaylandIntegration::WaylandIntegrationPrivate::setupRegistry()
{
    initEgl();
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);

    connect(m_registry, &KWayland::Client::Registry::outputAnnounced, this, &WaylandIntegrationPrivate::addOutput);
    connect(m_registry, &KWayland::Client::Registry::outputRemoved, this, &WaylandIntegrationPrivate::removeOutput);
    connect(m_registry, &KWayland::Client::Registry::outputDeviceV2Announced, this, &WaylandIntegrationPrivate::onDeviceChanged);
    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this] {
        m_registryInitialized = true;
        //qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Registry initialized";
    });


    connect(m_registry, &KWayland::Client::Registry::remoteAccessManagerAnnounced, this,
    [this](quint32 name, quint32 version) {
        Q_UNUSED(name);
        Q_UNUSED(version);
        m_remoteAccessManager = m_registry->createRemoteAccessManager(m_registry->interface(KWayland::Client::Registry::Interface::RemoteAccessManager).name, m_registry->interface(KWayland::Client::Registry::Interface::RemoteAccessManager).version);
        connect(m_remoteAccessManager, &KWayland::Client::RemoteAccessManager::bufferReady, this, [this](const void *output, const KWayland::Client::RemoteBuffer * rbuf) {
            QRect screenGeometry = (KWayland::Client::Output::get(reinterpret_cast<wl_output *>(const_cast<void *>(output))))->geometry();
            connect(rbuf, &KWayland::Client::RemoteBuffer::parametersObtained, this, [this, rbuf, screenGeometry] {
                if (m_boardVendorType)
                {
                    processBuffer(rbuf, screenGeometry);
                } else
                {
                    processBufferX86(rbuf, screenGeometry);
                }
            });
        });
    }
           );

    m_registry->create(m_connection);
    m_registry->setEventQueue(m_queue);
    m_registry->setup();
}
void WaylandIntegration::WaylandIntegrationPrivate::initEgl()
{
    static const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                             EGL_NONE
                                            };

    EGLint config_attribs[] = {EGL_SURFACE_TYPE,
                               EGL_WINDOW_BIT,
                               EGL_RED_SIZE,
                               1,
                               EGL_GREEN_SIZE,
                               1,
                               EGL_BLUE_SIZE,
                               1,
                               EGL_ALPHA_SIZE,
                               1,
                               EGL_RENDERABLE_TYPE,
                               EGL_OPENGL_ES2_BIT,
                               EGL_NONE
                              };

    EGLint major, minor, n;
    EGLBoolean ret;
    m_eglstruct.dpy = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(m_connection->display()));
    assert(m_eglstruct.dpy);

    ret = eglInitialize(m_eglstruct.dpy, &major, &minor);
    assert(ret == EGL_TRUE);
    ret = eglBindAPI(EGL_OPENGL_ES_API);
    assert(ret == EGL_TRUE);

    ret =
        eglChooseConfig(m_eglstruct.dpy, config_attribs, &m_eglstruct.conf, 1, &n);
    assert(ret && n == 1);

    m_eglstruct.ctx = eglCreateContext(m_eglstruct.dpy, m_eglstruct.conf,
                                       EGL_NO_CONTEXT, context_attribs);
    assert(m_eglstruct.ctx);

    printf("egl init success!\n");
}
void WaylandIntegration::WaylandIntegrationPrivate::appendBuffer(unsigned char *frame, int width, int height, int stride, int64_t time)
{
    if (!bGetFrame() || nullptr == frame || width <= 0 || height <= 0) {
        return;
    }
    int size = height * stride;
    unsigned char *ch = nullptr;
    if (m_bInit) {
        m_bInit = false;
        m_width = width;
        m_height = height;
        m_stride = stride;
        m_ffmFrame = new unsigned char[static_cast<unsigned long>(size)];
        for (int i = 0; i < m_bufferSize; i++) {
            ch = new unsigned char[static_cast<unsigned long>(size)];
            m_freeList.append(ch);
            //qDebug() << "创建内存空间";
        }
    }
    QMutexLocker locker(&m_mutex);
    if (m_waylandList.size() >= m_bufferSize) {
        //先进先出
        //取队首
        waylandFrame wFrame = m_waylandList.first();
        memset(wFrame._frame, 0, static_cast<size_t>(size));
        //拷贝当前帧
        memcpy(wFrame._frame, frame, static_cast<size_t>(size));
        wFrame._time = time;
        wFrame._width = width;
        wFrame._height = height;
        wFrame._stride = stride;
        wFrame._index = 0;
        //删队首
        m_waylandList.removeFirst();
        //存队尾
        m_waylandList.append(wFrame);
        //qDebug() << "环形缓冲区已满，删队首，存队尾";
    } else if (0 <= m_waylandList.size() &&  m_waylandList.size() < m_bufferSize) {
        if (m_freeList.size() > 0) {
            waylandFrame wFrame;
            wFrame._time = time;
            wFrame._width = width;
            wFrame._height = height;
            wFrame._stride = stride;
            wFrame._index = 0;
            //分配空闲内存
            wFrame._frame = m_freeList.first();
            memset(wFrame._frame, 0, static_cast<size_t>(size));
            //拷贝wayland推送的视频帧
            memcpy(wFrame._frame, frame, static_cast<size_t>(size));
            m_waylandList.append(wFrame);
            //qDebug() << "环形缓冲区未满，存队尾"
            //空闲内存占用，仅删除索引，不删除空间
            m_freeList.removeFirst();
        }
    }
}

int WaylandIntegration::WaylandIntegrationPrivate::frameIndex = 0;

bool WaylandIntegration::WaylandIntegrationPrivate::getFrame(waylandFrame &frame)
{
    QMutexLocker locker(&m_mutex);
    if (m_waylandList.size() <= 0 || nullptr == m_ffmFrame) {
        frame._width = 0;
        frame._height = 0;
        frame._frame = nullptr;
        return false;
    } else {
        int size = m_height * m_stride;
        //取队首，先进先出
        waylandFrame wFrame = m_waylandList.first();
        frame._width = wFrame._width;
        frame._height = wFrame._height;
        frame._stride = wFrame._stride;
        frame._time = wFrame._time;
        //m_ffmFrame 视频帧缓存
        frame._frame = m_ffmFrame;
        frame._index = frameIndex++;
        //拷贝到 m_ffmFrame 视频帧缓存
        memcpy(frame._frame, wFrame._frame, static_cast<size_t>(size));
        //删队首视频帧 waylandFrame，未删空闲内存 waylandFrame::_frame，只删索引，不删内存空间
        m_waylandList.removeFirst();
        //回收空闲内存，重复使用
        m_freeList.append(wFrame._frame);
        //qDebug() << "获取视频帧";
        return true;
    }
}

bool WaylandIntegration::WaylandIntegrationPrivate::isWriteVideo()
{
    QMutexLocker locker(&m_mutex);
    if (Utils::isFFmpegEnv) {
        if (m_recordAdmin->m_writeFrameThread->bWriteFrame()) {
            return true;
        }
    } else {
        if (isGstWriteVideoFrame) {
            return true;
        }
    }
    return !m_waylandList.isEmpty();
}

bool WaylandIntegration::WaylandIntegrationPrivate::bGetFrame()
{
    QMutexLocker locker(&m_bGetFrameMutex);
    return m_bGetFrame;
}

void WaylandIntegration::WaylandIntegrationPrivate::setBGetFrame(bool bGetFrame)
{
    QMutexLocker locker(&m_bGetFrameMutex);
    m_bGetFrame = bGetFrame;
}
