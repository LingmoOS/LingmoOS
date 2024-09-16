// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dpdfdoc.h"
#include "dpdfpage.h"
#include "dpdfannot.h"

#include "public/fpdfview.h"
#include "public/fpdf_text.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_doc.h"
#include "public/fpdf_edit.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdftext/cpdf_textpage.h"
#include "core/fpdfdoc/cpdf_linklist.h"
#include "fpdfsdk/cpdfsdk_helpers.h"

class DPdfPagePrivate
{
    friend class DPdfPage;
public:
    DPdfPagePrivate(DPdfDocHandler *handler, int index, qreal xRes, qreal yRes);

    ~DPdfPagePrivate();

public:
    void loadPage();

    void loadTextPage();

    /**
     * @brief 文档自身旋转
     * @return
     */
    int oriRotation();

    QSizeF sizeF() const
    {
        return QSizeF(m_width_pt * m_xRes / 72, m_height_pt * m_yRes / 72);
    }

    QRectF transPointToPixel(const QRectF &rect) const
    {
        return QRectF(rect.x() * m_xRes / 72, rect.y() * m_yRes / 72, rect.width() * m_xRes / 72, rect.height() * m_yRes / 72);
    }
    QSizeF transPointToPixel(const QSizeF &size) const
    {
        return QSizeF(size.width() * m_xRes / 72, size.height() * m_yRes / 72);
    }
    float transPointToPixelX(const float &x) const
    {
        return x * m_xRes / 72;
    }
    float transPointToPixelY(const float &y) const
    {
        return y * m_yRes / 72;
    }

    QRectF transPixelToPoint(const QRectF &rect) const
    {
        return QRectF(rect.x() * 72 / m_xRes, rect.y() * 72 / m_yRes, rect.width() * 72 / m_xRes, rect.height() * 72 / m_yRes);
    }
    QPointF transPixelToPoint(const QPointF &pos) const
    {
        return QPointF(pos.x() * 72 / m_xRes, pos.y() * 72 / m_yRes);
    }
    QSizeF transPixelToPoint(const QSizeF &size) const
    {
        return QSizeF(size.width() * 72 / m_xRes, size.height() * 72 / m_yRes);
    }
private:
    /**
     * @brief 加载注释,无需初始化，注释的坐标取值不受页自身旋转影响,goto部分link由于耗时，需要使用时调用initAnnot初始化
     * @return 加载失败说明该页存在问题
     */
    bool loadAnnots();

    /**
     * @brief 获取所有注释
     * @return
     */
    QList<DPdfAnnot *> allAnnots();

    /**
     * @brief 初始化需要延时的注释
     * @param dAnnot
     * @return
     */
    bool initAnnot(DPdfAnnot *dAnnot);

    /**
     * @brief 视图坐标转化为文档坐标
     * @param rotation 文档自身旋转
     * @param rect
     * @return
     */
    FS_RECTF transRect(const int &rotation, const QRectF &rect);

    /**
     * @brief 文档坐标转化视图坐标
     * @param rotation 文档自身旋转
     * @param rect
     * @return
     */
    QRectF transRect(const int &rotation, const FS_RECTF &rect);

private:
    FPDF_DOCUMENT m_doc = nullptr;

    int m_index = -1;

    qreal m_width_pt = 0;

    qreal m_height_pt = 0;

    qreal m_xRes = 72;

    qreal m_yRes = 72;

    FPDF_PAGE m_page = nullptr;

    FPDF_TEXTPAGE m_textPage = nullptr;

    QList<DPdfAnnot *> m_dAnnots;

    bool m_isValid = false;

    bool m_isLoadAnnots = false;
};

DPdfPagePrivate::DPdfPagePrivate(DPdfDocHandler *handler, int index, qreal xRes, qreal yRes):
    m_doc(reinterpret_cast<FPDF_DOCUMENT>(handler)), m_index(index), m_xRes(xRes), m_yRes(yRes)
{
    DPdfMutexLocker locker("DPdfPagePrivate::DPdfPagePrivate index = " + QString::number(index));

    //宽高会受自身旋转值影响 单位:point 1/72inch 高分屏上要乘以系数
    FPDF_GetPageSizeByIndex(m_doc, index, &m_width_pt, &m_height_pt);

    FPDF_PAGE page = FPDF_LoadNoParsePage(m_doc, m_index);
    m_isValid = (page != nullptr);
    FPDF_ClosePage(page);
}

DPdfPagePrivate::~DPdfPagePrivate()
{
    if (m_textPage)
        FPDFText_ClosePage(m_textPage);

    if (m_page)
        FPDF_ClosePage(m_page);

    qDeleteAll(m_dAnnots);
}

QList<DPdfAnnot *> DPdfPagePrivate::allAnnots()
{
    if (m_isLoadAnnots)
        return m_dAnnots;

    loadAnnots();

    return m_dAnnots;
}

void DPdfPagePrivate::loadPage()
{
    if (nullptr == m_page) {
        DPdfMutexLocker locker("DPdfPagePrivate::loadPage() index = " + QString::number(m_index));//即使其他文档的page在加载时，多线程调用此函数也会崩溃，非常线程不安全,此处需要加锁
        m_page = FPDF_LoadPage(m_doc, m_index);
    }
}

void DPdfPagePrivate::loadTextPage()
{
    loadPage();

    if (nullptr == m_textPage) {
        DPdfMutexLocker locker("DPdfPagePrivate::loadTextPage() index = " + QString::number(m_index));
        m_textPage = FPDFText_LoadPage(m_page);
    }
}

int DPdfPagePrivate::oriRotation()
{
    if (nullptr == m_page) {
        DPdfMutexLocker locker("DPdfPagePrivate::oriRotation() index = " + QString::number(m_index));

        FPDF_PAGE page = FPDF_LoadNoParsePage(m_doc, m_index);

        CPDF_Page *pPage = CPDFPageFromFPDFPage(page);

        int rotation = pPage->GetPageRotation();

        FPDF_ClosePage(page);

        return rotation;
    }

    return FPDFPage_GetRotation(m_page);
}

bool DPdfPagePrivate::loadAnnots()
{
    DPdfMutexLocker locker("DPdfPagePrivate::allAnnots");

    //使用临时page，不完全加载,防止刚开始消耗时间过长
    FPDF_PAGE page = m_page;

    if (page == nullptr)
        page = FPDF_LoadNoParsePage(m_doc, m_index);      //不调用ParseContent，目前观察不会导致多线程崩溃

    if (nullptr == page) {
        return false;
    }

    CPDF_Page *pPage = CPDFPageFromFPDFPage(page);

    int rotation = pPage->GetPageRotation();

    //获取当前注释
    int annotCount = FPDFPage_GetAnnotCount(page);

    for (int i = 0; i < annotCount; ++i) {
        FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, i);

        FPDF_ANNOTATION_SUBTYPE subType = FPDFAnnot_GetSubtype(annot);

        DPdfAnnot::AnnotType type = DPdfAnnot::AUnknown;

        if (FPDF_ANNOT_TEXT == subType)
            type = DPdfAnnot::AText;
        else if (FPDF_ANNOT_HIGHLIGHT == subType)
            type = DPdfAnnot::AHighlight;
        else if (FPDF_ANNOT_LINK == subType)
            type = DPdfAnnot::ALink;
        else if (FPDF_ANNOT_CIRCLE == subType)
            type = DPdfAnnot::ACIRCLE;
        else if (FPDF_ANNOT_WIDGET == subType)
            type = DPdfAnnot::AWIDGET;

        //取出的rect为基于自身旋转前，现将转成基于旋转后的 m_width_pt/m_height_pt 为受旋转影响后的宽高
        qreal actualHeight = (rotation % 2 == 0) ? m_height_pt : m_width_pt;

        if (DPdfAnnot::AText == type) {
            DPdfTextAnnot *dAnnot = new DPdfTextAnnot;

            //获取位置
            FS_RECTF rectF;
            if (FPDFAnnot_GetRect(annot, &rectF)) {
                QRectF annorectF = transRect(rotation, rectF);
                dAnnot->setRectF(transPointToPixel(annorectF));

//                FS_RECTF newrectf;
//                newrectf.left = static_cast<float>(annorectF.left());
//                newrectf.top = static_cast<float>(actualHeight - annorectF.top());
//                newrectf.right = static_cast<float>(annorectF.right());
//                newrectf.bottom = static_cast<float>(actualHeight - annorectF.bottom());
//                FPDFAnnot_SetRect(annot, &newrectf);
            }

            //获取文本
            //根据注释长度调整获取注释内容，修复获取注释内容不全
            FPDF_WCHAR *buffer = nullptr;
            FPDFAnnot_GetFullStringValue(annot, "Contents", &buffer);
            dAnnot->m_text = QString::fromUtf16(buffer);
            m_dAnnots.append(dAnnot);
            if (!buffer) {
                delete buffer;
                buffer = nullptr;
            }
        } else if (DPdfAnnot::AHighlight == type) {
            DPdfHightLightAnnot *dAnnot = new DPdfHightLightAnnot;
            //获取颜色
            unsigned int r = 0;
            unsigned int g = 0;
            unsigned int b = 0;
            unsigned int a = 255;
            if (FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &r, &g, &b, &a)) {
                dAnnot->setColor(QColor(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a)));
            }

            //获取区域
            ulong quadCount = FPDFAnnot_CountAttachmentPoints(annot);
            QList<QRectF> list;
            for (ulong i = 0; i < quadCount; ++i) {
                FS_QUADPOINTSF quad;
                if (!FPDFAnnot_GetAttachmentPoints(annot, i, &quad))
                    continue;

                QRectF rectF;
                rectF.setX(static_cast<double>(quad.x1));
                rectF.setY(actualHeight - static_cast<double>(quad.y1));
                rectF.setWidth(static_cast<double>(quad.x2 - quad.x1));
                rectF.setHeight(static_cast<double>(quad.y1 - quad.y3));

                list.append(transPointToPixel(rectF));
            }
            dAnnot->setBoundaries(list);

            //获取文本
            //根据注释长度调整获取注释内容，修复获取注释内容不全
            FPDF_WCHAR *buffer = nullptr;
            FPDFAnnot_GetFullStringValue(annot, "Contents", &buffer);
            dAnnot->m_text = QString::fromUtf16(buffer);
            m_dAnnots.append(dAnnot);
            if (!buffer) {
                delete buffer;
                buffer = nullptr;
            }
        } else if (DPdfAnnot::ALink == type) {
            DPdfLinkAnnot *dAnnot = new DPdfLinkAnnot;

            FPDF_LINK link = FPDFAnnot_GetLink(annot);

            FPDF_ACTION action = FPDFLink_GetAction(link);

            unsigned long type = FPDFAction_GetType(action);

            //获取位置
            FS_RECTF rectF;
            if (FPDFAnnot_GetRect(annot, &rectF)) {
                QRectF annorectF = transRect(rotation, rectF);

                dAnnot->setRectF(transPointToPixel(annorectF));
            }

            //获取类型
            if (PDFACTION_URI == type) {
                char uri[256] = {0};
                unsigned long lenth = FPDFAction_GetURIPath(m_doc, action, uri, 256);
                if (0 != lenth) {
                    dAnnot->setUrl(uri);
                }

                dAnnot->setLinkType(DPdfLinkAnnot::Uri);
            } else if (PDFACTION_REMOTEGOTO == type) {
                char filePath[256] = {0};
                unsigned long lenth = FPDFAction_GetFilePath(action, filePath, 256);
                if (0 != lenth) {
                    dAnnot->setFilePath(filePath);
                }

                dAnnot->setLinkType(DPdfLinkAnnot::RemoteGoTo);
            } else if (PDFACTION_GOTO == type || PDFACTION_UNSUPPORTED == type) { //跳转到文档某处
//                FPDF_DEST dest = FPDFAction_GetDest(m_doc, action);     //速度慢,暂时延后取值

//                int index = FPDFDest_GetDestPageIndex(m_doc, dest);

//                FPDF_BOOL hasX = false;
//                FPDF_BOOL hasY = false;
//                FPDF_BOOL hasZ = false;
//                FS_FLOAT x = 0;
//                FS_FLOAT y = 0;
//                FS_FLOAT z = 0;

//                bool result = FPDFDest_GetLocationInPage(dest, &hasX, &hasY, &hasZ, &x, &y, &z);

//                if (result)
//                    dAnnot->setPage(index, transPointToPixelX(hasX ? x : 0), transPointToPixelY(hasY ? y : 0));

                dAnnot->setLinkType(DPdfLinkAnnot::Goto);
            }

            m_dAnnots.append(dAnnot);
        } else if (DPdfAnnot::ACIRCLE == type) {

            DPdfCIRCLEAnnot *dAnnot = new DPdfCIRCLEAnnot;

            //获取位置
            FS_RECTF rectF;
            if (FPDFAnnot_GetRect(annot, &rectF)) {
                QRectF annorectF = transRect(rotation, rectF);
                dAnnot->setRectF(transPointToPixel(annorectF));

//                FS_RECTF newrectf;
//                newrectf.left = static_cast<float>(annorectF.left());
//                newrectf.top = static_cast<float>(actualHeight - annorectF.top());
//                newrectf.right = static_cast<float>(annorectF.right());
//                newrectf.bottom = static_cast<float>(actualHeight - annorectF.bottom());
//                FPDFAnnot_SetRect(annot, &newrectf);
            }

            //获取区域
            ulong quadCount = FPDFAnnot_CountAttachmentPoints(annot);
            QList<QRectF> list;
            for (ulong i = 0; i < quadCount; ++i) {
                FS_QUADPOINTSF quad;
                if (!FPDFAnnot_GetAttachmentPoints(annot, i, &quad))
                    continue;

                QRectF rectF;
                rectF.setX(static_cast<double>(quad.x1));
                rectF.setY(actualHeight - static_cast<double>(quad.y1));
                rectF.setWidth(static_cast<double>(quad.x2 - quad.x1));
                rectF.setHeight(static_cast<double>(quad.y1 - quad.y3));

                list.append(transPointToPixel(rectF));
            }
            dAnnot->setBoundaries(list);

            //获取文本
            //根据注释长度调整获取注释内容，修复获取注释内容不全
            FPDF_WCHAR *buffer = nullptr;
            FPDFAnnot_GetFullStringValue(annot, "Contents", &buffer);
            dAnnot->m_text = QString::fromUtf16(buffer);
            m_dAnnots.append(dAnnot);
            if (!buffer) {
                delete buffer;
                buffer = nullptr;
            }
        } else if (DPdfAnnot::AWIDGET == type) {
            //has WIDGET annot
            DPdfWidgetAnnot *dAnnot = new DPdfWidgetAnnot;

            m_dAnnots.append(dAnnot);

        } else {
            //其他类型 用于占位 对应索引
            DPdfUnknownAnnot *dAnnot = new DPdfUnknownAnnot;

            m_dAnnots.append(dAnnot);
        }

        FPDFPage_CloseAnnot(annot);
    }

    if (m_page == nullptr)
        FPDF_ClosePage(page);

    m_isLoadAnnots = true;

    return true;
}

bool DPdfPagePrivate::initAnnot(DPdfAnnot *dAnnot)
{
    if (DPdfAnnot::ALink != dAnnot->type())
        return true;

    DPdfLinkAnnot *linkAnnot = reinterpret_cast<DPdfLinkAnnot *>(dAnnot);

    //使用临时page，不完全加载,防止刚开始消耗时间过长
    FPDF_PAGE page = m_page;

    DPdfMutexLocker locker("DPdfPagePrivate::initAnnot index = " + QString::number(m_index));

    if (page == nullptr)
        page = FPDF_LoadNoParsePage(m_doc, m_index);      //不调用ParseContent，目前观察不会导致多线程崩溃

    if (nullptr == page) {
        return false;
    }

    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, allAnnots().indexOf(dAnnot));

    FPDF_LINK link = FPDFAnnot_GetLink(annot);

    FPDFPage_CloseAnnot(annot);

    FPDF_ACTION action = FPDFLink_GetAction(link);

    FPDF_DEST dest = FPDFAction_GetDest(m_doc, action);     //速度慢

    int index = FPDFDest_GetDestPageIndex(m_doc, dest);

    FPDF_BOOL hasX = false;
    FPDF_BOOL hasY = false;
    FPDF_BOOL hasZ = false;
    FS_FLOAT x = 0;
    FS_FLOAT y = 0;
    FS_FLOAT z = 0;

    bool result = FPDFDest_GetLocationInPage(dest, &hasX, &hasY, &hasZ, &x, &y, &z);

    if (result)
        linkAnnot->setPage(index, transPointToPixelX(hasX ? x : 0), transPointToPixelY(hasY ? y : 0));

    return result;
}

FS_RECTF DPdfPagePrivate::transRect(const int &rotation, const QRectF &rect)
{
    qreal actualWidth  = (rotation % 2 == 0) ? m_width_pt : m_height_pt;
    qreal actualHeight = (rotation % 2 == 0) ? m_height_pt : m_width_pt;

    FS_RECTF fs_rect;

    if (1 == rotation) {
        fs_rect.left = static_cast<float>(rect.y());
        fs_rect.top = static_cast<float>(rect.x() + rect.width());
        fs_rect.right = static_cast<float>(rect.y() + rect.height());
        fs_rect.bottom = static_cast<float>(rect.x());
    } else if (2 == rotation) {
        fs_rect.left = static_cast<float>(actualWidth - rect.x() - rect.width());
        fs_rect.top = static_cast<float>(rect.y() + rect.height());
        fs_rect.right = static_cast<float>(actualWidth - rect.x());
        fs_rect.bottom = static_cast<float>(rect.y());
    } else if (3 == rotation) {
        fs_rect.left = static_cast<float>(actualHeight - rect.y() - rect.height());
        fs_rect.top = static_cast<float>(actualWidth - rect.x());
        fs_rect.right = static_cast<float>(actualHeight - rect.y());
        fs_rect.bottom = static_cast<float>(actualWidth - rect.x() - rect.width());
    } else {
        fs_rect.left = static_cast<float>(rect.x());
        fs_rect.top = static_cast<float>(actualHeight - rect.y());
        fs_rect.right = static_cast<float>(rect.x() + rect.width());
        fs_rect.bottom = static_cast<float>(actualHeight - rect.y() - rect.height());
    }

    return fs_rect;
}

QRectF DPdfPagePrivate::transRect(const int &rotation, const FS_RECTF &fs_rect)
{
//    qreal actualwidth  = (rotation % 2 == 0) ? m_width_pt : m_height_pt;
//    qreal actualHeight = (rotation % 2 == 0) ? m_height_pt : m_width_pt;

    //    rotation:0
    //                 actualWidth
    //    |----------------------------------|
    //    |--------right-------              |
    //    |                                  |
    //    |----left----........              |
    //    |            |      .          actualHeight
    //    |            |. . . .              |
    //    |           top     |bottom        |
    //    |            |      |              |
    //   圆心---------------------------------|

    if (1 == rotation) {    //90
        return QRectF(static_cast<qreal>(fs_rect.bottom),
                      static_cast<qreal>(fs_rect.left),
                      static_cast<qreal>(fs_rect.top) - static_cast<qreal>(fs_rect.bottom),
                      static_cast<qreal>(fs_rect.right) - static_cast<qreal>(fs_rect.left));
    } else if (2 == rotation) { //180
        return QRectF(m_width_pt - static_cast<qreal>(fs_rect.right),
                      static_cast<qreal>(fs_rect.bottom),
                      static_cast<qreal>(fs_rect.right) - static_cast<qreal>(fs_rect.left),
                      static_cast<qreal>(fs_rect.top) - static_cast<qreal>(fs_rect.bottom));

    } else if (3 == rotation) { //270
        return QRectF(m_height_pt - static_cast<qreal>(fs_rect.top),
                      m_width_pt - static_cast<qreal>(fs_rect.right),
                      static_cast<qreal>(fs_rect.top) - static_cast<qreal>(fs_rect.bottom),
                      static_cast<qreal>(fs_rect.right) - static_cast<qreal>(fs_rect.left));
    }

    return QRectF(static_cast<qreal>(fs_rect.left),
                  m_height_pt - static_cast<qreal>(fs_rect.top),
                  static_cast<qreal>(fs_rect.right) - static_cast<qreal>(fs_rect.left),
                  static_cast<qreal>(fs_rect.top) - static_cast<qreal>(fs_rect.bottom));
}

DPdfPage::DPdfPage(DPdfDocHandler *handler, int pageIndex, qreal xRes, qreal yRes)
    : d_ptr(new DPdfPagePrivate(handler, pageIndex, xRes, yRes))
{

}

DPdfPage::~DPdfPage()
{

}

bool DPdfPage::isValid() const
{
    return d_func()->m_isValid;
}

QSizeF DPdfPage::sizeF() const
{
    return d_func()->sizeF();
}

int DPdfPage::index() const
{
    return d_func()->m_index;
}

QImage DPdfPage::image(int width, int height, QRect slice)
{
    if (nullptr == d_func()->m_doc)
        return QImage();

    if (!slice.isValid())
        slice = QRect(0, 0, width, height);

    QImage image(slice.width(), slice.height(), QImage::Format_ARGB32);

    if (image.isNull())
        return QImage();

    image.fill(0xFFFFFFFF);

    DPdfMutexLocker locker("DPdfPage::image index = " + QString::number(index()));

    FPDF_PAGE page = FPDF_LoadPage(d_func()->m_doc, d_func()->m_index);

    if (nullptr == page)
        return QImage();

    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(image.width(), image.height(), FPDFBitmap_BGRA, image.scanLine(0), image.bytesPerLine());

    if (nullptr != bitmap) {
        FPDF_RenderPageBitmap(bitmap, page, slice.x(), slice.y(), slice.width(), slice.height(), width, height, 0, FPDF_ANNOT);

        if (slice.width() == width && slice.height() == height) {
            FPDF_FORMFILLINFO info;

            info.version = 1;

            FPDF_FORMHANDLE firmHandle = FPDFDOC_InitFormFillEnvironment(d_func()->m_doc, &info);

            FPDF_FFLDraw(firmHandle, bitmap, page, 0, 0,  width, height, 0, FPDF_ANNOT);
        }

        FPDFBitmap_Destroy(bitmap);
    }

    FPDF_ClosePage(page);

    locker.unlock();

    //bgr转rgb 如果image设置成Format_RGB888+FPDFBitmap_BGR则需要进行以下转换,此方法移除Alpha,节省25%的内存.
    //如果使用Format_RGB32+FPDFBitmap_BGRA 此方法无需以下转换，可以提升部分效率，但是增加内存
//    for (int i = 0; i < image.height(); i++) {
//        uchar *pixels = image.scanLine(i);
//        for (int j = 0; j < image.width(); j++) {
//            qSwap(pixels[0], pixels[2]);
//            pixels += 3;
//        }
//    }

    return image;
}

int DPdfPage::countChars()
{
    d_func()->loadTextPage();

    DPdfMutexLocker locker("DPdfPage::countChars index = " + QString::number(index()));

    return FPDFText_CountChars(d_func()->m_textPage);
}

QVector<QRectF> DPdfPage::textRects(int start, int charCount)
{
    d_func()->loadTextPage();

    QVector<QRectF> result;

    DPdfMutexLocker locker("DPdfPage::textRects index = " + QString::number(index()));

    const std::vector<CFX_FloatRect> &pdfiumRects = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetRectArraykSkipGenerated(start, charCount);

    result.reserve(static_cast<int>(pdfiumRects.size()));

    for (const CFX_FloatRect &rect : pdfiumRects) {
        result.push_back(d_func()->transPointToPixel(QRectF(static_cast<qreal>(rect.left),
                                                            d_func()->m_height_pt - static_cast<qreal>(rect.top),
                                                            static_cast<qreal>(rect.right - rect.left),
                                                            static_cast<qreal>(rect.top - rect.bottom))));
    }

    return result;
}

void DPdfPage::allTextLooseRects(int &charCount, QStringList &texts, QVector<QRectF> &rects)
{
    d_func()->loadTextPage();

    DPdfMutexLocker locker("DPdfPage::allTextRects index = " + QString::number(index()));

    charCount = FPDFText_CountChars(d_func()->m_textPage);

    const std::vector<CFX_FloatRect> &pdfiumRects = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetRectArray(0, charCount);

    rects.clear();

    rects.reserve(static_cast<int>(pdfiumRects.size()));

    for (int i = 0; i < charCount; ++i) {
        FS_RECTF rect;
        if (FPDFText_GetLooseCharBox(d_func()->m_textPage, i, &rect)) {
            rects.push_back(d_func()->transPointToPixel(QRectF(static_cast<qreal>(rect.left),
                                                               d_func()->m_height_pt - static_cast<qreal>(rect.top),
                                                               static_cast<qreal>(rect.right - rect.left),
                                                               static_cast<qreal>(rect.top - rect.bottom))));

            QVector<ushort> result(2);

            //此处windows上注释乱码,嗅探为windows-1252
            FPDFText_GetText(d_func()->m_textPage, i, 1, result.data());

            texts.append(QString::fromUtf16(result.data()));
        }
    }
}

void DPdfPage::allTextRects(int &charCount, QStringList &texts, QVector<QRectF> &rects)
{
    d_func()->loadTextPage();

    DPdfMutexLocker locker("DPdfPage::allTextRects index = " + QString::number(index()));

    charCount = FPDFText_CountChars(d_func()->m_textPage);

    const std::vector<CFX_FloatRect> &pdfiumRects = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetRectArray(0, charCount);

    rects.clear();

    rects.reserve(static_cast<int>(pdfiumRects.size()));

    for (int i = 0; i < charCount; ++i) {
        double left = 0;
        double right = 0;
        double bottom = 0;
        double top = 0;

        if (FPDFText_GetCharBox(d_func()->m_textPage, i, &left, &right, &bottom, &top)) {
            rects.push_back(d_func()->transPointToPixel(QRectF(static_cast<qreal>(left),
                                                               d_func()->m_height_pt - static_cast<qreal>(top),
                                                               static_cast<qreal>(right - left),
                                                               static_cast<qreal>(top - bottom))));

            QVector<ushort> result(2);

            //此处windows上注释乱码,嗅探为windows-1252
            FPDFText_GetText(d_func()->m_textPage, i, 1, result.data());

            texts.append(QString::fromUtf16(result.data()));
        }
    }
}

bool DPdfPage::textRect(int index, QRectF &textrect)
{
    d_func()->loadTextPage();

    DPdfMutexLocker locker("DPdfPage::textRect(int index, QRectF &textrect) index = " + QString::number(this->index()));

    if (FPDFText_GetUnicode(d_func()->m_textPage, index) == L' ') {
        textrect = QRectF();
        return true;
    }

    FS_RECTF rect;
    if (FPDFText_GetLooseCharBox(d_func()->m_textPage, index, &rect)) {
        textrect = d_func()->transPointToPixel(QRectF(static_cast<qreal>(rect.left),
                                                      d_func()->m_height_pt - static_cast<qreal>(rect.top),
                                                      static_cast<qreal>(rect.right - rect.left),
                                                      static_cast<qreal>(rect.top - rect.bottom)));
        return true;
    }

    return  false;
}

QString DPdfPage::text(const QRectF &rect)
{
    d_func()->loadTextPage();

    QRectF pointRect = d_func()->transPixelToPoint(rect);

    qreal newBottom = d_func()->m_height_pt - pointRect.bottom();

    qreal newTop = d_func()->m_height_pt - pointRect.top();

    CFX_FloatRect fxRect(static_cast<float>(pointRect.left()), static_cast<float>(std::min(newBottom, newTop)),
                         static_cast<float>(pointRect.right()), static_cast<float>(std::max(newBottom, newTop)));

    DPdfMutexLocker locker("DPdfPage::text(const QRectF &rect) index = " + QString::number(this->index()));

    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetTextByRect(fxRect);

    return QString::fromWCharArray(text.c_str(), static_cast<int>(text.GetLength()));
}

QString DPdfPage::text(int index, int charCount)
{
    d_func()->loadTextPage();

    DPdfMutexLocker locker("DPdfPage::text(int index, int charCount) index = " + QString::number(this->index()));

    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetPageText(index, charCount);

    return QString::fromWCharArray(text.c_str(), static_cast<int>(text.GetLength()));
}

DPdfAnnot *DPdfPage::createTextAnnot(QPointF pos, QString text)
{
    d_func()->loadPage();

    QPointF pointPos = d_func()->transPixelToPoint(pos);

    FPDF_ANNOTATION_SUBTYPE subType = FPDF_ANNOT_TEXT;

    DPdfMutexLocker locker("DPdfPage::createTextAnnot(QPointF pos, QString text) index = " + QString::number(this->index()));

    FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(d_func()->m_page, subType);

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    FS_RECTF fs_rect = d_func()->transRect(d_func()->oriRotation(), QRectF(pointPos.x() - 10, pointPos.y() - 10, 20, 20));

    if (!FPDFAnnot_SetRect(annot, &fs_rect)) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    FPDFPage_CloseAnnot(annot);

    locker.unlock();

    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;

    //此处使用pixel坐标
    dAnnot->setRectF(d_func()->transPointToPixel(QRectF(pointPos.x() - 10, pointPos.y() - 10, 20, 20)));

    dAnnot->setText(text);

    const QList<DPdfAnnot *> &dAnnots = d_func()->allAnnots(); //only Load
    Q_UNUSED(dAnnots);
    d_func()->m_dAnnots.append(dAnnot);

    emit annotAdded(dAnnot);

    return dAnnot;
}

bool DPdfPage::updateTextAnnot(DPdfAnnot *dAnnot, QString text, QPointF pos)
{
    d_func()->loadPage();

    DPdfTextAnnot *textAnnot = static_cast<DPdfTextAnnot *>(dAnnot);

    if (nullptr == textAnnot)
        return false;

    int index = d_func()->allAnnots().indexOf(dAnnot);

    DPdfMutexLocker locker("DPdfPage::updateTextAnnot index = " + QString::number(this->index()));

    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(d_func()->m_page, index);

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }

    textAnnot->setText(text);

    if (!pos.isNull()) {
        QPointF pointPos = d_func()->transPixelToPoint(pos);
        QSizeF pointSize = QSizeF(20, 20);

        FS_RECTF fs_rect = d_func()->transRect(d_func()->oriRotation(), QRectF(pointPos.x() - pointSize.width() / 2, pointPos.y() - pointSize.height() / 2, pointSize.width(), pointSize.height()));

        if (!FPDFAnnot_SetRect(annot, &fs_rect)) {
            FPDFPage_CloseAnnot(annot);
            return false;
        }

        //此处使用pixel坐标
        QSizeF pixelSize = d_func()->transPointToPixel(pointSize);
        textAnnot->setRectF(QRectF(pos.x() - pixelSize.width() / 2, pos.y() - pixelSize.height() / 2, pixelSize.width(), pixelSize.height()));
    }

    FPDFPage_CloseAnnot(annot);

    emit annotUpdated(dAnnot);

    return true;
}

DPdfAnnot *DPdfPage::createHightLightAnnot(const QList<QRectF> &rects, QString text, QColor color)
{
    d_func()->loadPage();

    FPDF_ANNOTATION_SUBTYPE subType = FPDF_ANNOT_HIGHLIGHT;

    DPdfMutexLocker locker("DPdfPage::createHightLightAnnot index = " + QString::number(this->index()));

    FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(d_func()->m_page, subType);

    if (color.isValid() && !FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color,
                                               static_cast<unsigned int>(color.red()),
                                               static_cast<unsigned int>(color.green()),
                                               static_cast<unsigned int>(color.blue()),
                                               static_cast<unsigned int>(color.alpha()))) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    for (const QRectF &rect : rects) {
        QRectF pointRect = d_func()->transPixelToPoint(rect);

        FS_QUADPOINTSF quad;
        quad.x1 = static_cast<float>(pointRect.x());
        quad.y1 = static_cast<float>(d_func()->m_height_pt - pointRect.y());
        quad.x2 = static_cast<float>(pointRect.x() + pointRect.width());
        quad.y2 = static_cast<float>(d_func()->m_height_pt - pointRect.y());
        quad.x3 = static_cast<float>(pointRect.x());
        quad.y3 = static_cast<float>(d_func()->m_height_pt - pointRect.y() - pointRect.height());
        quad.x4 = static_cast<float>(pointRect.x() + pointRect.width());
        quad.y4 = static_cast<float>(d_func()->m_height_pt - pointRect.y() - pointRect.height());

        if (!FPDFAnnot_AppendAttachmentPoints(annot, &quad))
            continue;
    }

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    FPDFPage_CloseAnnot(annot);

    DPdfHightLightAnnot *dAnnot = new DPdfHightLightAnnot;

    dAnnot->setBoundaries(rects);

    dAnnot->setColor(color);

    dAnnot->setText(text);

    const QList<DPdfAnnot *> &dAnnots = d_func()->allAnnots(); //only Load

    Q_UNUSED(dAnnots);

    d_func()->m_dAnnots.append(dAnnot);

    emit annotAdded(dAnnot);

    return dAnnot;
}

bool DPdfPage::updateHightLightAnnot(DPdfAnnot *dAnnot, QColor color, QString text)
{
    d_func()->loadPage();

    DPdfHightLightAnnot *hightLightAnnot = static_cast<DPdfHightLightAnnot *>(dAnnot);

    if (nullptr == hightLightAnnot)
        return false;

    int index = d_func()->allAnnots().indexOf(dAnnot);

    DPdfMutexLocker locker("DPdfPage::updateHightLightAnnot index = " + QString::number(this->index()));

    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(d_func()->m_page, index);

    if (color.isValid()) {
        if (!FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color,
                                static_cast<unsigned int>(color.red()),
                                static_cast<unsigned int>(color.green()),
                                static_cast<unsigned int>(color.blue()),
                                static_cast<unsigned int>(color.alpha()))) {
            FPDFPage_CloseAnnot(annot);
            return false;
        }
        hightLightAnnot->setColor(color);
    }

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }
    hightLightAnnot->setText(text);

    FPDFPage_CloseAnnot(annot);

    emit annotUpdated(dAnnot);

    return true;
}

bool DPdfPage::removeAnnot(DPdfAnnot *dAnnot)
{
    d_func()->loadPage();

    int index = d_func()->allAnnots().indexOf(dAnnot);

    if (index < 0)
        return false;

    DPdfMutexLocker locker("DPdfPage::removeAnnot index = " + QString::number(this->index()));

    if (!FPDFPage_RemoveAnnot(d_func()->m_page, index))
        return false;

    const QList<DPdfAnnot *> &dAnnots = d_func()->allAnnots(); //only Load
    Q_UNUSED(dAnnots);
    d_func()->m_dAnnots.removeAll(dAnnot);

    emit annotRemoved(dAnnot);

    delete dAnnot;

    return true;
}

QVector<DPdfGlobal::PageSection> DPdfPage::search(const QString &text, bool matchCase, bool wholeWords)
{
    d_func()->loadTextPage();

    DPdfMutexLocker locker("DPdfPage::search index = " + QString::number(this->index()));

    QVector<DPdfGlobal::PageSection> sections;

    unsigned long flags = 0x00000000;

    if (matchCase)
        flags |= FPDF_MATCHCASE;

    if (wholeWords)
        flags |= FPDF_MATCHWHOLEWORD;

    FPDF_SCHHANDLE schandle = FPDFText_FindStart(d_func()->m_textPage, text.utf16(), flags, 0);
    if (schandle) {
        int page = d_func()->m_index;
        FPDF_PAGE pdfPage = FPDF_LoadPage(d_func()->m_doc, page);
        double pageHeight = FPDF_GetPageHeight(pdfPage);
        FPDF_TEXTPAGE textPage = d_func()->m_textPage;
        while (FPDFText_FindNext(schandle)) {
            FPDF_SCHHANDLE sh = schandle;
            QVector<QRectF> region;//一个section对应的region
            int idx = FPDFText_GetSchResultIndex(sh);
            if(idx < 0)
                continue;
            int count = FPDFText_GetSchCount(sh);
            int rectCount = FPDFText_CountRects(textPage, idx, count);
            for (int r = 0; r < rectCount; ++r) {
                double left, top, right, bottom;
                FPDFText_GetRect(textPage, r, &left, &top, &right, &bottom);
                QRectF rect = d_func()->transPointToPixel(
                            QRectF(left, pageHeight - top, right - left, top - bottom));

                //一次查找会有多个rect，若这些rect在同一行需要做合并处理，满足显示效果
                if(region.count() > 0 && region.last().x() < rect.x()) {
                    region.last() = region.last().united(rect);
                    continue;
                }
                region << rect;
            }

            //添加一个section信息
            DPdfGlobal::PageSection section;
            for(auto r : region) {
                section.append(DPdfGlobal::PageLine{QString(), r});
            }
            sections.append(section);
        }
    }

    FPDFText_FindClose(schandle);
    return sections;
}

QList<DPdfAnnot *> DPdfPage::annots()
{
    QList<DPdfAnnot *> dannots;

    const QList<DPdfAnnot *> &dAnnots = d_func()->allAnnots();

    foreach (DPdfAnnot *dannot, dAnnots) {
        if (dannot->type() == DPdfAnnot::AText || dannot->type() == DPdfAnnot::AHighlight) {
            dannots.append(dannot);
            continue;
        }
    }

    return dannots;
}

QList<DPdfAnnot *> DPdfPage::links()
{
    QList<DPdfAnnot *> links;

    const QList<DPdfAnnot *> &dAnnots = d_func()->allAnnots();
    foreach (DPdfAnnot *annot, dAnnots) {
        if (annot->type() == DPdfAnnot::ALink) {
            links.append(annot);
        }
    }

    return links;
}

QList<DPdfAnnot *> DPdfPage::widgets()
{
    QList<DPdfAnnot *> widgets;

    const QList<DPdfAnnot *> &dAnnots = d_func()->allAnnots();
    foreach (DPdfAnnot *annot, dAnnots) {
        if (annot->type() == DPdfAnnot::AWIDGET) {
            widgets.append(annot);
            continue;
        }
    }

    return widgets;
}

bool DPdfPage::initAnnot(DPdfAnnot *dAnnot)
{
    return d_func()->initAnnot(dAnnot);
}
