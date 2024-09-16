// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "../subthemes/scanner.h"

#include <QDebug>



class Background {

public:
    Background();
    ~Background();
    void setId(QString id);
    QString getId() const;
    bool getDeleteable();
    void setDeletable(bool deletable);
    void Delete();
    QString Thumbnail();

private:
    QString id;
    bool deletable;
};

#endif
