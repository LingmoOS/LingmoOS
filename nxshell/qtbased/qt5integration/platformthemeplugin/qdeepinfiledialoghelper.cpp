/*
 * SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "qdeepinfiledialoghelper.h"
#include "qdeepintheme.h"

#include "filedialogmanager_interface.h"
#include "filedialog_interface.h"

#include <QDialog>
#include <QEvent>
#include <QWindow>
#include <QDBusObjectPath>
#include <QFileDialog>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qnativeinterface.h>
#define DISPLAY qApp->nativeInterface<QNativeInterface::QX11Application>()->display()
#else
#include <QX11Info>
#define DISPLAY QX11Info::display()
#endif
#include <QDebug>
#include <QApplication>
#include <QLoggingCategory>
#include <private/qguiapplication_p.h>

#include <X11/Xlib.h>

#include <DPlatformHandle>
DGUI_USE_NAMESPACE

QT_BEGIN_NAMESPACE
#ifndef DIALOG_SERVICE
#define DIALOG_SERVICE "com.deepin.filemanager.filedialog"
#endif
Q_LOGGING_CATEGORY(fileDialogHelper, "dtk.qpa.fileDialogHelper");

QList<QUrl> stringList2UrlList(const QStringList &list)
{
    QList<QUrl> urlList;

    for (const QString &str : list)
        urlList << str;

    return urlList;
}

QStringList urlList2StringList(const QList<QUrl> &list)
{
    QStringList stringList;

    for (const QUrl &url : list)
        stringList << url.toString();

    return stringList;
}

DFileDialogManager *QDeepinFileDialogHelper::manager = Q_NULLPTR;
QString QDeepinFileDialogHelper::dialogService = QStringLiteral();

QDeepinFileDialogHelper::QDeepinFileDialogHelper()
{
    connect(this, &QDeepinFileDialogHelper::accept, this, [this] {
        if (sourceDialog && filedlgInterface) {
            const QMap<QString, QVariant> &map_lineedit = filedlgInterface->allCustomWidgetsValue(LineEditType);
            const QMap<QString, QVariant> &map_combobox = filedlgInterface->allCustomWidgetsValue(ComboBoxType);

            auto map_eidt_begin = map_lineedit.constBegin();

            while (map_eidt_begin != map_lineedit.constEnd()) {
                sourceDialog->setProperty(QString("_dtk_widget_lineedit_%1_value").arg(map_eidt_begin.key()).toUtf8(),
                                          map_eidt_begin.value());
                ++map_eidt_begin;
            }

            auto map_combobox_begion = map_combobox.constBegin();

            while (map_combobox_begion != map_combobox.constEnd()) {
                sourceDialog->setProperty(QString("_dtk_widget_combobox_%1_value").arg(map_combobox_begion.key()).toUtf8(),
                                          map_combobox_begion.value());
                ++map_combobox_begion;
            }
        }
    });
}

QDeepinFileDialogHelper::~QDeepinFileDialogHelper()
{
    if (auxiliaryWindow) {
        // Note that we must hide auxiliaryWindow if it's modal to avoid direct invocation of deleteLater() to QDeepinFileDialogHelper
        // When someone creates a QFileDialog, runs exec amd uses a timer to invoke QFileDialog::deleteLater later. auxiliaryWindow
        // will just be deleted but not be hidden. Mainwindow will keep blocked by a dead modal window.
        hideAuxiliaryWindow();
        auxiliaryWindow->deleteLater();
    }

    if (filedlgInterface) {
        filedlgInterface->deleteLater(); // dbus
        filedlgInterface->QObject::deleteLater();
    }
}

void QDeepinFileDialogHelper::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive && filedlgInterface)
        filedlgInterface->activateWindow();
}

static inline void setTransientForHint(WId wid, WId propWid)
{
    if (!qApp->platformName().contains("xcb"))
        return;

    // set WM_TRANSIENT_FOR propWid for wid
    // to make sure filedialog window on the top of app window
    XSetTransientForHint(DISPLAY, wid, propWid);
}

void QDeepinFileDialogHelper::onWindowActiveChanged()
{
    if (!filedlgInterface)
        return;

    // FIX dtk#65
    // XSetTransientForHint 有可能被覆盖为桌面的 id,当窗口激活时再次设置一次
    if (filedlgInterface->windowActive() && auxiliaryWindow &&
            auxiliaryWindow->parent(QWindow::IncludeTransients)) {
        Window fileDlgWId = filedlgInterface->winId();
        Window parentWId = auxiliaryWindow->parent(QWindow::IncludeTransients)->winId();
        setTransientForHint(fileDlgWId, parentWId);
    }

    if (DPlatformHandle::isDXcbPlatform()) {
        QWindow *focus_window = qApp->focusWindow();
        if (!focus_window)
            return;

        if (focus_window->type() != Qt::Widget
                && focus_window->type() != Qt::Window
                && focus_window->type() != Qt::Dialog) {
            return;
        }

        if (!filedlgInterface->windowActive() && qApp->applicationState() == Qt::ApplicationActive) {
            filedlgInterface->activateWindow();
        }
    }
}

bool QDeepinFileDialogHelper::show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent)
{
    ensureDialog();
    applyOptions();

    if (filedlgInterface) {
        if (parent)
            activeWindow = parent;
        else
            activeWindow = QGuiApplication::focusWindow();

        filedlgInterface->setParent(parent);

        Qt::WindowFlags nd_flags = static_cast<Qt::WindowFlags>(filedlgInterface->windowFlags());
        Qt::WindowFlags need_flags = Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                                        | Qt::WindowMinMaxButtonsHint | Qt::WindowContextHelpButtonHint
                                        | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput
                                        | Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnBottomHint
                                        | Qt::WindowCloseButtonHint | Qt::BypassWindowManagerHint;

        if (flags & need_flags)
            filedlgInterface->setWindowFlags(nd_flags | (flags & need_flags));

        static bool i_am_dbus_server = iAmFileDialogDBusServer();

        if (i_am_dbus_server) {
            WId native_dialog_winId = filedlgInterface->winId();
            QWindow *real_native_dialog = nullptr;

            for (QWindow *window : qGuiApp->topLevelWindows()) {
                if (window->winId() == native_dialog_winId) {
                    real_native_dialog = window;
                    break;
                }
            }

            if (real_native_dialog) {
                real_native_dialog->setTransientParent(parent);
                real_native_dialog->setModality(modality);
                // call later
                QMetaObject::invokeMethod(filedlgInterface.data(), "show", Qt::QueuedConnection);

                return true;
            }
        }

        auxiliaryWindow->setParent(parent);
        auxiliaryWindow->setFlags(flags);
        auxiliaryWindow->setModality(modality);

        if (modality != Qt::NonModal) {
            QGuiApplicationPrivate::showModalWindow(auxiliaryWindow);

            if (modality == Qt::ApplicationModal) {
                connect(qApp, &QGuiApplication::applicationStateChanged, this, &QDeepinFileDialogHelper::onApplicationStateChanged, Qt::UniqueConnection);
                connect(filedlgInterface, &DFileDialogHandle::windowActiveChanged, this, &QDeepinFileDialogHelper::onWindowActiveChanged, Qt::UniqueConnection);
            }
        }
    }

    if (filedlgInterface)
        filedlgInterface->show();

    if (filedlgInterface && parent) {
        setTransientForHint(filedlgInterface->winId(), parent->winId());
    }
    // 如果没有dbus接口 return false 至少可以显示默认的文件选择对话框
    return filedlgInterface;
}

void QDeepinFileDialogHelper::exec()
{
    qCDebug(fileDialogHelper) << "exec";

    ensureDialog();

    // 快速打开关闭多次有一定概率出现没有调用 show 的情况,这里先 show
    // 保证exec一定会显示出现
    if (filedlgInterface)
        filedlgInterface->show();

    // block input to the window, allow input to other GTK dialogs
    QEventLoop loop;
    execLoop = &loop;
    connect(this, &QPlatformDialogHelper::accept, &loop, &QEventLoop::quit);
    connect(this, &QPlatformDialogHelper::reject, &loop, &QEventLoop::quit);
    connect(this, &QObject::destroyed, &loop, &QEventLoop::quit);
    loop.exec();

    qCDebug(fileDialogHelper) << "Exec finished, dispose event loop.";
}

void QDeepinFileDialogHelper::hide()
{
    qCDebug(fileDialogHelper) << "hide";

    ensureDialog();

    if (filedlgInterface)
        filedlgInterface->hide();

    if (auxiliaryWindow)
        hideAuxiliaryWindow();

    if (execLoop && execLoop->isRunning()) {
        execLoop->quit();
    }
}

bool QDeepinFileDialogHelper::defaultNameFilterDisables() const
{
    qCDebug(fileDialogHelper) << __FUNCTION__;

    return true;
}

void QDeepinFileDialogHelper::setDirectory(const QUrl &directory)
{
    if (QDeepinFileDialogHelper::directory() == directory) {
        return;
    }

    qCDebug(fileDialogHelper) << __FUNCTION__ << directory;

    ensureDialog();

    if (filedlgInterface)
        filedlgInterface->setDirectoryUrl(directory.toString());
    else
        options()->setInitialDirectory(directory);
}

QUrl QDeepinFileDialogHelper::directory() const
{
    qCDebug(fileDialogHelper) << __FUNCTION__;

    ensureDialog();

    if (filedlgInterface)
        return QUrl(filedlgInterface->directoryUrl());

    return options()->initialDirectory();
}

void QDeepinFileDialogHelper::selectFile(const QUrl &fileUrl)
{
    qCDebug(fileDialogHelper) << __FUNCTION__ << fileUrl;

    ensureDialog();

    if (filedlgInterface)
        filedlgInterface->selectUrl(fileUrl.toString());
    else
        options()->setInitiallySelectedFiles({fileUrl});
}

QList<QUrl> QDeepinFileDialogHelper::selectedFiles() const
{
    qCDebug(fileDialogHelper) << __FUNCTION__;

    ensureDialog();

    if (filedlgInterface)
        return stringList2UrlList(filedlgInterface->selectedUrls());

    return options()->initiallySelectedFiles();
}

void QDeepinFileDialogHelper::setFilter()
{
    qCDebug(fileDialogHelper) << __FUNCTION__;

    ensureDialog();

    if (filedlgInterface)
        filedlgInterface->setFilter(options()->filter());
}

void QDeepinFileDialogHelper::selectNameFilter(const QString &filter)
{
    qCDebug(fileDialogHelper) << __FUNCTION__ << filter;

    ensureDialog();

    if (filedlgInterface)
        filedlgInterface->selectNameFilter(filter);
    else
        options()->setInitiallySelectedNameFilter(filter);
}

QString QDeepinFileDialogHelper::selectedNameFilter() const
{
    qCDebug(fileDialogHelper) << __FUNCTION__;

    ensureDialog();

    if (filedlgInterface)
        return filedlgInterface->selectedNameFilter();

    return options()->initiallySelectedNameFilter();
}

void QDeepinFileDialogHelper::initDBusFileDialogManager()
{
    if (manager)
        return;

    if (qEnvironmentVariableIsSet("_d_fileDialogServiceName")) {
        dialogService = qgetenv("_d_fileDialogServiceName");
    } else {
        dialogService = DIALOG_SERVICE;
    }

    const auto *managerObjectPath = "/com/deepin/filemanager/filedialogmanager";
    auto conn = QDBusConnection::sessionBus();
    auto reply = conn.call(QDBusMessage::createMethodCall(dialogService, managerObjectPath , "org.freedesktop.DBus.Peer", "Ping"));

    if(reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(fileDialogHelper) << reply.errorMessage();
    }

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(dialogService).value()
        || !QStandardPaths::findExecutable("dde-desktop").isEmpty()) {
        manager = new DFileDialogManager(dialogService, managerObjectPath, QDBusConnection::sessionBus());
    }
}

bool QDeepinFileDialogHelper::iAmFileDialogDBusServer()
{
    static bool _value = manager->connection().interface()->servicePid(manager->service()) == static_cast<uint>(getpid());

    return _value;
}

void QDeepinFileDialogHelper::ensureDialog() const
{
    if (filedlgInterface)
        return;

    if (manager) {
        QDBusPendingReply<QDBusObjectPath> reply = manager->createDialog(QString());
        reply.waitForFinished();
        const QString &path = reply.value().path();

        if (path.isEmpty()) {
            qCWarning(fileDialogHelper) << "Can not create native dialog, Will be use QFileDialog";
        } else {
            filedlgInterface = new DFileDialogHandle(dialogService, path, QDBusConnection::sessionBus());
            auxiliaryWindow = new QWindow();
            auxiliaryWindow->setObjectName("QDeepinFileDialogHelper_auxiliaryWindow");

            connect(filedlgInterface, &QObject::destroyed, auxiliaryWindow, &QWindow::deleteLater);
            connect(filedlgInterface, &QObject::destroyed, filedlgInterface, &DFileDialogHandle::deleteLater);
            connect(filedlgInterface, &DFileDialogHandle::accepted, this, &QDeepinFileDialogHelper::accept);
            connect(filedlgInterface, &DFileDialogHandle::rejected, this, &QDeepinFileDialogHelper::reject);
            connect(filedlgInterface, &DFileDialogHandle::destroyed, this, &QDeepinFileDialogHelper::reject);
            connect(filedlgInterface, &DFileDialogHandle::destroyed, this, [this](){
                qCWarning(fileDialogHelper) << "filedialog dbus service destroyed.";
                if (filedlgInterface) {
                    filedlgInterface->QObject::deleteLater();
                    filedlgInterface = nullptr;
                }

                if (auxiliaryWindow && auxiliaryWindow->isModal() && qApp->modalWindow() == auxiliaryWindow)
                    hideAuxiliaryWindow();
            });

            QTimer *heartbeatTimer = new QTimer(filedlgInterface);

            connect(heartbeatTimer, &QTimer::timeout, this, [this, heartbeatTimer] {
                if (!filedlgInterface)
                    return ;

                QDBusPendingReply<> reply = filedlgInterface->makeHeartbeat();

                reply.waitForFinished();

                if (reply.isError()) {
                    qCWarning(fileDialogHelper) << "Make heartbeat is failed:" << reply.error();

                    if (reply.error().type() == QDBusError::UnknownMethod) {
                        qCWarning(fileDialogHelper) << "Make heartbeat is't support for current dbus file dialog, Will be stop heartbeat timer.";

                        heartbeatTimer->stop();
                        return;
                    }

                    filedlgInterface->QObject::deleteLater();
                    const_cast<QDeepinFileDialogHelper*>(this)->reject();
                }
            });
            int heartbeatInterval = filedlgInterface->heartbeatInterval();
            heartbeatTimer->setInterval(qMax(1 * 1000, qMin(int(heartbeatInterval / 1.5), heartbeatInterval - 5 * 1000)));
            heartbeatTimer->start();
        }
    }

    if (!filedlgInterface && qobject_cast<QApplication*>(qGuiApp)) {
        QDeepinTheme::m_usePlatformNativeDialog = false;
    }
}

void QDeepinFileDialogHelper::applyOptions()
{
    QFileDialogOptions *options = this->options().data();

    for (int i = 0; i < QFileDialogOptions::DialogLabelCount; ++i) {
        if (options->isLabelExplicitlySet((QFileDialogOptions::DialogLabel)i)) {
            if (filedlgInterface)
                filedlgInterface->setLabelText(i, options->labelText((QFileDialogOptions::DialogLabel)i));
            else
                qCWarning(fileDialogHelper) << "DFileDialogHandle invalid!!";

        }
    }
    if (filedlgInterface) {
        filedlgInterface->setOptions(int(options->options()));
        filedlgInterface->setFilter(int(options->filter()));
        filedlgInterface->setWindowTitle(options->windowTitle());
        filedlgInterface->setViewMode(int(options->viewMode()));
        filedlgInterface->setFileMode(int(options->fileMode()));
        filedlgInterface->setAcceptMode(int(options->acceptMode()));
        filedlgInterface->setNameFilters(options->nameFilters());
    }


    if (options->initialDirectory().isLocalFile())
        setDirectory(options->initialDirectory());

    foreach (const QUrl &filename, options->initiallySelectedFiles())
        selectFile(filename);

    selectNameFilter(options->initiallySelectedNameFilter());

    if (!sourceDialog) {
        sourceDialog = reinterpret_cast<QObject*>(qvariant_cast<quintptr>(property("_dtk_widget_QFileDialog")));

        if (sourceDialog && filedlgInterface) {
            const QStringList lineedit_list = sourceDialog->property("_dtk_widget_custom_lineedit_list").toStringList();
            const QStringList combobox_list = sourceDialog->property("_dtk_widget_custom_combobox_list").toStringList();

            filedlgInterface->beginAddCustomWidget();

            for (const QString &i : lineedit_list)
                filedlgInterface->addCustomWidget(LineEditType, i);

            for (const QString &i : combobox_list)
                filedlgInterface->addCustomWidget(ComboBoxType, i);

            filedlgInterface->endAddCustomWidget();

            const QVariant & mixedSelection = sourceDialog->property("_dtk_widget_filedialog_mixed_selection");
            if (mixedSelection.isValid()) {
                filedlgInterface->setAllowMixedSelection(mixedSelection.toBool());
            }
        }
    }
}

void QDeepinFileDialogHelper::hideAuxiliaryWindow() const
{
    QGuiApplicationPrivate::hideModalWindow(auxiliaryWindow);

    if (activeWindow)
        activeWindow->requestActivate();
}
QT_END_NAMESPACE
