#include "dbuscallthread.h"

DbusCallThread::DbusCallThread(QVariantList json, QString runtime, QString appName, QString debPath)
{
    m_runtime = runtime;
    m_appName = appName;
    m_jsonInfo = json;
    m_debPath = debPath;
}

void DbusCallThread::run()
{
    
    QDBusInterface *serviceInterface = new QDBusInterface("com.lingmo.runtime",
                                                          "/com/lingmo/runtime",
                                                          "com.lingmo.runtime.interface",
                                                          QDBusConnection::systemBus());
    serviceInterface->setTimeout(2147483647);

    if (!serviceInterface->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        return ;
    }
    serviceInterface->call(QDBus::Block, "kre_install_app", m_runtime, m_debPath);

    // 是否安装成功的判断
    QDBusReply<int> reply = serviceInterface->call("kre_install_status", m_runtime, m_appName);
    if (reply) {
        serviceInterface->call(QDBus::Block, "saveJsonFile", m_jsonInfo);
        emit installSuccess();
    } else {
        emit installFail();
    }
    delete serviceInterface;
    serviceInterface = nullptr;
}
