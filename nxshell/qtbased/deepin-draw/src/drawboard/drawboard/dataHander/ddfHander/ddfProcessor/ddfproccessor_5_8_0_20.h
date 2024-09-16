// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_8_0_20_H
#define DDFPROCCESSOR_5_8_0_20_H
#include "ddfproccessor_5_8_0_16_1.h"

//添加功能md5校验，以检查ddf文件是否被修改过
class DRAWLIB_EXPORT DdfUnitProccessor_5_8_0_20: public DdfUnitProccessor_5_8_0_16_1
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_8_0_20(QObject *parent = nullptr);
    int  version() override;

    bool saveTo(const PageContextData &data,
                const QString &file,
                DdfHander *hander) override;
    PageContextData loadFrom(const QString &file,
                      DdfHander *hander) override;

protected:

    qint64 calTotalBytes(const PageContextData &data, DdfHander *hander) override;

    void saveMd5ToFile(const QString &file);

    bool checkMd5BeforeLoad(const QString &file);
    void messageFileBroken(const QString &file, DdfHander *hander);

};

#endif // DDFPROCCESSOR_5_8_0_20_H
