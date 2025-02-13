#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include "accountsmanager.h"
#include "useraccount.h"
#include "usersmodel.h"

class QmlPlugins : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char * uri) override {
        qmlRegisterType<QtAccountsService::AccountsManager>(uri, 1, 0, "AccountsManager");
        qmlRegisterType<QtAccountsService::UserAccount>(uri, 1, 0, "UserAccount");
        qmlRegisterType<QtAccountsService::UsersModel>(uri, 1, 0, "UsersModel");
    }
};

#include "qmlplugins.moc"