// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include "Global.h"
#include "dpdfpage.h"

#include <QList>
#include <QtPlugin>
#include <QRect>
#include <QStandardItemModel>
#include <QString>
#include <QWidget>
#include <QDateTime>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QApplication>
#include <QScreen>

class QColor;
class QImage;
class QSizeF;
class DPdfAnnot;
class QProcess;

namespace deepin_reader {

const int Z_ORDER_HIGHLIGHT   = 2;
const int Z_ORDER_SELECT_TEXT = 3;
const int Z_ORDER_ICON        = 4;

struct Link {
    QPainterPath boundary;
    int page = -1;
    qreal left = 0;
    qreal top = 0;
    QString urlOrFileName;
    Link() : boundary(), page(-1), left(0.0), top(0.0), urlOrFileName() {}
    Link(const QPainterPath &boundary, int page, qreal left = 0.0, qreal top = 0.0) : boundary(boundary), page(page), left(left), top(top), urlOrFileName() {}
    Link(const QRectF &boundingRect, int page, qreal left = 0.0, qreal top = 0.0) : boundary(), page(page), left(left), top(top), urlOrFileName() { boundary.addRect(boundingRect); }
    Link(const QPainterPath &boundary, const QString &url) : boundary(boundary), page(-1), left(0.0), top(0.0), urlOrFileName(url) {}
    //Link(const QRectF &boundingRect, const QString &url) : boundary(), page(-1), left(0.0), top(0.0), urlOrFileName(url) { boundary.addRect(boundingRect); }
    //Link(const QPainterPath &boundary, const QString &fileName, int page) : boundary(boundary), page(page), left(0.0), top(0.0), urlOrFileName(fileName) {}
    //Link(const QRectF &boundingRect, const QString &fileName, int page) : boundary(), page(page), left(0.0), top(0.0), urlOrFileName(fileName) { boundary.addRect(boundingRect); }

    bool isValid() const
    {
        return page >= 1 || !urlOrFileName.isEmpty();
    }
};

struct Section;

typedef QVector< Section > Outline;

typedef QMap<QString, QVariant> Properties;

typedef DPdfGlobal::PageSection PageSection;
typedef DPdfGlobal::PageLine PageLine;
struct Section {
    int nIndex = -1;
    QPointF offsetPointF;
    QString title;
    Outline children;
};

struct Word {
    QString text;

    QRectF boundingBox;

    QRectF wordBoundingRect() const
    {
        return boundingBox;
    }

    Word()
    {

    }

    Word(const QString &wordText, const QRectF &rectf): text(wordText), boundingBox(rectf)
    {

    }
};

struct SearchResult {
    int page = 0;
    QVector<PageSection> sections;

    /**
     * @brief setctionsFillText 使用getText，填充sections的text
     */
    bool setctionsFillText(std::function<QString(int, QRectF)> getText);

    /**
     * @brief sectionBoundingRect 返回section的boundingRect
     */
    static QRectF sectionBoundingRect(const PageSection &section);
};

struct FileInfo {
    QString filePath;
    QString theme;
    QString auther;
    QString keyword;
    QString producter;
    QString creater;
    QDateTime createTime;
    QDateTime changeTime;
    QString format;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int numpages = 0;
    bool     optimization = false;
    bool     safe  = false;
    float    size = 0;
};

class Annotation : public QObject
{
    Q_OBJECT
public:
    Annotation() : QObject() {}

    virtual ~Annotation() {}

    virtual QList<QRectF> boundary() const = 0;

    virtual QString contents() const = 0;

    virtual DPdfAnnot *ownAnnotation() = 0;

    //数值同dpdf类型
    enum AnnotationType {
        AUnknown = 0,         ///< 前期支持以外的
        AText = 1,            ///< TextAnnotation
        AHighlight = 2,       ///< HighlightAnnotation
        ALink = 3,
        AUNDERLINE = 4,
        ASQUARE = 5,
        ACIRCLE = 6,
        AWIDGET = 7
    };

    virtual int type() = 0;

    int page = 0;

signals:
    void wasModified();
};
typedef QList<Annotation *> AnnotationList;

class FormField : public QObject
{
    Q_OBJECT
public:
    FormField() : QObject() {}
    virtual ~FormField() {}
    virtual QRectF boundary() const = 0;
    virtual QString name() const = 0;
    virtual QWidget *createWidget() = 0;
signals:
    void wasModified();
};

class Page: public QObject
{
    Q_OBJECT
public:
    Page() : QObject()
    {

    }
    virtual ~Page() {}

    virtual QSizeF sizeF() const = 0;
    virtual QImage render(int width, int height, const QRect &slice = QRect()) const = 0;
    virtual Link getLinkAtPoint(const QPointF &) { return Link(); }
    virtual QString text(const QRectF &rect) const = 0;
    virtual QString cachedText(const QRectF &rect) const { return text(rect); }
    virtual QVector<PageSection> search(const QString &text, bool matchCase, bool wholeWords) const = 0;
    virtual QList< Annotation * > annotations() const { return QList< Annotation * >(); }
    virtual bool canAddAndRemoveAnnotations() const { return false; }
    virtual bool hasWidgetAnnots() const { return false; }
    virtual Annotation *addHighlightAnnotation(const QList<QRectF> &boundaries, const QString &text, const QColor &color) { Q_UNUSED(boundaries) Q_UNUSED(text) Q_UNUSED(color) return nullptr; }
    virtual bool removeAnnotation(Annotation *annotation) { Q_UNUSED(annotation) return  false;}
    virtual QList< FormField * > formFields() const { return QList< FormField * >(); }
    virtual QList<Word> words() {return QList<Word>();}
    virtual bool updateAnnotation(Annotation *, const QString &, const QColor &) {return false;}
    virtual Annotation *addIconAnnotation(const QRectF &ponit, const QString &text) { Q_UNUSED(ponit) Q_UNUSED(text) return nullptr; }
    virtual Annotation *moveIconAnnotation(Annotation *annot, const QRectF &rect) { Q_UNUSED(annot) Q_UNUSED(rect) return nullptr; }
};

class Document: public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError = 0,
        NeedPassword,       //需要密码
        WrongPassword,      //密码错误
        FileError,          //打开失败
        FileDamaged,        //打开成功 文件损坏
        ConvertFailed       //转换失败
    };

    Document() : QObject() {}
    virtual ~Document() {}
    virtual int pageCount() const = 0;
    virtual Page *page(int index) const = 0;
    virtual QStringList saveFilter() const = 0;
    virtual QString label(int) const { return QString(); }
    virtual bool save() const = 0;
    virtual bool saveAs(const QString &filePath) const = 0;
    virtual Outline outline() const { return Outline(); }
    virtual Properties properties() const = 0;
};

class DocumentFactory
{
public:
    static Document *getDocument(const int &fileType, const QString &filePath,
                                 const QString &convertedFileDir, const QString &password,
                                 QProcess **pprocess,
                                 deepin_reader::Document::Error &error);
};


} // deepin_reader


#endif // DOCUMENTMODEL_H
