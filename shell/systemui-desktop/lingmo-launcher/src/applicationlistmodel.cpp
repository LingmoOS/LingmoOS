#include "applicationlistmodel.h"
#include <QMetaType>

ApplicationListModel::ApplicationListModel(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<ApplicationListModel*>();
}

ApplicationListModel::~ApplicationListModel()
{

}

void ApplicationListModel::setAppData(ApplicationData data)
{
    LauncherItem *item = new LauncherItem(this);
    item->setName(data.name);
    item->setIcon(data.icon);
    item->setEntryPath(data.entryPath);
    item->setStorageId(data.storageId);
    item->setLocation(data.location);
    item->setWindow(data.window);

//    addItem(item);
}
