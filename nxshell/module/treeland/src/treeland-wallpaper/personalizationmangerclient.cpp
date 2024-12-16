// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "personalizationmangerclient.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QScreen>
#include <QStandardPaths>
#include <QTranslator>
#include <QtWaylandClient/QWaylandClientExtension>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

PersonalizationV1::PersonalizationV1()
    : QWaylandClientExtensionTemplate<PersonalizationV1>(1)
{
    connect(this,
            &PersonalizationV1::activeChanged,
            this,
            &PersonalizationV1::onActiveChanged);
    m_cacheDirectory =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/wallpaper/";

    QTranslator *translate = new QTranslator(this);
    auto dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &dir : dirs) {
        if (translate
                ->load(QLocale::system(), "wallpaper", ".", dir + "/ddm/translations", ".qm")) {
            qApp->installTranslator(translate);
        }
    }

    Q_FOREACH (QScreen *screen, qApp->screens()) {
        WallpaperMetaData *meta = new WallpaperMetaData;
        meta->screen = screen;
        meta->output = screen->name();
        m_screens.insert(meta->output, meta);
    }
}

PersonalizationV1::~PersonalizationV1()
{
    if (m_wallpaperContext) {
        m_wallpaperContext->deleteLater();
    }

    if (m_modes.size() > 0) {
        qDeleteAll(m_modes);
    }

    if (m_screens.size() > 0) {
        qDeleteAll(m_screens);
    }
}

void PersonalizationV1::onActiveChanged()
{
    if (!isActive())
        return;

    if (!m_wallpaperContext) {
        m_wallpaperContext = new PersonalizationWallpaper(get_wallpaper_context());
        connect(m_wallpaperContext,
                &PersonalizationWallpaper::metadataChanged,
                this,
                &PersonalizationV1::onMetadataChanged);
    }

    m_wallpaperContext->get_metadata();
}

void PersonalizationV1::addWallpaper(const QString &path)
{
    if (!m_wallpaperContext)
        return;

    QString local_path = QUrl(path).toLocalFile();

    QFileInfo file_info(local_path);
    QString dest = m_cacheDirectory + file_info.fileName();

    QDir dir(m_cacheDirectory);
    if (!dir.exists()) {
        dir.mkpath(m_cacheDirectory);
    }

    QFile::copy(local_path, dest);
}

void PersonalizationV1::removeWallpaper(const QString &path, const QString &group, int index)
{
    if (m_modes[group]->currentIndex() == index)
        return;

    QString local_path = QUrl(path).toLocalFile();
    QFile::remove(local_path);

    m_modes[group]->remove(index);
}

void PersonalizationV1::changeWallpaper(const QString &path,
                                             const QString &output,
                                             const QString &group,
                                             int index,
                                             quint32 op,
                                             bool isdark)
{
    if (!m_wallpaperContext)
        return;

    QString dest = QUrl(path).toLocalFile();
    if (dest.isEmpty())
        return;

    QFile file(dest);
    if (file.open(QIODevice::ReadOnly)) {
        m_modes[group]->setCurrentIndex(index);
        setCurrentGroup(group);

        auto meta_data = m_screens.value(output);
        if (meta_data != nullptr) {
            meta_data->imagePath = dest;
            meta_data->output = output;
            meta_data->currentIndex = index;
            meta_data->options = op;
            meta_data->isdark = isdark;

            m_wallpaperContext->set_on(op);
            m_wallpaperContext->set_isdark(isdark);
            m_wallpaperContext->set_fd(file.handle(), converToJson(m_screens));
            m_wallpaperContext->set_output(meta_data->output);
            m_wallpaperContext->commit();

            Q_EMIT wallpaperChanged(dest);
        }
    }
}

void PersonalizationV1::setBackground(const QString &path,
                                           const QString &group,
                                           int index,
                                           bool isdark)
{
    changeWallpaper(path,
                    m_currentOutput,
                    group,
                    index,
                    PersonalizationWallpaper::options_background,
                    isdark);
}

void PersonalizationV1::setLockscreen(const QString &path,
                                           const QString &group,
                                           int index,
                                           bool isdark)
{
    changeWallpaper(path,
                    m_currentOutput,
                    group,
                    index,
                    PersonalizationWallpaper::options_lockscreen,
                    isdark);
}

void PersonalizationV1::setBoth(const QString &path, const QString &group, int index)
{
    changeWallpaper(path,
                    m_currentOutput,
                    group,
                    index,
                    PersonalizationWallpaper::options_lockscreen
                        | PersonalizationWallpaper::options_lockscreen);
}

QString PersonalizationV1::currentGroup()
{
    if (m_currentOutput.isEmpty())
        return QString();

    auto meta = m_screens[m_currentOutput];
    return meta->group;
}

void PersonalizationV1::setCurrentGroup(const QString &group)
{
    if (m_currentOutput.isEmpty())
        return;

    auto meta = m_screens[m_currentOutput];
    meta->group = group;
    Q_EMIT currentGroupChanged(group);
}

QString PersonalizationV1::output()
{
    return m_currentOutput;
}

void PersonalizationV1::setOutput(const QString &name)
{
    m_currentOutput = name;
    Q_EMIT outputChanged(m_currentOutput);
}

QStringList PersonalizationV1::outputModel()
{
    return m_screens.keys();
}

QString PersonalizationV1::cacheDirectory()
{
    return m_cacheDirectory;
}

WallpaperCardModel *PersonalizationV1::wallpaperModel(const QString &group, const QString &dir)
{
    if (m_modes.contains(group))
        return m_modes[group];

    auto model = new WallpaperCardModel(this);
    model->setDirectory(dir);
    m_modes[group] = model;
    return m_modes[group];
}

QString PersonalizationV1::converToJson(QMap<QString, WallpaperMetaData *> screens)
{
    QMapIterator<QString, WallpaperMetaData *> it(screens);

    QJsonObject json;
    while (it.hasNext()) {
        it.next();
        QJsonObject content;
        content.insert("Group", it.value()->group);
        content.insert("ImagePath", it.value()->imagePath);
        content.insert("Output", it.value()->output);
        content.insert("CurrentIndex", it.value()->currentIndex);
        content.insert("Options", QString::number(it.value()->options));

        json[it.key()] = content;
    }

    QJsonDocument json_doc(json);
    return json_doc.toJson(QJsonDocument::Compact);
}

void PersonalizationV1::onMetadataChanged(const QString &metadata)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(metadata.toLocal8Bit());

    if (!json_doc.isNull()) {
        QJsonObject json = json_doc.object();

        QMapIterator<QString, WallpaperMetaData *> it(m_screens);
        while (it.hasNext()) {
            it.next();
            QJsonObject context = json.value(it.key()).toObject();
            if (context.isEmpty())
                continue;

            it.value()->currentIndex = context["CurrentIndex"].toInt();
            it.value()->group = context["Group"].toString();
            it.value()->imagePath = context["ImagePath"].toString();
            it.value()->output = context["Output"].toString();
            it.value()->options = context["Options"].toInt();
        }
    }

    if (m_currentOutput.isEmpty())
        return;

    auto meta = m_screens[m_currentOutput];
    auto model = m_modes[meta->group];
    if (model) {
        model->setCurrentIndex(meta->currentIndex);
    }
    Q_EMIT wallpaperChanged(meta->imagePath);
}

PersonalizationWindow::PersonalizationWindow(struct ::personalization_window_context_v1 *object)
    : QWaylandClientExtensionTemplate<PersonalizationWindow>(1)
    , QtWayland::personalization_window_context_v1(object)
{
}

PersonalizationWallpaper::PersonalizationWallpaper(
    struct ::personalization_wallpaper_context_v1 *object)
    : QWaylandClientExtensionTemplate<PersonalizationWallpaper>(1)
    , QtWayland::personalization_wallpaper_context_v1(object)
{
}

void PersonalizationWallpaper::personalization_wallpaper_context_v1_metadata(
    const QString &metadata)
{
    Q_EMIT metadataChanged(metadata);
}
