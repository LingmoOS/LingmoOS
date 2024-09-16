#ifndef DPDFPAGE_H
#define DPDFPAGE_H

#include <QObject>
#include <QImage>
#include <QScopedPointer>

#include "dpdfglobal.h"

class DPdfAnnot;
class DPdfPagePrivate;
class DPdfDocHandler;
class DEEPDF_EXPORT DPdfPage : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DPdfPage)
    friend class DPdfDoc;

public:
    ~DPdfPage();

    /**
     * @brief 是否有效
     * @return
     */
    bool isValid() const;

    /**
     * @brief 当页索引
     * @return
     */
    int index() const;

    /**
     * @brief 大小 (根据目标设备dpi算出一个相同物理尺寸对应的像素大小)
     * @return
     */
    QSizeF sizeF() const;

    /**
     * @brief 按像素宽高获取原图(如果需要切片，将不会加载widget类型的注释，如一些数字签章)
     * @param width (in pixel)
     * @param height (in pixel)
     * @param rect 要取的切片,默认为全图 (in pixel)
     * @return
     */
    QImage image(int width, int height, QRect slice = QRect());

    /**
     * @brief 字符数
     * @return
     */
    int countChars();

    /**
     * @brief 根据索引获取文本范围
     * @param index
     * @param textrect (in pixel)
     * @return
     */
    bool textRect(int index, QRectF &textrect);

    /**
     * @brief 获取多个字符文本范围
     * @param index
     * @param charCount
     * @return (in pixel)
     */
    QVector<QRectF> textRects(int index, int charCount);

    /**
     * @brief 获取本页所有文字和周围空间区域
     * @param charCount 文本字符数
     * @param rects
     */
    void allTextLooseRects(int &charCount, QStringList &texts, QVector<QRectF> &rects);

    /**
     * @brief 获取本页所有文字区域
     * @param charCount 文本字符数
     * @param rects
     */
    void allTextRects(int &charCount, QStringList &texts, QVector<QRectF> &rects);

    /**
     * @brief 根据范围获取文本
     * @param rect (in pixel)
     * @return
     */
    QString text(const QRectF &rect);

    /**
     * @brief 根据索引获取文本
     * @param index
     * @param charCount
     * @return
     */
    QString text(int index, int charCount = 1);

    /**
     * @brief 添加文字注释
     * @param point 点击的位置 基于原尺寸
     * @param text 注释内容
     * @return 添加失败返回nullptr
     */
    DPdfAnnot *createTextAnnot(QPointF pos, QString text);

    /**
     * @brief 更新注释
     * @param dAnnot 给更新的注释指针
     * @param text 注释文字
     * @param point 点击的位置 传空则不更新 基于原尺寸
     * @return
     */
    bool updateTextAnnot(DPdfAnnot *dAnnot, QString text, QPointF pos = QPointF());

    /**
     * @brief 添加高亮注释
     * @param list 高亮的区域 基于原尺寸
     * @param text 注释内容
     * @param color 高亮颜色
     * @return 添加失败返回nullptr
     */
    DPdfAnnot *createHightLightAnnot(const QList<QRectF> &rects, QString text, QColor color = QColor());

    /**
     * @brief 更新高亮注释
     * @param dAnnot 给更新的注释指针
     * @param color 传空则不更新颜色
     * @param text 注释文字
     * @return
     */
    bool updateHightLightAnnot(DPdfAnnot *dAnnot,  QColor color = QColor(), QString text = "");

    /**
     * @brief 删除注释
     * @param annot 即将删除的注释指针，执行成功传入的指针会被删除
     * @return
     */
    bool removeAnnot(DPdfAnnot *dAnnot);

    /**
     * @brief 搜索
     * @param text 搜索关键字
     * @param matchCase 区分大小写
     * @param wholeWords 整个单词
     * @return
     */
    QVector<DPdfGlobal::PageSection> search(const QString &text, bool matchCase = false, bool wholeWords = false);

    /**
     * @brief 获取当前支持操作的所有注释
     * @return 注释列表，只会列出已支持的注释
     */
    QList<DPdfAnnot *> annots();

    /**
     * @brief 获取当前支持操作的所有链接
     * @return
     */
    QList<DPdfAnnot *> links();

    /**
     * @brief 获取当前widget annots
     * @return
     */
    QList<DPdfAnnot *> widgets();

    /**
     * @brief 初始化需要延时的注释,如果link中的goto
     * @param dAnnot
     * @return
     */
    bool initAnnot(DPdfAnnot *dAnnot);

signals:
    /**
     * @brief 添加注释时触发 ，在需要的时候可以重新获取annotations()
     * @param annot 新增加的annot
     */
    void annotAdded(DPdfAnnot *dAnnot);

    /**
     * @brief 注释被更新时触发 ，在需要的时候可以重新获取annotations()
     * @param annot 被更新的annot
     */
    void annotUpdated(DPdfAnnot *dAnnot);

    /**
     * @brief 注释被删除时触发 ，在需要的时候可以重新获取annotations()
     * @param annot 被移除的annot 注意这个已经是个将要被析构后的地址 只用于做匹配移除
     */
    void annotRemoved(DPdfAnnot *dAnnot);

private:
    DPdfPage(DPdfDocHandler *handler, int pageIndex, qreal xRes = 72, qreal yRes = 72);

    QScopedPointer<DPdfPagePrivate> d_ptr;
};

#endif // DPDFPAGE_H
