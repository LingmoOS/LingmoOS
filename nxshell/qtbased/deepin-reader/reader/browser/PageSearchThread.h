// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGESEARCHTHREAD_H
#define PAGESEARCHTHREAD_H

#include "Model.h"

#include <QThread>

class BrowserPage;
class DocSheet;
class PageSearchThread : public QThread
{
    Q_OBJECT
public:
    explicit PageSearchThread(QObject *parent = nullptr);

    virtual ~PageSearchThread() override;

public:
    void startSearch(DocSheet *sheet, QString text);

    void stopSearch();

signals:
    void sigSearchReady(const deepin_reader::SearchResult &res);

    /**
     * @brief sigSearchResultNotEmpty
     * 搜索结果不为空信号
     */
    void sigSearchResultNotEmpty();

protected:
    void run() override;

private:
    /**
     * @brief initCJKtoKangxi
     * 由于docx转换工具会将部分CJK字体转为康熙字典部首字体，需要在搜索时再搜索一遍这些康熙字典部首字体
     * 该函数会根据CJK与康熙字典部首匹配文件初始化m_cjktokangximap
     */
    void initCJKtoKangxi();

private:
    bool m_quit = false;
    int m_startIndex = 0;
    QString m_searchText;
    DocSheet *m_sheet = nullptr;
    static QMap<QChar, QChar> m_cjktokangximap;
};

#endif // PAGESEARCHTHREAD_H
