#ifndef FOO_HELPER_H
#define FOO_HELPER_H

#include <QObject>

#include <actionreply.h>

using namespace KAuth;

class FdHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    ActionReply standardaction(QVariantMap args);
};

#endif
