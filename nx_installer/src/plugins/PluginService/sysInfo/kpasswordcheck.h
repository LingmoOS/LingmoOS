#ifndef KPASSWORDCHECK_H
#define KPASSWORDCHECK_H

#include <QObject>
#include <pwquality.h>
#include <string.h>
#include "serverdll.h"
#include <QObject>
#include <QScopedPointer>
#include "ksystemsetting_unit.h"
//密码复杂度的配置文件路径
#define PWQUALITYPATH   "/etc/security/pwquality.conf"
namespace KServer {

class KPasswordCheck : public QObject
{
    Q_OBJECT
public:
    explicit KPasswordCheck(QObject *parent = nullptr);
    ~KPasswordCheck();
    static KPasswordCheck* getInstance();
    static KPasswordCheck* init();
    void releasePWquality();
    int initPWquality();
    int passwdCheck(QString passwd, QString& strerror, QString user);
    QString pwqualityError(char* buf, size_t len, int rv, void *quxerror);

public:

    static KPasswordCheck *m_PWInstance;
    pwquality_settings_t *m_settings;
    void *auxerror;
    char buf[256], passwd[256];
    char flag;
    int ret;
};
}

#endif // KPASSWORDCHECK_H
