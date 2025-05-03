#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class RunTime : public QObject
{
    Q_OBJECT
public:
    explicit RunTime(QObject *parent = nullptr);
    ~RunTime();
private:
    void ignoreSigInt();
};


#endif // RUNTIME_H
