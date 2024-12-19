#ifndef LISTMODELMANAGER_H
#define LISTMODELMANAGER_H

#include <QObject>

class ListModelManager : public QObject
{
    Q_OBJECT

public:
    explicit ListModelManager(QObject *parent = nullptr);

private:
    QStringList m_pageModel;

};

#endif // LISTMODELMANAGER_H
