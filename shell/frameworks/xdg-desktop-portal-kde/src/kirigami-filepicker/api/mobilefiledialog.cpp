// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "mobilefiledialog.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QStandardPaths>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "dirmodel.h"
#include "dirmodelutils.h"
#include "fileplacesmodel.h"
#include "mobilefiledialog_debug.h"

#include "filechooserqmlcallback.h"

constexpr auto URI = "org.kde.kirigamifilepicker";

MobileFileDialog::MobileFileDialog(QObject *parent)
    : QObject(parent)
    , m_engine(new QQmlApplicationEngine(this))
    , m_window(nullptr)
    , m_customTitleSet(false)
{
    qmlRegisterType<DirModel>(URI, 0, 1, "DirModel");
    qmlRegisterSingletonType<DirModelUtils>(URI, 0, 1, "DirModelUtils", [=](QQmlEngine *, QJSEngine *) {
        return new DirModelUtils;
    });
    qmlRegisterType<FileChooserQmlCallback>(URI, 0, 1, "FileChooserCallback");
    qmlRegisterType<FilePlacesModel>(URI, 0, 1, "FilePlacesModel");

    Q_INIT_RESOURCE(filepicker);

    auto *i18nContext = new KLocalizedContext(m_engine);
    m_engine->rootContext()->setContextObject(i18nContext);

    m_engine->load(QStringLiteral("qrc:/org.kde.kirigamifilepicker/FilePickerWindow.qml"));
    m_window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().first());
    m_callback = m_window->findChild<FileChooserQmlCallback *>(QStringLiteral("callback"));

    // Connect everything to callback
    connect(m_callback, &FileChooserQmlCallback::accepted, this, &MobileFileDialog::accepted);
    connect(m_callback, &FileChooserQmlCallback::cancel, this, &MobileFileDialog::cancel);
    connect(m_callback, &FileChooserQmlCallback::titleChanged, this, &MobileFileDialog::titleChanged);
    connect(m_callback, &FileChooserQmlCallback::selectMultipleChanged, this, &MobileFileDialog::selectMultipleChanged);
    connect(m_callback, &FileChooserQmlCallback::selectExistingChanged, this, &MobileFileDialog::selectExistingChanged);
    connect(m_callback, &FileChooserQmlCallback::nameFiltersChanged, this, &MobileFileDialog::nameFiltersChanged);
    connect(m_callback, &FileChooserQmlCallback::mimeTypeFiltersChanged, this, &MobileFileDialog::mimeTypeFiltersChanged);
    connect(m_callback, &FileChooserQmlCallback::folderChanged, this, &MobileFileDialog::folderChanged);
    connect(m_callback, &FileChooserQmlCallback::currentFileChanged, this, &MobileFileDialog::currentFileChanged);
    connect(m_callback, &FileChooserQmlCallback::acceptLabelChanged, this, &MobileFileDialog::acceptLabelChanged);
    connect(m_callback, &FileChooserQmlCallback::selectFolderChanged, this, &MobileFileDialog::selectFolderChanged);

    // Set default path for file dialog
    setFolder(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
}

// FileDialog methods pass through to the callback to provide a nice c++ api
QString MobileFileDialog::title() const
{
    return m_callback->title();
}

void MobileFileDialog::setTitle(const QString &title)
{
    // Don't accept an empty titles
    if (!title.isEmpty()) {
        m_customTitleSet = true;
        m_callback->setTitle(title);
    }
}

bool MobileFileDialog::selectMultiple() const
{
    return m_callback->selectMultiple();
}

void MobileFileDialog::setSelectMultiple(bool selectMultiple)
{
    m_callback->setSelectMultiple(selectMultiple);
}

bool MobileFileDialog::selectExisting() const
{
    return m_callback->selectExisting();
}

void MobileFileDialog::setSelectExisting(bool selectExisting)
{
    m_callback->setSelectExisting(selectExisting);

    // Detect that no custom title is set
    if (!m_customTitleSet) {
        if (selectFolder()) {
            setTitle(i18n("Select Folder"));
        } else {
            if (selectExisting) {
                setTitle(i18n("Open File"));
            } else {
                setTitle(i18n("Save File"));
            }
        }
    }
}

QStringList MobileFileDialog::nameFilters() const
{
    return m_callback->nameFilters();
}

void MobileFileDialog::setNameFilters(const QStringList &nameFilters)
{
    m_callback->setNameFilters(nameFilters);
}

QStringList MobileFileDialog::mimeTypeFilters() const
{
    return m_callback->mimeTypeFilters();
}

void MobileFileDialog::setMimeTypeFilters(const QStringList &mimeTypeFilters)
{
    m_callback->setMimeTypeFilters(mimeTypeFilters);
}

QUrl MobileFileDialog::folder() const
{
    return m_callback->folder();
}

void MobileFileDialog::setFolder(const QUrl &folder)
{
    m_callback->setFolder(folder);
}

QString MobileFileDialog::currentFile() const
{
    return m_callback->currentFile();
}

void MobileFileDialog::setCurrentFile(const QString &currentFile)
{
    m_callback->setCurrentFile(currentFile);
}

QString MobileFileDialog::acceptLabel() const
{
    return m_callback->acceptLabel();
}

void MobileFileDialog::setAcceptLabel(const QString &acceptLabel)
{
    m_callback->setAcceptLabel(acceptLabel);
}

bool MobileFileDialog::selectFolder() const
{
    return m_callback->selectFolder();
}

void MobileFileDialog::setSelectFolder(bool selectFolder)
{
    m_callback->setSelectFolder(selectFolder);
}

uint MobileFileDialog::exec()
{
    // Show window
    m_window->setVisible(true);
    m_window->raise();
    m_window->requestActivate();
    m_window->setIcon(QIcon::fromTheme(QStringLiteral("system-file-manager")));

    // Reset old data
    m_results.clear();

    uint exitCode = 0;

    const auto acceptedConn = connect(m_callback, &FileChooserQmlCallback::accepted, this, [this, &exitCode](const QList<QUrl> &urls) {
        m_results = urls;
        exitCode = 0;
        qDebug(KirigamiFilepicker) << "Got results" << m_results;
    });
    const auto cancelConn = connect(m_callback, &FileChooserQmlCallback::cancel, this, [&exitCode] {
        qDebug(KirigamiFilepicker) << "Quit without results";
        exitCode = 1;
    });

    QEventLoop loop;

    connect(this, &MobileFileDialog::accepted, &loop, &QEventLoop::quit);
    connect(this, &MobileFileDialog::cancel, &loop, &QEventLoop::quit);

    loop.exec();

    qDebug(KirigamiFilepicker) << "exiting file dialog";

    // Disconnect signals, to avoid them being connected twice
    // when the dialog is used again
    disconnect(acceptedConn);
    disconnect(cancelConn);

    if (m_window) {
        m_window->setVisible(false);
    }

    return exitCode;
}

QList<QUrl> MobileFileDialog::results() const
{
    return m_results;
}

#include "moc_mobilefiledialog.cpp"
