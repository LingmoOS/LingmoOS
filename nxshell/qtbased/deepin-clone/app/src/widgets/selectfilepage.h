// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SELECTFILEPAGE_H
#define SELECTFILEPAGE_H

#include "contentpairpage.h"
#include "selectactionpage.h"

class SelectFilePage : public ContentPairPage
{
    Q_OBJECT
public:
    explicit SelectFilePage(SelectActionPage::Mode mode, SelectActionPage::Action action, QWidget *parent = 0);

    QString source() const;
    QString target() const;

    void setSource(const QString &source);
    void setTarget(const QString &target);

signals:
    void sourceChanged();
    void targetChanged();

private:
    void onSourceChanged();
    void hideItemForFile(const QString &filePath, UtilityList *list);
    void selectItemForFile(const QString &filePath, UtilityList *list);
};

#endif // SELECTFILEPAGE_H
