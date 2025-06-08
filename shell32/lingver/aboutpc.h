#ifndef ABOUTPC_H
#define ABOUTPC_H

#include <QObject>

class AboutPC : public QObject
{
    Q_OBJECT

public:
    explicit AboutPC(QObject *parent = nullptr);
    ~AboutPC();
};
#endif // ABOUTPC_H
