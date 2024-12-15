// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFHANDER_H
#define DDFHANDER_H
#include "datahander.h"
#include "globaldefine.h"
#include "sitemdata.h"


class PageContext;
class DdfUnitProccessor;
class DRAWLIB_EXPORT DdfHander : public DataHander
{
    Q_OBJECT
public:
    enum ErrorType {
        EDdfFileMD5Error = EUserDefineError + 1, EUserCancelLoad_OldPen, EUserCancelLoad_OldBlur,
        EDdfNotFoundContextToSave, EDdfLoadNeedOneContext
    };
    explicit DdfHander(QObject *parent = nullptr);
    DdfHander(const QString &file, QObject *parent = nullptr);
    DdfHander(const QString &file, PageContext *context, QObject *parent = nullptr);
    ~DdfHander();

    QString file()const;
    void setFile(const QString &file);

    PageContext *context() const;
    void setContext(PageContext *context);

    bool load();
    bool save();

    static PageContext *load(const QString &file);
    static bool save(PageContext *context, const QString &file);

    int  ddfVersion() const;

    bool isSupportedFile(const QString &file) override;
    bool checkMd5Valid(const QString &file_path);
    bool isDdfFileDirty(const QString &file_path);

//    bool save(const QString &file,
//              const QVariant &saveObj,
//              const QList<QVariant> &params) override;

//    QVariant  load(const QString &file,
//                   const QList<QVariant> &params) override;


    static void installProcessor(DdfUnitProccessor *processor);
    static void removeProcessor(DdfUnitProccessor *processor);
    static void initProcessors();
    static void clearProcessors();

    static DdfUnitProccessor *newerProcessor();
    static DdfUnitProccessor *processor(int version);
    static DdfUnitProccessor *processor(const QString &ddfFile);

    void setSaveProcessor(DdfUnitProccessor *processor);
    DdfUnitProccessor *saveProcessor() const;

    void setLoadProcessor(DdfUnitProccessor *processor);
    DdfUnitProccessor *loadProcessor() const;

    bool checkFileBeforeSave(const QString &file) override;
    bool checkFileBeforeLoad(const QString &file) override;


    PRIVATECLASS(DdfHander)
};

#endif // DDFHANDER_H
