// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SheetRenderer.h"

#include "PageRenderThread.h"

#include <QDebug>

SheetRenderer::SheetRenderer(DocSheet *parent) : QObject(parent), m_sheet(parent)
{

}

SheetRenderer::~SheetRenderer()
{
    qDebug() << "正在添加关闭文档任务...";
    DocCloseTask task;

    task.document = m_document;

    task.pages = m_pages;

    PageRenderThread::appendTask(task);
    qDebug() << "关闭文档任务已添加";
}

bool SheetRenderer::openFileExec(const QString &password)
{
    QEventLoop loop;

    connect(this, &SheetRenderer::sigOpened, &loop, &QEventLoop::quit);

    openFileAsync(password);

    loop.exec();

    return deepin_reader::Document::NoError == m_error;
}

void SheetRenderer::openFileAsync(const QString &password)
{
    DocOpenTask task;

    task.sheet = m_sheet;

    task.password = password;

    task.renderer = this;

    PageRenderThread::appendTask(task);
}

bool SheetRenderer::opened()
{
    return m_document != nullptr;
}

int SheetRenderer::getPageCount()
{
    return m_pages.count();
}

QImage SheetRenderer::getImage(int index, int width, int height, const QRect &slice)
{
    if (m_pages.count() <= index)
        return QImage();

    QImage image = m_pages.value(index)->render(width, height, slice);

    return image;
}

deepin_reader::Link SheetRenderer::getLinkAtPoint(int index, const QPointF &point)
{
    if (m_pages.count() <= index)
        return deepin_reader::Link();

    return  m_pages.value(index)->getLinkAtPoint(point);
}

QList<deepin_reader::Word> SheetRenderer::getWords(int index)
{
    if (m_pages.count() <= index)
        return QList<deepin_reader::Word>();

    return m_pages.value(index)->words();
}

QList<deepin_reader::Annotation *> SheetRenderer::getAnnotations(int index)
{
    if (m_pages.count() <= index)
        return QList<deepin_reader::Annotation *>();

    return m_pages.value(index)->annotations();
}

QSizeF SheetRenderer::getPageSize(int index) const
{
    if (m_pages.count() <= index)
        return QSizeF();

    return m_pages.value(index)->sizeF();
}

deepin_reader::Annotation *SheetRenderer::addHighlightAnnotation(int index, const QList<QRectF> &boundaries, const QString &text, const QColor &color)
{
    if (m_pages.count() <= index)
        return nullptr;

    return m_pages.value(index)->addHighlightAnnotation(boundaries, text, color);
}

bool SheetRenderer::removeAnnotation(int index, deepin_reader::Annotation *annotation)
{
    if (m_pages.count() <= index)
        return false;

    return m_pages.value(index)->removeAnnotation(annotation);
}

bool SheetRenderer::updateAnnotation(int index, deepin_reader::Annotation *annotation, const QString &text, const QColor &color)
{
    if (m_pages.count() <= index)
        return false;

    return m_pages.value(index)->updateAnnotation(annotation, text, color);
}

deepin_reader::Annotation *SheetRenderer::addIconAnnotation(int index, const QRectF &rect, const QString &text)
{
    if (m_pages.count() <= index)
        return nullptr;

    return m_pages.value(index)->addIconAnnotation(rect, text);
}

deepin_reader::Annotation *SheetRenderer::moveIconAnnotation(int index, deepin_reader::Annotation *annot, const QRectF &rect)
{
    if (m_pages.count() <= index)
        return nullptr;

    return m_pages.value(index)->moveIconAnnotation(annot, rect);
}

QString SheetRenderer::getText(int index, const QRectF &rect)
{
    if (m_pages.count() <= index)
        return QString();

    return m_pages.value(index)->text(rect);
}

QVector<deepin_reader::PageSection> SheetRenderer::search(int index, const QString &text, bool matchCase, bool wholeWords)
{
    if (m_pages.count() <= index)
        return QVector<deepin_reader::PageSection>();

    return m_pages.value(index)->search(text, matchCase, wholeWords);
}

bool SheetRenderer::inLink(int index, const QPointF &pos)
{
    if (m_pages.count() <= index)
        return false;

    deepin_reader::Link link = m_pages.value(index)->getLinkAtPoint(pos);

    return link.isValid();
}

bool SheetRenderer::hasWidgetAnnots(int index)
{
    if (m_pages.count() <= index)
        return false;

    return m_pages.value(index)->hasWidgetAnnots();
}

deepin_reader::Outline SheetRenderer::outline()
{
    if (m_document == nullptr)
        return deepin_reader::Outline();

    return m_document->outline();
}

deepin_reader::Properties SheetRenderer::properties() const
{
    if (m_document == nullptr)
        return deepin_reader::Properties();

    return m_document->properties();
}

bool SheetRenderer::save()
{
    if (m_document == nullptr)
        return false;

    return m_document->save();
}

void SheetRenderer::loadPageLable()
{
    if (m_pageLabelLoaded || m_document == nullptr)
        return;

    m_pageLabelLoaded = true;

    m_lable2Page.clear();

    int pageCount = m_document->pageCount();

    for (int i = 0; i < pageCount; i++) {
        const QString &labelPage = m_document->label(i);

        if (!labelPage.isEmpty() && labelPage.toInt() != i + 1) {
            m_lable2Page.insert(labelPage, i);
        }
    }
}

int SheetRenderer::pageLableIndex(const QString pageLable)
{
    if (m_lable2Page.count() <= 0 || !m_lable2Page.contains(pageLable))
        return -1;

    return m_lable2Page.value(pageLable);
}

bool SheetRenderer::pageHasLable()
{
    loadPageLable();

    if (m_lable2Page.count() > 0) {
        return true;
    }

    return false;
}

QString SheetRenderer::pageNum2Lable(const int index)
{
    QMap<QString, int>::const_iterator iter;
    for (iter = m_lable2Page.constBegin(); iter != m_lable2Page.constEnd(); ++iter) {
        if (iter.value() == index)
            return iter.key();
    }

    return  QString::number(index + 1);
}

bool SheetRenderer::saveAs(const QString &filePath)
{
    if (filePath.isEmpty() || m_document == nullptr)
        return false;

    return m_document->saveAs(filePath);;
}

void SheetRenderer::handleOpened(deepin_reader::Document::Error error, deepin_reader::Document *document, QList<deepin_reader::Page *> pages)
{
    m_error = error;

    m_document = document;

    m_pages = pages;

    emit sigOpened(error);
}

