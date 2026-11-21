#include "filedialoghelper.h"

#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeDatabase>
#include <QRandomGenerator>
#include <QWindow>

#include <algorithm>

static const auto portalService = QStringLiteral("org.freedesktop.portal.Desktop");
static const auto portalPath = QStringLiteral("/org/freedesktop/portal/desktop");
static const auto fileChooserInterface = QStringLiteral("org.freedesktop.portal.FileChooser");
static const auto requestInterface = QStringLiteral("org.freedesktop.portal.Request");

namespace {

// (us): 0 is a glob pattern, 1 a MIME type.
struct FilterRule {
    uint type = 0;
    QString value;
};

struct Filter {
    QString name;
    QList<FilterRule> rules;
};

} // namespace

Q_DECLARE_METATYPE(FilterRule)
Q_DECLARE_METATYPE(Filter)

namespace {

QDBusArgument &operator<<(QDBusArgument &argument, const FilterRule &rule)
{
    argument.beginStructure();
    argument << rule.type << rule.value;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FilterRule &rule)
{
    argument.beginStructure();
    argument >> rule.type >> rule.value;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Filter &filter)
{
    argument.beginStructure();
    argument << filter.name << filter.rules;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Filter &filter)
{
    argument.beginStructure();
    argument >> filter.name >> filter.rules;
    argument.endStructure();
    return argument;
}

} // namespace

static void registerFilterTypes()
{
    static bool registered = false;
    if (registered)
        return;
    qDBusRegisterMetaType<FilterRule>();
    qDBusRegisterMetaType<QList<FilterRule>>();
    qDBusRegisterMetaType<Filter>();
    qDBusRegisterMetaType<QList<Filter>>();
    registered = true;
}

// "Images (*.png *.jpg)" -> name "Images", rules for each glob.
static Filter filterFromNameFilter(const QString &nameFilter)
{
    Filter filter;
    const int bracket = nameFilter.lastIndexOf('(');
    filter.name = bracket > 0 ? nameFilter.left(bracket).trimmed() : nameFilter;
    const auto globs = QPlatformFileDialogHelper::cleanFilterList(nameFilter);
    for (const QString &glob : globs)
        filter.rules.append({0, glob});
    if (filter.name.isEmpty())
        filter.name = globs.join(QLatin1Char(' '));
    return filter;
}

static Filter filterFromMimeType(const QString &mimeType)
{
    const auto type = QMimeDatabase().mimeTypeForName(mimeType);
    // QFileDialog::setMimeTypeFilters() names the default type this way.
    if (type.isDefault())
        return filterFromNameFilter(QCoreApplication::translate("QFileDialog", "All files (*)"));

    Filter filter;
    filter.name = type.isValid() && !type.comment().isEmpty() ? type.comment() : mimeType;
    filter.rules.append({1, mimeType});
    return filter;
}

// The portal echoes the display name; Qt callers expect back one of the strings they set.
static QString qtNameFilter(const Filter &filter, const QFileDialogOptions &options)
{
    // With hidden details QFileDialog::selectedMimeTypeFilter() compares the bare name.
    if (options.testOption(QFileDialogOptions::HideNameFilterDetails))
        return filter.name;
    for (const QString &nameFilter : options.nameFilters()) {
        if (filterFromNameFilter(nameFilter).name == filter.name)
            return nameFilter;
    }
    return filter.name;
}

// The portal wants NUL-terminated paths as byte arrays.
static QByteArray portalPathValue(const QUrl &url)
{
    QByteArray path = QFile::encodeName(url.toLocalFile());
    if (!path.isEmpty())
        path.append('\0');
    return path;
}

FileDialogHelper::FileDialogHelper()
{
    registerFilterTypes();
}

FileDialogHelper::~FileDialogHelper()
{
    closeRequest();
}

void FileDialogHelper::exec()
{
    m_loop.exec();
}

bool FileDialogHelper::show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent)
{
    Q_UNUSED(flags)

    // A request still open from an earlier show() would keep its dialog and hook.
    closeRequest();
    const quint64 serial = ++m_showSerial;

    const auto dialogOptions = options();
    const bool saveMode = dialogOptions->acceptMode() == QFileDialogOptions::AcceptSave;
    const bool directoryMode = dialogOptions->fileMode() == QFileDialogOptions::Directory
            || dialogOptions->fileMode() == QFileDialogOptions::DirectoryOnly;

    const QString token = QStringLiteral("lingmo%1").arg(QRandomGenerator::global()->generate());
    QVariantMap portalOptions;
    portalOptions.insert(QStringLiteral("handle_token"), token);
    portalOptions.insert(QStringLiteral("modal"), modality != Qt::NonModal);
    portalOptions.insert(QStringLiteral("multiple"),
                         dialogOptions->fileMode() == QFileDialogOptions::ExistingFiles);
    portalOptions.insert(QStringLiteral("directory"), directoryMode);

    const QString acceptLabel = dialogOptions->labelText(QFileDialogOptions::Accept);
    if (!acceptLabel.isEmpty())
        portalOptions.insert(QStringLiteral("accept_label"), acceptLabel);

    QList<Filter> filters;
    for (const QString &mimeType : dialogOptions->mimeTypeFilters())
        filters.append(filterFromMimeType(mimeType));
    // QFileDialog::setMimeTypeFilters() also sets each type's name filter; send it once.
    const qsizetype mimeFilterCount = filters.size();
    for (const QString &nameFilter : dialogOptions->nameFilters()) {
        const Filter filter = filterFromNameFilter(nameFilter);
        const auto mimeEnd = filters.cbegin() + mimeFilterCount;
        if (std::none_of(filters.cbegin(), mimeEnd,
                         [&filter](const Filter &mime) { return mime.name == filter.name; }))
            filters.append(filter);
    }
    if (!filters.isEmpty()) {
        portalOptions.insert(QStringLiteral("filters"), QVariant::fromValue(filters));
        const QString wanted = m_selectedNameFilter.isEmpty()
                ? dialogOptions->initiallySelectedNameFilter()
                : m_selectedNameFilter;
        if (!wanted.isEmpty()) {
            const Filter current =
                    wanted.contains('/') && !dialogOptions->nameFilters().contains(wanted)
                    ? filterFromMimeType(wanted)
                    : filterFromNameFilter(wanted);
            for (const Filter &filter : std::as_const(filters)) {
                if (filter.name != current.name)
                    continue;
                portalOptions.insert(QStringLiteral("current_filter"), QVariant::fromValue(filter));
                m_selectedNameFilter = wanted;
                break;
            }
        }
    }

    const QUrl directory = m_directory.isValid() ? m_directory : dialogOptions->initialDirectory();
    if (directory.isLocalFile())
        portalOptions.insert(QStringLiteral("current_folder"), portalPathValue(directory));

    QUrl selected = m_selectedFile;
    if (!selected.isValid() && !dialogOptions->initiallySelectedFiles().isEmpty())
        selected = dialogOptions->initiallySelectedFiles().constFirst();
    if (selected.isLocalFile() && saveMode) {
        if (QFileInfo::exists(selected.toLocalFile()))
            portalOptions.insert(QStringLiteral("current_file"), portalPathValue(selected));
        portalOptions.insert(QStringLiteral("current_name"), selected.fileName());
    }

    // The dialog runs in another process, so it can only be parented onto an
    // identifier: an X11 window id, or an xdg-foreign handle on Wayland.
    QString parentWindow;
    if (parent && QGuiApplication::platformName() == QLatin1String("xcb"))
        parentWindow = QStringLiteral("x11:%1").arg(parent->winId(), 0, 16);
    else if (parent)
        parentWindow = m_parentExport.exportWindow(parent);

    QString title = dialogOptions->windowTitle();
    if (title.isEmpty())
        title = saveMode ? QCoreApplication::translate("QFileDialog", "Save As")
                        : directoryMode ? QCoreApplication::translate("QFileDialog", "Find Directory")
                                        : QCoreApplication::translate("QFileDialog", "Open");

    QDBusMessage message = QDBusMessage::createMethodCall(
            portalService, portalPath, fileChooserInterface,
            saveMode ? QStringLiteral("SaveFile") : QStringLiteral("OpenFile"));
    message << parentWindow << title << portalOptions;

    // The request path follows from the token, so Response is listened for before the
    // call: a backend that answers at once must not be missed.
    const QString sender = QDBusConnection::sessionBus().baseService().mid(1).replace(QLatin1Char('.'),
                                                                                     QLatin1Char('_'));
    m_request = QDBusObjectPath(
            QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2").arg(sender, token));
    QDBusConnection::sessionBus().connect(portalService, m_request.path(), requestInterface,
                                          QStringLiteral("Response"), this,
                                          SLOT(gotResponse(uint, QVariantMap, QDBusMessage)));

    auto *watcher =
            new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, serial](QDBusPendingCallWatcher *call) {
                call->deleteLater();
                const QDBusPendingReply<QDBusObjectPath> reply = *call;
                if (serial != m_showSerial) {
                    // Hidden or shown again before the portal answered: this
                    // dialog is nobody's now.
                    if (!reply.isError()) {
                        QDBusConnection::sessionBus().asyncCall(QDBusMessage::createMethodCall(
                                portalService, reply.value().path(), requestInterface,
                                QStringLiteral("Close")));
                    }
                    return;
                }
                if (reply.isError()) {
                    disconnectResponse(m_request.path());
                    m_request = QDBusObjectPath();
                    Q_EMIT reject();
                    m_loop.quit();
                    return;
                }
                // Already answered through the expected path.
                if (m_request.path().isEmpty())
                    return;
                // An older portal ignores the token and picks the path itself.
                if (reply.value() != m_request) {
                    disconnectResponse(m_request.path());
                    m_request = reply.value();
                    QDBusConnection::sessionBus().connect(
                            portalService, m_request.path(), requestInterface,
                            QStringLiteral("Response"), this,
                            SLOT(gotResponse(uint, QVariantMap, QDBusMessage)));
                }
            });
    return true;
}

void FileDialogHelper::hide()
{
    ++m_showSerial;
    closeRequest();
    m_loop.quit();
}

void FileDialogHelper::closeRequest()
{
    if (m_request.path().isEmpty())
        return;
    disconnectResponse(m_request.path());
    QDBusConnection::sessionBus().asyncCall(QDBusMessage::createMethodCall(
            portalService, m_request.path(), requestInterface, QStringLiteral("Close")));
    m_request = QDBusObjectPath();
}

// Each request has its own path, so its hook and bus match rule stay until
// disconnected here or the helper is destroyed.
void FileDialogHelper::disconnectResponse(const QString &path)
{
    QDBusConnection::sessionBus().disconnect(portalService, path, requestInterface,
                                             QStringLiteral("Response"), this,
                                             SLOT(gotResponse(uint, QVariantMap, QDBusMessage)));
}

void FileDialogHelper::gotResponse(uint response, const QVariantMap &results,
                                   const QDBusMessage &message)
{
    disconnectResponse(message.path());
    // A request already closed, or replaced before its reply arrived, can still answer.
    if (message.path() != m_request.path())
        return;
    m_request = QDBusObjectPath();
    m_selectedFiles.clear();

    if (response == 0) {
        const auto uris = results.value(QStringLiteral("uris")).toStringList();
        for (const QString &uri : uris)
            m_selectedFiles.append(QUrl(uri));
        if (!m_selectedFiles.isEmpty()) {
            m_selectedFile = m_selectedFiles.constFirst();
            m_directory = m_selectedFile.adjusted(QUrl::RemoveFilename);
            Q_EMIT directoryEntered(m_directory);
            Q_EMIT currentChanged(m_selectedFile);
        }
        const auto filter = qdbus_cast<Filter>(results.value(QStringLiteral("current_filter")));
        if (!filter.name.isEmpty()) {
            m_selectedNameFilter = qtNameFilter(filter, *options());
            Q_EMIT filterSelected(m_selectedNameFilter);
        }
        Q_EMIT accept();
    } else {
        Q_EMIT reject();
    }

    m_loop.quit();
}

bool FileDialogHelper::defaultNameFilterDisables() const
{
    return false;
}

void FileDialogHelper::setDirectory(const QUrl &directory)
{
    m_directory = directory;
}

QUrl FileDialogHelper::directory() const
{
    return m_directory;
}

void FileDialogHelper::selectFile(const QUrl &filename)
{
    m_selectedFile = filename;
}

QList<QUrl> FileDialogHelper::selectedFiles() const
{
    return m_selectedFiles;
}

void FileDialogHelper::setFilter()
{
}

void FileDialogHelper::selectNameFilter(const QString &filter)
{
    m_selectedNameFilter = filter;
}

QString FileDialogHelper::selectedNameFilter() const
{
    return m_selectedNameFilter;
}

bool FileDialogHelper::isSupportedUrl(const QUrl &url) const
{
    return url.isLocalFile();
}
