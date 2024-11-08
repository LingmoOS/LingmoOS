#include "asyncproviderproxycreator.h"
#include "appwidgetproviderproxy.h"

#include <QDebug>

namespace AppWidget {

AsyncProviderProxyCreator::AsyncProviderProxyCreator(QObject* parent)
    : QThread(parent)
{

}

void AsyncProviderProxyCreator::startCreatorByIndexAndName(int index, QString providername)
{
    m_providerName = providername;
    m_index = index;
    start();
}

void AsyncProviderProxyCreator::run()
{
   qDebug() << __FILE__ << __FUNCTION__ << "providername =" << m_providerName;
   AppWidgetProviderProxy* providerproxy = new AppWidgetProviderProxy(m_index, m_providerName);
   m_appWidgetProviderProxy = providerproxy;
}

AppWidgetProviderProxy* AsyncProviderProxyCreator::getProviderProxy()
{
    if (isFinished()) {
        return m_appWidgetProviderProxy;
    }
    return nullptr;
}

} // namespace AppWidget
