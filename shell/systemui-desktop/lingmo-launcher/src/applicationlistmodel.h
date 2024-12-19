#ifndef APPLICATIONLISTMODEL_H
#define APPLICATIONLISTMODEL_H

#include <QObject>
#include "basemodel.h"
#include "launcheritem.h"
#include "type.h"

class ApplicationListModel : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationListModel(QObject *parent = nullptr);
    ~ApplicationListModel();

    void setAppData(ApplicationData data);
};

#endif // APPLICATIONLISTMODEL_H
