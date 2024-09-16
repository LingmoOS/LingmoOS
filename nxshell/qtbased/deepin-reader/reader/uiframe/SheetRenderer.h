// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHEETRENDERER_H
#define SHEETRENDERER_H

#include <QObject>

#include "DocSheet.h"
#include "Model.h"

/**
 * @brief The SheetRenderer class
 * 用于加载文档
 */
class SheetRenderer : public QObject
{
    Q_OBJECT
public:
    explicit SheetRenderer(DocSheet *parent);

    ~SheetRenderer();

    /**
     * @brief openFileExec
     * 阻塞式打开文档
     * @param password
     */
    bool openFileExec(const QString &password);

    /**
     * @brief openFileAsync
     * 异步式打开文档，完成后会发出sigFileOpened
     * @param password 文档密码
     * @return
     */
    void openFileAsync(const QString &password);

    /**
     * @brief opened
     * 是否已打开
     * @return
     */
    bool opened();

    /**
     * @brief handleOpened
     * 处理打开
     * @param result
     * @param error
     */
    void handleOpened(deepin_reader::Document::Error error,  deepin_reader::Document *document, QList<deepin_reader::Page *> pages);

    /**
     * @brief getPageCount
     * 获取页数量
     * @return
     */
    int getPageCount();

    /**
     * @brief getImage
     * 获取图片
     * @param index 索引
     * @param width
     * @param height
     * @param slice
     * @return
     */
    QImage getImage(int index, int width, int height, const QRect &slice = QRect());

    /**
     * @brief getLinkAtPoint
     * 获取点的link
     * @param point
     * @return
     */
    deepin_reader::Link getLinkAtPoint(int index, const QPointF &point);

    /**
     * @brief getWords
     * 得到文档无旋转下的左右文字
     * @param index 对应索引
     * @return 文字列表
     */
    QList<deepin_reader::Word> getWords(int index);

    /**
     * @brief getAnnotations
     * 得到当前页的注释,需要自行删除
     * param index 对应索引
     * @return
     */
    QList<deepin_reader::Annotation *> getAnnotations(int index);

    /**
     * @brief 文档原始大小
     * param index 对应索引
     * @return
     */
    QSizeF getPageSize(int index) const;

    /**
     * @brief 获取指定范围文本
     * @param rect
     * @return
     */
    QString getText(int index, const QRectF &rect);

    /**
     * @brief 搜索对应页内文字
     * @param text
     * @param matchCase
     * @param wholeWords
     * @return
     */
    QVector<deepin_reader::PageSection> search(int index, const QString &text, bool matchCase, bool wholeWords);

    deepin_reader::Annotation *addHighlightAnnotation(int index, const QList<QRectF> &boundaries, const QString &text, const QColor &color);

    deepin_reader::Annotation *addIconAnnotation(int index, const QRectF &rect, const QString &text);

    deepin_reader::Annotation *moveIconAnnotation(int index, deepin_reader::Annotation *annot, const QRectF &rect);

    bool removeAnnotation(int index, deepin_reader::Annotation *annotation);

    bool updateAnnotation(int index, deepin_reader::Annotation *annotation, const QString &text, const QColor &color);

    /**
     * @brief inLink
     * 当前鼠标位置是否有link
     * @param pos
     * @return
     */
    bool inLink(int index, const QPointF &pos);

    /**
     * @brief hasWidgets
     * @param index
     * @return
     */
    bool hasWidgetAnnots(int index);

    /**
     * @brief outline
     * 文档中的链接
     * @return
     */
    deepin_reader::Outline outline();

    /**
     * @brief properties
     * 文档属性
     * @return
     */
    deepin_reader::Properties properties() const;

    /**
     * @brief pageLableIndex
     * 根据文档页页码得到文档页编号
     * @param pageLable 文档页页码
     * @return 文档页编号
     */
    int pageLableIndex(const QString);

    /**
     * @brief 加载页面LABLE
     */
    void loadPageLable();

    /**
     * @brief pageHasLable
     * 文档是否有文档页页码
     * @return  如果有返回true,反之返回false
     */
    bool pageHasLable();

    /**
     * @brief pageNum2Lable
     * 根据文档页编号得到文档页页码
     * @param index 文档页编号
     * @return 如果有返回文档页页码,反之返回空字符串
     */
    QString pageNum2Lable(const int);

    /**
     * @brief 保存
     * @path 保存到路径
     * @return
     */
    bool save();

    /**
     * @brief saveAs
     * 另存为
     * @param filePath
     * @return
     */
    bool saveAs(const QString &filePath);

signals:
    /**
     * @brief sigOpened
     * 打开完成
     * @param error
     */
    void sigOpened(deepin_reader::Document::Error error);

private:
    DocSheet *m_sheet = nullptr;
    deepin_reader::Document::Error m_error = deepin_reader::Document::NoError;
    bool m_pageLabelLoaded = false;             //是否已经加载page label
    QMap<QString, int> m_lable2Page;            // 文档下标页码
    deepin_reader::Document *m_document = nullptr;
    QList<deepin_reader::Page *> m_pages;
};

#endif // SHEETRENDERER_H
