#ifndef CFILEJOB_H
#define CFILEJOB_H

#include <QThread>

class CFileJob : public QThread
{
    Q_OBJECT

public:
    explicit CFileJob(QObject *parent = nullptr);

signals:

};

#endif // CFILEJOB_H
