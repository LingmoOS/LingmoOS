#include "blurmanager.h"
#include "lingmo-blur-manager-client-protocol.h"

#include <QWindow>
#include <QRegion>
#include <QGuiApplication>

#include "blurxcbinterface.h"
#include "blurwaylandinterface.h"

namespace kdk
{

static BlurManager* g_blurmanager = nullptr;

BlurManager::BlurManager(QObject *parent)
    :QObject(parent)
{
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
         m_blurInterface = new BlurWaylandInterface(this);
    else
        m_blurInterface = new BlurXcbInterface(this);
}

BlurAbstractInterface *BlurManager::interface()
{
    return m_blurInterface;
}

BlurManager *BlurManager::self()
{
    if(g_blurmanager)
        return g_blurmanager;
    g_blurmanager = new BlurManager();
    return g_blurmanager;
}

bool BlurManager::setBlurBehindWithStrength(QWindow *window, bool enable, const QRegion &region, uint32_t strength)
{
    return self()->interface()->setBlurBehindWithStrength(window, enable, region, strength);
}

}
