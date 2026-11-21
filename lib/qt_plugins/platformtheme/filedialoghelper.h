#ifndef FILEDIALOGHELPER_H
#define FILEDIALOGHELPER_H

#include "waylandexport.h"

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QEventLoop>
#include <QUrl>
#include <qpa/qplatformdialoghelper.h>

// Drives org.freedesktop.portal.FileChooser, so every Qt application shows the
// same dialog as sandboxed and non-Qt ones.
class FileDialogHelper : public QPlatformFileDialogHelper
{
    Q_OBJECT

public:
    FileDialogHelper();
    ~FileDialogHelper() override;

    void exec() override;
    bool show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent) override;
    void hide() override;

    bool defaultNameFilterDisables() const override;
    void setDirectory(const QUrl &directory) override;
    QUrl directory() const override;
    void selectFile(const QUrl &filename) override;
    QList<QUrl> selectedFiles() const override;
    void setFilter() override;
    void selectNameFilter(const QString &filter) override;
    QString selectedNameFilter() const override;
    bool isSupportedUrl(const QUrl &url) const override;

private slots:
    void gotResponse(uint response, const QVariantMap &results, const QDBusMessage &message);

private:
    void closeRequest();
    void disconnectResponse(const QString &path);

    // Bumped by every show() and hide(); a portal reply from an earlier show
    // is closed instead of adopted.
    quint64 m_showSerial = 0;
    QDBusObjectPath m_request;
    WaylandExport m_parentExport;
    QEventLoop m_loop;
    QUrl m_directory;
    QUrl m_selectedFile;
    QList<QUrl> m_selectedFiles;
    QString m_selectedNameFilter;
};

#endif // FILEDIALOGHELPER_H
