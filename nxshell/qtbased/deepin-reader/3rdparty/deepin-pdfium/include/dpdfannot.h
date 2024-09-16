#ifndef DPDFANNOT_H
#define DPDFANNOT_H

#include "dpdfglobal.h"

#include <QRectF>
#include <QString>
#include <QList>
#include <QColor>

class DEEPDF_EXPORT DPdfAnnot
{
public:
    enum AnnotType {
        AUnknown = 0,         ///< 前期支持以外的
        AText = 1,            ///< TextAnnotation
        AHighlight = 2,       ///< HighlightAnnotation
        ALink = 3,
        AUNDERLINE = 4,
        ASQUARE = 5,
        ACIRCLE = 6,
        AWIDGET = 7
    };

    virtual ~DPdfAnnot();

    /**
     * @brief 是否在该点上
     * @param pos (in point)
     * @return
     */
    virtual bool pointIn(QPointF pos) = 0;

    /**
     * @brief boundaries
     * @return (in point)
     */
    virtual QList<QRectF> boundaries() = 0;

    AnnotType type();

    void setText(QString text);

    QString text();

protected:
    AnnotType m_type;
    QString m_text;
};

class DEEPDF_EXPORT DPdfTextAnnot : public DPdfAnnot
{
    friend class DPdfPage;
    friend class DPdfPagePrivate;
public:
    DPdfTextAnnot();

    bool pointIn(QPointF pos) override;

    QList<QRectF> boundaries() override;

    void setRectF(const QRectF &rectf);

private:
    QRectF m_rect;
};

class DEEPDF_EXPORT DPdfSquareAnnot : public DPdfAnnot
{
public:
    DPdfSquareAnnot();

    bool pointIn(QPointF pos) override;

    QList<QRectF> boundaries() override;

    void setRectF(const QRectF &rectf);

private:
    QRectF m_rect;
};

class DEEPDF_EXPORT DPdfCIRCLEAnnot : public DPdfAnnot
{
    friend class DPdfPage;
    friend class DPdfPagePrivate;
public:
    DPdfCIRCLEAnnot();

    bool pointIn(QPointF pos) override;

    QList<QRectF> boundaries() override;

    void setRectF(const QRectF &rectf);

    void setBoundaries(QList<QRectF> rectList);

private:
    QList<QRectF> m_rectList;
    QRectF m_rect;
};

class DEEPDF_EXPORT DPdfHightLightAnnot : public DPdfAnnot
{
    friend class DPdfPage;
    friend class DPdfPagePrivate;
public:
    DPdfHightLightAnnot();

    bool pointIn(QPointF pos) override;

    void setColor(QColor color);

    QColor color();

    void setBoundaries(QList<QRectF> rectList);

    QList<QRectF> boundaries() override;

private:
    QList<QRectF> m_rectList;
    QColor m_color;
};

class DEEPDF_EXPORT DPdfLinkAnnot : public DPdfAnnot
{
public:
    DPdfLinkAnnot();

    enum LinkType {
        Unknown = 0,
        Uri,        //URI, including web pages and other Internet resources
        RemoteGoTo, //Go to a destination within another document.
        Goto        //Go to a destination within current document.
    };

    bool pointIn(QPointF pos) override;

    QList<QRectF> boundaries() override;

    void setRectF(const QRectF &rect);

    void setUrl(QString url);

    QString url() const;

    void setFilePath(QString filePath);

    QString filePath() const;

    void setPage(int index, float left, float top);

    int pageIndex() const;

    QPointF offset() const;

    void setLinkType(int type);

    int linkType() const;

    bool isValid() const;

private:
    QRectF m_rect;
    QString m_url;
    QString m_filePath;

    int m_linkType = Unknown;
    int m_index = -1;
    float m_left = 0;
    float m_top = 0;
};

class DEEPDF_EXPORT DPdfWidgetAnnot : public DPdfAnnot
{
public:
    DPdfWidgetAnnot();

    bool pointIn(QPointF pos) override;

    QList<QRectF> boundaries() override;

};

class DEEPDF_EXPORT DPdfUnknownAnnot : public DPdfAnnot
{
public:
    DPdfUnknownAnnot();

    bool pointIn(QPointF pos) override;

    QList<QRectF> boundaries() override;

};

class DEEPDF_EXPORT DPdfUnderlineAnnot : public DPdfAnnot
{
public:
    DPdfUnderlineAnnot();

    bool pointIn(QPointF pos) override;

    QList<QRectF> boundaries() override;

    void setRectF(const QRectF &rectf);

private:
    QRectF m_rect;
};
#endif // DPDFANNOT_H
