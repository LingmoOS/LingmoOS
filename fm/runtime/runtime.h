#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class RunTime : public QObject
{
    Q_OBJECT
public:
    explicit RunTime(QObject *parent = nullptr);
    ~RunTime();
};

#endif // RUNTIME_H
