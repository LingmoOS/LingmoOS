#ifndef DPDFDOC_H
#define DPDFDOC_H

#include "dpdfglobal.h"

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPointF>
#include <QVariant>
#include <QScopedPointer>

class DPdfPage;
class DPdfDocHandler;
class DPdfDocPrivate;
class DEEPDF_EXPORT DPdfDoc : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DPdfDoc)

public:
    enum Status {
        NOT_LOADED = -1,
        SUCCESS = 0,
        FILE_ERROR,
        FORMAT_ERROR,
        PASSWORD_ERROR,
        HANDLER_ERROR,
        FILE_NOT_FOUND_ERROR
    };

    struct Section;
    typedef QVector< Section > Outline;
    typedef QMap<QString, QVariant> Properies;

    struct Section {
        int nIndex;
        QPointF offsetPointF;
        QString title;
        Outline children;
    };

    DPdfDoc(QString filename, QString password = QString());

    virtual ~DPdfDoc();

    /**
     * @brief 文档是否有效
     * @return
     */
    bool isValid() const;

    /**
     * @brief 是否是加密文档
     * @return
     */
    bool isEncrypted() const;

    /**
     * @brief 文档路径
     * @return
     */
    QString filePath() const;

    /**
     * @brief 文档页数
     * @return
     */
    int pageCount() const;

    /**
     * @brief 文档状态
     * @return
     */
    Status status() const;

    /**
     * @brief 创建新的page返回
     * @param i
     * @return
     */
    DPdfPage *page(int i, qreal xRes, qreal yRes);

    /**
     * @brief 目录
     * @return
     */
    Outline outline(qreal xRes, qreal yRes);

    /**
     * @brief 文档属性信息
     * Keys:
     * Version
     * Encrypted
     * Linearized
     * KeyWords
     * Title
     * Creator
     * Producer
     * @return
     */
    Properies proeries();

    /**
     * @brief 下标真实页码
     * @return
     */
    QString label(int index) const;

    /**
     * @brief 保存到当前文件
     * @return
     */
    bool save();

    /**
     * @brief 另存为到文件
     * @param 文件名
     * @return
     */
    bool saveAs(const QString &filePath);

public:
    /**
     * @brief 尝试加载文档是否成功
     * @param filename
     * @param password
     * @return
     */
    static Status tryLoadFile(const QString &filename, const QString &password = QString());

private:
    /**
     * @brief isLinearized 是否是线性化PDF文件(线性化PDF文件是PDF文件的一种特殊格式，可以在互联网上更快地查看)
     * @param fileName
     * @return
     */
    bool isLinearized(const QString &fileName);

private:
    Q_DISABLE_COPY(DPdfDoc)
    QScopedPointer<DPdfDocPrivate> d_ptr;
};

#endif // DPDFDOC_H
