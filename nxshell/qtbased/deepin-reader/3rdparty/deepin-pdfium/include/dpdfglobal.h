#ifndef DPDFGLOBAL_H
#define DPDFGLOBAL_H

#include <QtCore/qglobal.h>
#include <QMutexLocker>
#include <QDebug>
#include <QTime>
#include <QRectF>

#ifndef BUILD_DEEPDF_STATIC
#    if defined(BUILD_DEEPDF_LIB)
#      define DEEPDF_EXPORT Q_DECL_EXPORT
#    else
#      define DEEPDF_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define DEEPDF_EXPORT
#endif

class DPdfGlobal
{
public:
    DPdfGlobal();

    ~DPdfGlobal();

    static QString textCodeType(const char *text);

    typedef struct {
        QString text;
        QRectF  rect;
    } PageLine;
    /**
     * @brief PageSection 一个选区的集合
     * page包含多个section
     * section包含多个line
     * line包含一个text和一个rect
     */
    typedef QList<PageLine> PageSection;

private:
    void init();

    void destory();
};

//pdfium即使不同文档之间loadpage和renderpage也不是线程安全，需要加锁
class DPdfMutexLocker : public QMutexLocker
{
public:
    DPdfMutexLocker(const QString &tmpLog);
    ~DPdfMutexLocker();

    QString m_log;
    QTime m_time;
};

#endif // DPDFGLOBAL_H
