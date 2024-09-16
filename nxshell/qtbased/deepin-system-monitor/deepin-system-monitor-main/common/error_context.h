// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ERROR_CONTEXT_H
#define ERROR_CONTEXT_H

#include <QSharedDataPointer>
#include <QObject>

class QString;
class ErrorContextData;

/**
* @brief ErrorContext Error context to keep track of internal error
*/
class ErrorContext : public QObject
{
    Q_OBJECT
public:
    enum ErrorCode { kErrorTypeNoError = 0, kErrorTypeCommon, kErrorTypeSystem, kErrorTypeDBus };

    /**
    * @brief Default constructor
    */
    explicit ErrorContext(QObject* parent = nullptr);
    /**
    * @brief Constructor with params
    */
    ErrorContext(int code, int subCode, const QString &errName, const QString &errMessage, QObject* parent = nullptr);
    /**
    * @brief Copy constructor
    */
    ErrorContext(const ErrorContext &);
    /**
    * @brief Copy assignment
    */
    ErrorContext &operator=(const ErrorContext &);
    ~ErrorContext();

    /**
    * @brief operator== Compare with other error context
    * @return true: equal; false: not equal
    */
    bool operator==(const ErrorContext &other) const;
    /**
    * @brief getCode Get error code
    * @return Error code
    */
    int getCode() const;
    /**
    * @brief setCode Set error code
    * @param code Error code
    */
    void setCode(int code);

    /**
    * @brief getSubCode Get sub error code
    * @return Sub error code
    */
    int getSubCode() const;
    /**
    * @brief setSubCode Set sub error code
    * @param subCode Sub error code
    */
    void setSubCode(int subCode);

    /**
    * @brief getErrorName Get error name
    * @return Error name
    */
    QString getErrorName() const;
    /**
    * @brief setErrorName Set error name
    * @param errName Error name
    */
    void setErrorName(const QString &errName);

    /**
    * @brief getErrorMessage Get error description
    * @return Error description
    */
    QString getErrorMessage() const;
    /**
    * @brief setErrorMessage Set error description
    * @param errMessage Error description
    */
    void setErrorMessage(const QString &errMessage);

    /**
    * @brief isValid Check if it's a valid error context
    * @return true: valid error context (error occured); false: invalid error context (no error)
    */
    bool isValid() const;
    /**
    * @brief operator bool Bool operator for ErrorContext class
    */
    explicit operator bool() const;

    /**
    * @brief reset Reset error context to invalid state
    */
    void reset();

private:
    QSharedDataPointer<ErrorContextData> data;
};

/**
* @brief operator! Not operator for ErrorCotext
* @param ec Error context to be checked
* @return true: valid error context (error occured); false: invalid error context (no error)
*/
bool operator!(const ErrorContext &ec) noexcept;

// 声明类型
Q_DECLARE_METATYPE(ErrorContext);

#endif  // ERROR_CONTEXT_H
