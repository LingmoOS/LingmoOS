#ifndef PASSWORD_H
#define PASSWORD_H

#include <QObject>

class Password : public QObject
{
    Q_OBJECT
public:
    Password(QObject *parent = nullptr);

    Q_INVOKABLE QString cryptPassword(const QString &password) const;
};

#endif // PASSWORD_H
