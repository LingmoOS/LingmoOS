#ifndef KERROR_H
#define KERROR_H

#include <QSharedDataPointer>

class QString;
class KErrorData;

class KError
{
public:
    enum ErrorCode {
        kErrorNoError = 0,
        kErrorCommon,
        kErrorSystem,
        kErrorDBus
    };

    KError();

    KError(int code, int subCode, const QString &errName, const QString &errMessage);

    KError(const KError &);

    KError &operator=(const KError &);

    ~KError();

    bool operator==(const KError &other) const;

    int getCode() const;

    void setCode(int code);

    int getSubCode() const;

    void setSubCode(int subCode);

    QString getErrorName() const;

    void setErrorName(const QString &errName);

    QString getErrorMessage() const;

    void setErrorMessage(const QString &errMessage);

    bool isValid() const;

    explicit operator bool() const;

    void reset();

private:
    QSharedDataPointer<KErrorData> m_data;
};

bool operator!(const KError &ke) noexcept;

#endif // KERROR_H
