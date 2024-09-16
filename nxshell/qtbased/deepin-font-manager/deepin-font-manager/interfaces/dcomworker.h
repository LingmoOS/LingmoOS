// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRunnable>

class DComWorker : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit DComWorker(QObject *parent = nullptr);
    void run();
};

class GetFontListWorker  : public DComWorker
{
    Q_OBJECT
public:
    enum Type {
        ALL,
        CHINESE,
        MONOSPACE,
        AllInSquence,
        Startup,
    };
    explicit GetFontListWorker(Type type, QObject *parent = nullptr);
    void run();

private:
    void removeUserAddFonts();

private:
    Type m_type;
};

class FontManager
{
public:
    static FontManager *instance();

public:
    void getFontListInSequence();
    void getStartFontList();
    void getChineseAndMonoFont();

private:
    static FontManager *m_fontManager;
};
