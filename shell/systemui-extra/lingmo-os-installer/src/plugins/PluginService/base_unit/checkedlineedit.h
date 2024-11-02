#ifndef CHECKEDLINEEDIT_H
#define CHECKEDLINEEDIT_H

#include <QString>

namespace KInstaller {

enum class CheckState
{
    OK,
    EmptyError,
    FirstCharError,
    InvalidCharError,
    TooLongError,
    TooShortError,
    NoNumberError,
    NoLowerCharError,
    NoUpperCharError,
    NoSpecialCharError,
    TooWeakError,

};

CheckState KCheckUserName(const QString &username);
CheckState KCheckHostName(const QString &hostname);
CheckState KCheckPassWD(const QString &passwd, QString& strerror);



}

#endif // CHECKEDLINEEDIT_H
