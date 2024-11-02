#ifndef ASYNCPROVIDERPROXYCREATOR_H
#define ASYNCPROVIDERPROXYCREATOR_H

#include <QThread>

namespace AppWidget {
class AppWidgetProviderProxy;
class AsyncProviderProxyCreator : public QThread
{
    Q_OBJECT

public:
    AsyncProviderProxyCreator(QObject *parent = nullptr);
    ~AsyncProviderProxyCreator() = default;

    void run() override;
    void startCreatorByIndexAndName(int index, QString providername);
    AppWidgetProviderProxy* getProviderProxy();

private:
    int m_index;
    QString m_providerName;
    AppWidgetProviderProxy* m_appWidgetProviderProxy = nullptr;

};
} // namespace AppWidget

#endif // ASYNCPROVIDERPROXYCREATOR_H
