// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ATTRIBUTIONREGISTERMANAGER_H
#define ATTRIBUTIONREGISTERMANAGER_H
#include "iattributionregister.h"

class AttributionRegisterManager : public IAttributionRegister
{
public:
    using IAttributionRegister::IAttributionRegister;
    virtual void registe() override;
    void addRegister(IAttributionRegister *pRegister);
    virtual ~AttributionRegisterManager() override;
private:
    void addDefaultAttri();

private:
    QList<IAttributionRegister *> m_registers;
};

#endif // ATTRIBUTIONREGISTERMANAGER_H
