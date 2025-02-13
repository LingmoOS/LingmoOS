#include "background.h"
#include <QtConcurrent>

static QVariantList getBackgroundPaths()
{
    QVariantList list;
    QDirIterator it("/usr/share/backgrounds/lingmoos", QStringList() << "*.jpg" << "*.png", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString bg = it.next();
        list.append(QVariant(bg));
    }
    std::sort(list.begin(), list.end());
    return list;
}

Background::Background(QObject *parent)
    : QObject(parent)
    , m_interface("com.lingmo.Settings",
                  "/Theme",
                  "com.lingmo.Theme",
                  QDBusConnection::sessionBus(), this)
{
    if (m_interface.isValid()) {
        m_currentPath = m_interface.property("wallpaper").toString();

        QDBusConnection::sessionBus().connect(m_interface.service(),
                                              m_interface.path(),
                                              m_interface.interface(),
                                              "backgroundTypeChanged", this, SIGNAL(backgroundTypeChanged()));
        QDBusConnection::sessionBus().connect(m_interface.service(),
                                              m_interface.path(),
                                              m_interface.interface(),
                                              "backgroundColorChanged", this, SIGNAL(backgroundColorChanged()));
    }
}

QVariantList Background::backgrounds()
{
    QFuture<QVariantList> future = QtConcurrent::run(&getBackgroundPaths);
    QVariantList list = future.result();
    return list;
}

QString Background::currentBackgroundPath()
{
    return m_currentPath;
}

void Background::setBackground(QString path)
{
    if (m_currentPath != path && !path.isEmpty()) {
        m_currentPath = path;

        if (m_interface.isValid()) {
            m_interface.call("setWallpaper", path);
            emit backgroundChanged();
        }
    }
}

int Background::backgroundType()
{
    return m_interface.property("backgroundType").toInt();
}

void Background::setBackgroundType(int type)
{
    m_interface.call("setBackgroundType", QVariant::fromValue(type));
}

QString Background::backgroundColor()
{
    return m_interface.property("backgroundColor").toString();
}

void Background::setBackgroundColor(const QString &color)
{
    m_interface.call("setBackgroundColor", QVariant::fromValue(color));
}
