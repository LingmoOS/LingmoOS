// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageSearchThread.h"
#include "BrowserPage.h"
#include "SheetRenderer.h"

#include <QRectF>
#include <QDebug>
#include <QFile>

QMap<QChar, QChar> PageSearchThread::m_cjktokangximap;

PageSearchThread::PageSearchThread(QObject *parent) : QThread(parent)
{

}

PageSearchThread::~PageSearchThread()
{
    m_quit = true;
    this->wait();
}

void PageSearchThread::startSearch(DocSheet *sheet, QString text)
{
    stopSearch();
    m_quit = false;
    m_startIndex = 0;
    m_sheet = sheet;
    m_searchText = text;
    start();
}

void PageSearchThread::stopSearch()
{
    m_quit = true;
    this->wait();
    m_sheet = nullptr;
}

void PageSearchThread::run()
{
    if (nullptr == m_sheet)
        return;

    initCJKtoKangxi();

    bool isSearchResultNotEmpty = false; // 没有搜索结果
    int size = m_sheet->pageCount();
    QString searchTextKangxi = m_searchText;
    for (int i = 0; i < m_searchText.size(); i++) {
        if (m_cjktokangximap.contains(m_searchText.at(i))) {
            searchTextKangxi.replace(i, 1, m_cjktokangximap.value(m_searchText.at(i)));
        }
    }

    for (int index = 0; index < size; index++) {
        if (m_quit) return;

        SearchResult searchres;

        searchres.page = index + 1;

        searchres.sections = m_sheet->renderer()->search(index, m_searchText, false, false);
        if (searchTextKangxi != m_searchText) { // 存在康熙字典部首字体
            searchres.sections.append(m_sheet->renderer()->search(index, searchTextKangxi, false, false));
        }

        std::function<QString(int, QRectF)> getText = std::bind(&SheetRenderer::getText, m_sheet->renderer(), std::placeholders::_1, std::placeholders::_2);
        bool hasWord = searchres.setctionsFillText(getText);
        //
        if (hasWord) {
            if (!isSearchResultNotEmpty) {
                isSearchResultNotEmpty = true;
                // 只要搜索到结果就emit该信号
                emit sigSearchResultNotEmpty();
            }
            emit sigSearchReady(searchres);
        }
    }
}

void PageSearchThread::initCJKtoKangxi()
{
    if (!m_cjktokangximap.isEmpty()) {
        return;
    }

    QFile file(":/CJK2Kangxi.dict");
    if (!file.open(QIODevice::ReadOnly)) {
        qInfo() << "open dictFile error :" << file.error();
        return;
    }

    QByteArray content = file.readAll();
    file.close();
    QTextStream stream(&content, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        const QStringList items = stream.readLine().split(QChar(':'));
        if (items.size() == 2) {
            m_cjktokangximap.insert(QChar(items[0].toInt(nullptr, 16)), QChar(items[1].toInt(nullptr, 16)));
        }
    }
}
