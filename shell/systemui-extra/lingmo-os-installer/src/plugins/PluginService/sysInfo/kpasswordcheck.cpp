#include "kpasswordcheck.h"
#include <QDebug>
#include "locale.h"
#include "libintl.h"
#include "stdio.h"
#include "pwquality.h"
namespace KServer {

KPasswordCheck* KPasswordCheck::m_PWInstance = nullptr;

KPasswordCheck::KPasswordCheck(QObject* parent ):QObject(parent)
{
    m_PWInstance = this;
    ret = 0;
    m_settings = pwquality_default_settings();
    initPWquality();
}

KPasswordCheck::~KPasswordCheck()
{

}

KPasswordCheck* KPasswordCheck::getInstance()
{
    return m_PWInstance;
}

KPasswordCheck* KPasswordCheck::init()
{
    if(!m_PWInstance) {
        qCritical() << "";
        return new KPasswordCheck();
    }
    return m_PWInstance;
}

int KPasswordCheck::initPWquality()
{
    ret = pwquality_read_config(m_settings,PWQUALITYPATH, &auxerror);
    qDebug() << "pwquality_read_config:" << ret;
    return ret;
}

void KPasswordCheck::releasePWquality()
{
    pwquality_free_settings(m_settings);
}

int KPasswordCheck::passwdCheck(QString passwd, QString& strerror, QString user)
{ 
    ret = pwquality_check(m_settings, passwd.toStdString().c_str(), NULL, user.toStdString().c_str(), &auxerror);
    strerror = pwqualityError(buf, sizeof(buf), ret, auxerror);
    return ret;
}

QString KPasswordCheck::pwqualityError(char* buf, size_t len, int rv, void *quxerror)
{
    static char intbuf[PWQ_MAX_ERROR_MESSAGE_LEN];

           if (buf == NULL) {
                   buf = intbuf;
                   len = sizeof(intbuf);
           }

           switch(rv) {
           case PWQ_ERROR_MEM_ALLOC:
                   if (auxerror) {
                           snprintf(buf, len, "%s - %s", QObject::tr("Memory allocation error when setting"), (const char *)auxerror);
                           free(auxerror);
                           return buf;
                   }
                   return QObject::tr("Memory allocation error");
           case PWQ_ERROR_SAME_PASSWORD:
                   return QObject::tr("The password is the same as the old one");
           case PWQ_ERROR_PALINDROME:
                   return QObject::tr("The password is a palindrome");
           case PWQ_ERROR_CASE_CHANGES_ONLY:
                   return QObject::tr("The password differs with case changes only");
           case PWQ_ERROR_TOO_SIMILAR:
                   return QObject::tr("The password is too similar to the old one");
           case PWQ_ERROR_USER_CHECK:
                   return QObject::tr("The password contains the user name in some form");
           case PWQ_ERROR_GECOS_CHECK:
                   return QObject::tr("The password contains words from the real name of the user in some form");
           case PWQ_ERROR_BAD_WORDS:
                   return QObject::tr("The password contains forbidden words in some form");
           case PWQ_ERROR_MIN_DIGITS:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains less than %1 digits")).arg((long)auxerror);
                   }
                   return QObject::tr("The password contains too few digits");
           case PWQ_ERROR_MIN_UPPERS:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains less than %1 uppercase letters")).arg((long)auxerror);
                   }
                   return QObject::tr("The password contains too few uppercase letters");
           case PWQ_ERROR_MIN_LOWERS:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains less than %1 lowercase letters")).arg((long)auxerror);
                   }
                   return QObject::tr("The password contains too few lowercase letters");
           case PWQ_ERROR_MIN_OTHERS:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains less than %1 non-alphanumeric characters")).arg((long)auxerror);
                   }
                   return QObject::tr("The password contains too few non-alphanumeric characters");
           case PWQ_ERROR_MIN_LENGTH:
                   if (auxerror) {
                           return QString(QObject::tr("The password is shorter than %1 characters")).arg((long)auxerror);
                   }
                   return QObject::tr("The password is too short");
           case PWQ_ERROR_ROTATED:
                   return QObject::tr("The password is just rotated old one");
           case PWQ_ERROR_MIN_CLASSES:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains less than %1 character classes")).arg((long)auxerror);
                   }
                   return QObject::tr("The password does not contain enough character classes");
           case PWQ_ERROR_MAX_CONSECUTIVE:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains more than %1 same characters consecutively")).arg((long)auxerror);
                   }
                   return QObject::tr("The password contains too many same characters consecutively");
           case PWQ_ERROR_MAX_CLASS_REPEAT:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains more than %1 characters of the same class consecutively")).arg((long)auxerror);
                   }
                   return QObject::tr("The password contains too many characters of the same class consecutively");
           case PWQ_ERROR_MAX_SEQUENCE:
                   if (auxerror) {
                           return QString(QObject::tr("The password contains monotonic sequence longer than %1 characters")).arg((long)auxerror);
                   }
                   return QObject::tr("The password contains too long of a monotonic character sequence");
           case PWQ_ERROR_EMPTY_PASSWORD:
                   return QObject::tr("No password supplied");
           case PWQ_ERROR_RNG:
                   return QObject::tr("Cannot obtain random numbers from the RNG device");
           case PWQ_ERROR_GENERATION_FAILED:
                   return QObject::tr("Password generation failed - required entropy too low for settings");
           case PWQ_ERROR_CRACKLIB_CHECK:
                   if (auxerror) {
                           return QString(QObject::tr("The password fails the dictionary check")).arg((long)auxerror);
                   }
                   return QObject::tr("The password fails the dictionary check");
           case PWQ_ERROR_UNKNOWN_SETTING:
                   if (auxerror) {
                           free(auxerror);
                           return QString(QObject::tr("Unknown setting")).arg((long)auxerror);
                   }
                   return QObject::tr("Unknown setting");
           case PWQ_ERROR_INTEGER:
                   if (auxerror) {
                           free(auxerror);
                           return QString(QObject::tr("Bad integer value of setting")).arg((long)auxerror);
                   }
                   return QObject::tr("Bad integer value");
           case PWQ_ERROR_NON_INT_SETTING:
                   if (auxerror) {
                           free(auxerror);
                           return QString(QObject::tr("Setting %s is not of integer type")).arg((long)auxerror);
                   }
                   return QObject::tr("Setting is not of integer type");
           case PWQ_ERROR_NON_STR_SETTING:
                   if (auxerror) {
                           free(auxerror);
                           return QString(QObject::tr("Setting %s is not of string type")).arg((long)auxerror);
                   }
                   return QObject::tr("Setting is not of string type");
           case PWQ_ERROR_CFGFILE_OPEN:
                   return QObject::tr("Opening the configuration file failed");
           case PWQ_ERROR_CFGFILE_MALFORMED:
                   return QObject::tr("The configuration file is malformed");
           case PWQ_ERROR_FATAL_FAILURE:
                   return QObject::tr("Fatal failure");
           default:
                   return QObject::tr("Unknown error");
           }
}

}
