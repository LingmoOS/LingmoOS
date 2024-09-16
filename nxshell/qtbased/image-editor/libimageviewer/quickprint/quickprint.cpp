// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickprint.h"
#include "quickprint_p.h"

#include "unionimage/baseutils.h"

#include <DPrintPreviewDialog>

#include <QtGlobal>
#include <QThread>
#include <QFileInfo>
#include <QApplication>
#include <QScreen>

DWIDGET_USE_NAMESPACE

// 加载动画控件大小
static const int s_SpinnerSize = 40;
// 数据加载时间超过 500ms 将触发加载动画(仅异步加载有效)
static const int s_LoadSpinnerTimeOut = 500;

QuickPrintPrivate::QuickPrintPrivate(QuickPrint *q)
    : q_ptr(q)
    , imageLoader(new PrintImageLoader)
{
    // 快速打印可能无需加载主窗体，单独加载动态库翻译信息
    Libutils::base::loadLibTransaltor();

    QObject::connect(imageLoader.data(), &PrintImageLoader::loadFinished, this, &QuickPrintPrivate::printLoadFinished);
    // Note: 队列发送，在调用接口结束后发出，避免部分操作流程嵌套。
    QObject::connect(
        this,
        &QuickPrintPrivate::notifyLoadFinished,
        this,
        [this](int ret, bool) { Q_EMIT q_ptr->printFinished(ret); },
        Qt::QueuedConnection);
}

/**
   @brief 启动加载动画定时器，若加载时间超过500ms，则显示加载动画
 */
void QuickPrintPrivate::startLoadSpinnerTimer()
{
    if (!procSpinnerTimer.isActive()) {
        procSpinnerTimer.start(s_LoadSpinnerTimeOut, this);
    }
}

/**
   @brief 初始化并开始加载动画
 */
void QuickPrintPrivate::startSpinner()
{
    if (!spinner) {
        spinner.reset(new DSpinner);

        spinner->setObjectName("QuickPrint_Spinner");
        spinner->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
        spinner->setAttribute(Qt::WA_TranslucentBackground, true);
        spinner->setFixedSize(s_SpinnerSize, s_SpinnerSize);

        // 居中
        QPoint pos = QCursor::pos();
        for (QScreen *screen : qApp->screens()) {
            if (screen && screen->geometry().contains(pos)) {
                QRect qr = spinner->geometry();
                qr.moveCenter(screen->geometry().center());
                spinner->move(qr.topLeft());
            }
        }

        spinner->start();
        spinner->show();
    }
}

/**
   @brief 停止加载动画
 */
void QuickPrintPrivate::stopSpinner()
{
    if (procSpinnerTimer.isActive()) {
        procSpinnerTimer.stop();
    }

    if (spinner) {
        spinner->stop();
        spinner->hide();
    }
}

/**
   @brief 显示警告信息，提示文件中包含无权限或已损坏的文件，无法打印
 */
void QuickPrintPrivate::showWarningNotify(const QString &errorString)
{
    // 错误信息预留
    Q_UNUSED(errorString)

    DDialog warnDialog(this->parentWidget);
    warnDialog.setObjectName("QuickPrint_WarnDialog");
    warnDialog.setIcon(QIcon::fromTheme("deepin-image-viewer"));
    // "选中的文件中包含无权限或已损坏的文件，无法打印。"
    warnDialog.setMessage(QObject::tr("The selected file has permission denied or is corrupted and cannot be printed."));
    warnDialog.addButton(QObject::tr("Confirm"));

    warnDialog.exec();
}

/**
   @brief 展示打印对话框，父窗口（若有）将被设置为 \a parentWidget
 */
int QuickPrintPrivate::showPrintDialog(QWidget *parentWidget)
{
    if (loadDataList.isEmpty()) {
        return QDialog::Rejected;
    }

    DPrintPreviewDialog printDialog(parentWidget);
    printDialog.setObjectName("QuickPrint_PrintDialog");
    printDialog.setAsynPreview(loadDataList.size());
    // 设置打印文件名，用于 Cups 服务记录打印任务
    printDialog.setDocName(loadDataList.first()->filePath);

    connect(&printDialog,
            QOverload<DPrinter *, const QVector<int> &>::of(&DPrintPreviewDialog::paintRequested),
            this,
            &QuickPrintPrivate::asyncPrint);

    return printDialog.exec();
}

/**
   @brief 绘制图片到打印器 \a printer ，仅处理 \a pageRange 索引包含的图片
 */
void QuickPrintPrivate::asyncPrint(DPrinter *printer, const QVector<int> &pageRange)
{
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QRect rect = printer->pageRect();
    qreal ratio = 0.0;

    for (int page : pageRange) {
        if (page > 0 && page < (loadDataList.size() + 1)) {
            QImage &img = loadDataList[page - 1]->data;
            // 按比例缩放图片到打印纸张尺寸
            ratio = rect.width() * 1.0 / img.width();
            if (qreal(rect.height() - img.height() * ratio) > 0) {
                painter.drawImage(
                    QRectF(0, abs(qreal(rect.height() - img.height() * ratio)) / 2, rect.width(), img.height() * ratio), img);
            } else {
                ratio = rect.height() * 1.0 / img.height();
                painter.drawImage(QRectF(qreal(rect.width() - img.width() * ratio) / 2, 0, img.width() * ratio, rect.height()),
                                  img);
            }

            if (page != pageRange.last()) {
                printer->newPage();
            }
        }
    }

    painter.end();
}

/**
   @brief 若加载时间超过500ms，则显示加载动画
 */
void QuickPrintPrivate::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == procSpinnerTimer.timerId()) {
        procSpinnerTimer.stop();
        if (imageLoader->isLoading()) {
            startSpinner();
        }
    }
}

/**
   @brief 加载将要打印的图片文件 \a fileList
 */
bool QuickPrintPrivate::setPrintImage(const QStringList &fileList)
{
    if (fileList.isEmpty()) {
        return false;
    }

    /// todo:判断MTP文件代理

    bool async = checkNeedAsyncLoadData(fileList);
    return imageLoader->loadImageList(fileList, async);
}

/**
   @brief 加载文件列表 \a fileList 数据，将判断是否同步加载，
        可用线程数小于3，加载数据小于11且文件总大小于15MB的数据不会调用多线程处理
 */
bool QuickPrintPrivate::checkNeedAsyncLoadData(const QStringList &fileList)
{
    static const int s_ThreadLimit = 2;
    static const int s_FileCountLimit = 10;
    static constexpr qint64 s_FileSizeLimitMB = 15 * 1024 * 1024;

    bool async = true;
    if (QThread::idealThreadCount() <= s_ThreadLimit) {
        // Note: 部分环境(服务器/云桌面)可用线程较少，使用多线程处理会出现异常
        async = false;
    } else if (fileList.size() <= s_FileCountLimit) {
        qint64 fileSizeCount = 0;
        for (const QString &file : fileList) {
            fileSizeCount += QFileInfo(file).size();
        }

        if (fileSizeCount < s_FileSizeLimitMB) {
            async = false;
        }
    }

    return async;
}

/**
   @brief 打印加载完成，若有错误，\a error 置为 true ，errorString 包含错误信息
 */
void QuickPrintPrivate::printLoadFinished(bool error, const QString &errorString)
{
    this->stopSpinner();

    int ret = QDialog::Rejected;
    if (error) {
        this->showWarningNotify(errorString);
    } else {
        loadDataList = imageLoader->takeLoadData();

        // 在此处弹出打印窗口
        ret = this->showPrintDialog(this->parentWidget);
    }

    // 在处理完成后，触发提示信号，唤醒等待 EventLoop，并队列发送 printFinsihed() 信号
    Q_EMIT this->notifyLoadFinished(ret, error);
}

/**
   @brief 等待打印图片数据加载完成
 */
bool QuickPrintPrivate::waitLoadFinished()
{
    bool ret = true;
    if (imageLoader->isLoading()) {
        QEventLoop loop;
        connect(this, &QuickPrintPrivate::notifyLoadFinished, this, [&ret, &loop](int, bool error) {
            ret = !error;
            loop.quit();
        });
        loop.exec();
    }

    return ret;
}

/**
   @class QuickPrint
   @brief 图片打印后端处理，提供图像加载和打印界面展示
        使用 showPrintDialog() 执行同步打印，函数将等待数据解析完成后弹出打印窗口，
        注意此函数可能会占用GUI线程，因此不提供加载动画。
        使用 showPrintDialogAsync() 执行异步打印，（硬件支持）放入子线程加载数据，
        加载时间超过500ms提供加载动画，加载完成后自动弹出加载界面。
 */
QuickPrint::QuickPrint(QObject *parent)
    : QObject(parent)
    , dd_ptr(new QuickPrintPrivate(this))
{
}

QuickPrint::~QuickPrint() {}

/**
   @brief 执行同步打印文件 \a fileList (指此函数在此使用 EventLoop 等待)，阻塞当前函数处理。
        加载完成后弹出打印预览对话框，等待操作完成后退出函数，对话框设置 \a parentWidget 为父窗口。
 */
bool QuickPrint::showPrintDialog(const QStringList &fileList, QWidget *parentWidget)
{
    Q_D(QuickPrint);

    if (!showPrintDialogAsync(fileList, parentWidget)) {
        return false;
    }
    return d->waitLoadFinished();
}

/**
   @brief 执行异步打印文件 \a fileList ，在后台多线程加载。加载完成后自动弹出打印预览对话框，
        对话框设置 \a parentWidget 为父窗口，对话框交互结束后，抛出 printFinished() 信号
   @note 部分硬件环境可能不支持多线程，线程数小于2等
 */
bool QuickPrint::showPrintDialogAsync(const QStringList &fileList, QWidget *parentWidget)
{
    Q_D(QuickPrint);

    d->startLoadSpinnerTimer();
    d->parentWidget = parentWidget;
    if (!d->setPrintImage(fileList)) {
        return false;
    }
    // 等待 loadFinished() 信号，触发展示打印窗口
    return true;
}

/**
   @return 是否处于加载状态
 */
bool QuickPrint::isLoading()
{
    return d_func()->imageLoader->isLoading();
}

/**
   @brief 中断打印，清空打印缓存数据。
 */
void QuickPrint::cancel()
{
    Q_D(QuickPrint);

    d->stopSpinner();
    d->imageLoader->cancel();
}

// C-Style API
extern "C" {

/**
   @brief 导出的C接口，使用EventLoop等待数据加载，弹出打印对话框。
 */
bool quickPrintDialog(const QStringList &fileList, QWidget *parentWidget)
{
    QuickPrint print;
    return print.showPrintDialog(fileList, parentWidget);
}
}
