// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATAHANDER_H
#define DATAHANDER_H
#include "globaldefine.h"
#include <QObject>

class PageContext;
struct SMessage;
class DRAWLIB_EXPORT DataHander : public QObject
{
    Q_OBJECT
public:
    enum ErrorType {NoError = 0,

                    EFileNameIllegal = 1000, EFileNotExist,

                    //LOAD EEROR TYPE
                    EUnReadableFile, EUnSupportFile,
                    EDamagedFile, EParsingToContextFailed,
                    EMd5Valid,

                    //WRITE EEROR TYPE
                    EUnWritableFile, EUnWritableDir, EInsufficientPartitionSpace,

                    EUserDefineError = 10000
                   };
    explicit DataHander(QObject *parent = nullptr);

    int     error() const;
    QString errorString()const;
    void    unsetError();
    void    setError(int error, const QString &errorString);


    static bool isLegalFile(const QString &path);
    static QString toLegalFile(const QString &filePath);

    virtual bool isSupportedFile(const QString &file) = 0;

//    virtual bool save(const QString &file,
//                      const QVariant &saveObj,
//                      const QList<QVariant> &params) = 0;

//    virtual QVariant load(const QString &file,
//                          const QList<QVariant> &params) = 0;


signals:
    void progressBegin(const QString &describe);
    void progressChanged(int progress, int total, const QString &describe);
    void progressEnd(int ret, const QString &describe);
    void message_waitAnswer(const SMessage &message, int &retureRet);
    void message(const SMessage &message);


protected:
    virtual bool checkFileBeforeSave(const QString &file);
    virtual bool checkFileBeforeLoad(const QString &file);
    void messageUnSupport(const QString &file);
    void messageUnReadable(const QString &file);
    void messageUnWritable(const QString &file);
    void messageFileNotExist(const QString &file);
    void messageMd5valid(const QString &file);



    int     _error = 0;
    QString _errorString;
};


class DRAWLIB_EXPORT ImageHander : public DataHander
{
    Q_OBJECT
public:
    explicit ImageHander(QObject *parent = nullptr);

    QImage load(const QString &file);
    bool   save(const QImage &img, const QString &file, int qulit = 100);

    bool  isSupportedFile(const QString &file) override;

//    bool  save(const QString &file,
//               const QVariant &saveObj,
//               const QList<QVariant> &params) override;

//    QVariant load(const QString &file,
//                  const QList<QVariant> &params) override;
};

#endif // DATAHANDER_H
