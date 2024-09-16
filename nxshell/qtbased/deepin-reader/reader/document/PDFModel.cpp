// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PDFModel.h"
#include "dpdfannot.h"
#include "dpdfpage.h"
#include "dpdfdoc.h"
#include "Application.h"

#include <QDebug>
#include <QScreen>
#include <QThread>

#include <functional>

#define LOCK_DOCUMENT QMutexLocker docMutexLocker(m_docMutex);

namespace deepin_reader {
PDFAnnotation::PDFAnnotation(DPdfAnnot *dannotation) : Annotation(),
    m_dannotation(dannotation)
{

}

PDFAnnotation::~PDFAnnotation()
{
    m_dannotation = nullptr;
}

QList<QRectF> PDFAnnotation::boundary() const
{
    QList<QRectF> rectFList;

    if (m_dannotation->type() == DPdfAnnot::AText || m_dannotation->type() == DPdfAnnot::AHighlight) {

        foreach (QRectF rect, m_dannotation->boundaries())
            rectFList.append(rect);
    }

    return rectFList;
}

QString PDFAnnotation::contents() const
{
    if (nullptr == m_dannotation)
        return QString();

    return m_dannotation->text();
}

int PDFAnnotation::type()
{
    if (nullptr == m_dannotation)
        return -1;

    return m_dannotation->type();
}

DPdfAnnot *PDFAnnotation::ownAnnotation()
{
    return m_dannotation;
}

PDFPage::PDFPage(QMutex *mutex, DPdfPage *page) :
    m_docMutex(mutex), m_page(page)
{

}

PDFPage::~PDFPage()
{

}

QSizeF PDFPage::sizeF() const
{
    return m_page->sizeF();
}

QImage PDFPage::render(int width, int height, const QRect &slice) const
{
    LOCK_DOCUMENT

    QRect ratioRect = slice.isValid() ? QRect(slice.x(), slice.y(), slice.width(), slice.height()) : QRect();

    return m_page->image(width, height, ratioRect);
}

Link PDFPage::getLinkAtPoint(const QPointF &pos)
{
    Link link;
    const QList<DPdfAnnot *> &dlinkAnnots = m_page->links();
    if (dlinkAnnots.size() > 0) {
        for (DPdfAnnot *annot : dlinkAnnots) {
            DPdfLinkAnnot *linkAnnot = dynamic_cast<DPdfLinkAnnot *>(annot);
            if (linkAnnot && linkAnnot->pointIn(pos)) {
                if (!linkAnnot->isValid())
                    m_page->initAnnot(annot);

                link.page = linkAnnot->pageIndex() + 1;
                if (linkAnnot->url().isEmpty()) {
                    link.urlOrFileName = linkAnnot->filePath();
                } else {
                    // 删除邮件或本地文件的前缀"http://"
                    link.urlOrFileName = (linkAnnot->url().startsWith(QLatin1String("http://mailto:"))
                                          || linkAnnot->url().startsWith(QLatin1String("http://file://")))
                                         ? linkAnnot->url().mid(7) : linkAnnot->url();
                }
                link.left = linkAnnot->offset().x();
                link.top = linkAnnot->offset().y();
                return link;
            }
        }
    }
    return link;
}

bool PDFPage::hasWidgetAnnots() const
{
    return m_page->widgets().count() > 0;
}

QString PDFPage::text(const QRectF &rect) const
{
    return m_page->text(rect).simplified();
}

QList<Word> PDFPage::words()
{
    LOCK_DOCUMENT

    if (m_wordLoaded)
        return m_words;

    int charCount = 0;

    QStringList texts;

    QVector<QRectF> rects;

    m_page->allTextLooseRects(charCount, texts, rects);

    m_wordLoaded = true;

    if (charCount <= 0)
        return m_words;

    int index = 0;

    qreal maxY = rects[0].y();

    qreal maxHeight = rects[0].height();

    QList<Word> rowWords;

    while (1) {
        if (index == charCount) {
            m_words.append(rowWords);
            break;
        }

        Word word;
        word.text = texts[index];
        word.boundingBox = rects[index];

        //不进行优化
//        m_words.append(word);
//        ++index;
//        continue;

        //如果重叠的部分只在三分之一，视为换行
        if ((word.boundingBox.y() > maxY + maxHeight - (word.boundingBox.height() / 3)) ||
                (word.boundingBox.y() + word.boundingBox.height() < maxY  + (word.boundingBox.height() / 3))) {

            //最本行所有文字 补齐高和低不符合的 填满宽度
            for (int i = 0; i < rowWords.count(); ++i) {
                rowWords[i].boundingBox.setY(maxY);
                rowWords[i].boundingBox.setHeight(maxHeight);

                //如果是空格类型 且没有宽度
                if (!rowWords[i].text.isEmpty() && rowWords[i].text.trimmed().isEmpty() && rowWords[i].boundingBox.width() < 0.0001) {
                    continue;
                    //                if (i - 1 >= 0 && i + 1 < rowWords.count()) {
                    //                    if (rowWords[i + 1].boundingBox.x() - (rowWords[i - 1].boundingBox.x() + rowWords[i - 1].boundingBox.width()) < 10) {
                    //                        rowWords[i].boundingBox.setY(maxY);
                    //                        rowWords[i].boundingBox.setX(rowWords[i - 1].boundingBox.x() + rowWords[i - 1].boundingBox.width());
                    //                        rowWords[i].boundingBox.setWidth(rowWords[i + 1].boundingBox.x() - rowWords[i - 1].boundingBox.x() - rowWords[i - 1].boundingBox.width());
                    //                    }
                    //                }
                }

                //找出本行下一个非空的索引
                int nextIndex = i + 1;
                while (nextIndex < rowWords.count()) {
                    if (!rowWords[nextIndex].text.trimmed().isEmpty() || rowWords[nextIndex].boundingBox.width() > 0.0001)
                        break;
                    nextIndex++;
                }

                //将本文字宽度拉长到下一个字/如果和下一个x重合，则稍微减少点自己宽度
                if (((nextIndex) < rowWords.count()) && !rowWords[nextIndex].text.trimmed().isEmpty() &&
                        (((rowWords[i].boundingBox.x() + rowWords[i].boundingBox.width() < rowWords[nextIndex].boundingBox.x()) &&
                          (rowWords[nextIndex].boundingBox.x() - (rowWords[i].boundingBox.x() + rowWords[i].boundingBox.width()) < 10)) ||
                         ((rowWords[i].boundingBox.x() + rowWords[i].boundingBox.width() > rowWords[nextIndex].boundingBox.x())))
                   ) {
                    rowWords[i].boundingBox.setWidth(rowWords[nextIndex].boundingBox.x() - rowWords[i].boundingBox.x());
                }
            }

            m_words.append(rowWords);
            rowWords.clear();
            maxY = word.boundingBox.y();
            maxHeight = word.boundingBox.height();
        }

        //如果上一个是空格 这一个还是空 则只是追加
//        if (!rowWords.isEmpty() && rowWords[rowWords.count() - 1].text.trimmed().isEmpty() && word.text.trimmed().isEmpty()) {
//            rowWords[rowWords.count() - 1].text.append(word.text);
//            ++index;
//            continue;
//        }

        //先取出最高和最低
        if (word.boundingBox.y() < maxY)
            maxY = word.boundingBox.y();

        if (word.boundingBox.height() + (word.boundingBox.y() - maxY) > maxHeight)
            maxHeight = word.boundingBox.height() + (word.boundingBox.y() - maxY);

        rowWords.append(word);

        ++index;
    }

//    for (int i = 0; i < charCount; i++) {
//        if (texts.count() <= i || rects.count() <= i)
//            break;

//        Word word;
//        word.text = texts[i];
//        QRectF rectf = rects[i];

//        //换行
//        if (rectf.top() > lastOffset.y() || lastOffset.y() > rectf.bottom())
//            lastOffset = QPointF(rectf.x(), rectf.center().y());

//        if (rectf.width() > 0) {
//            word.boundingBox = QRectF(lastOffset.x(), rectf.y(), rectf.width() + rectf.x() - lastOffset.x(), rectf.height());

//            lastOffset = QPointF(word.boundingBox.right(), word.boundingBox.center().y());

//            int curWordsSize = m_words.size();
//            if (curWordsSize > 2
//                    && static_cast<int>(m_words.at(curWordsSize - 1).wordBoundingRect().width()) == 0
//                    && m_words.at(curWordsSize - 2).wordBoundingRect().width() > 0
//                    && word.boundingBox.center().y() <= m_words.at(curWordsSize - 2).wordBoundingRect().bottom()
//                    && word.boundingBox.center().y() >= m_words.at(curWordsSize - 2).wordBoundingRect().top()) {
//                QRectF prevBoundRectF = m_words.at(curWordsSize - 2).wordBoundingRect();
//                m_words[curWordsSize - 1].boundingBox = QRectF(prevBoundRectF.right(),
//                                                               prevBoundRectF.y(),
//                                                               word.boundingBox.x() - prevBoundRectF.right(),
//                                                               prevBoundRectF.height());
//            }
//        }

//        m_words.append(word);
//    }

    return m_words;
}

QVector<PageSection> PDFPage::search(const QString &text, bool matchCase, bool wholeWords) const
{
    LOCK_DOCUMENT

    return m_page->search(text, matchCase, wholeWords);
}

QList< Annotation * > PDFPage::annotations() const
{
    QList< Annotation * > annotations;

    const QList<DPdfAnnot *> &annos = m_page->annots();

    foreach (DPdfAnnot *dannotation, annos) {
        annotations.append(new PDFAnnotation(dannotation));
    }

    return annotations;
}

Annotation *PDFPage::addHighlightAnnotation(const QList<QRectF> &boundaries, const QString &text, const QColor &color)
{
    return new PDFAnnotation(m_page->createHightLightAnnot(boundaries, text, color));
}

bool PDFPage::removeAnnotation(Annotation *annotation)
{
    PDFAnnotation *PDFAnnotation = static_cast< class PDFAnnotation * >(annotation);

    if (PDFAnnotation == nullptr)
        return false;

    m_page->removeAnnot(PDFAnnotation->m_dannotation);

    PDFAnnotation->m_dannotation = nullptr;

    delete PDFAnnotation;

    return true;
}

bool PDFPage::updateAnnotation(Annotation *annotation, const QString &text, const QColor &color)
{
    if (nullptr == annotation)
        return false;

    if (m_page->annots().contains(annotation->ownAnnotation())) {
        if (annotation->type() == DPdfAnnot::AText)
            m_page->updateTextAnnot(annotation->ownAnnotation(), text);
        else
            m_page->updateHightLightAnnot(annotation->ownAnnotation(), color, text);
        return true;
    }

    return false;
}

Annotation *PDFPage::addIconAnnotation(const QRectF &rect, const QString &text)
{
    if (nullptr == m_page)
        return nullptr;

    return new PDFAnnotation(m_page->createTextAnnot(rect.center(), text));
}

Annotation *PDFPage::moveIconAnnotation(Annotation *annot, const QRectF &rect)
{
    if (nullptr == m_page || nullptr == annot)
        return nullptr;

    if (annot->ownAnnotation()) {
        m_page->updateTextAnnot(annot->ownAnnotation(), annot->ownAnnotation()->text(), rect.center());

        return annot;
    }
    return nullptr;
}

PDFDocument::PDFDocument(DPdfDoc *document) :
    m_document(document)
{
    m_docMutex = new QMutex;

    QScreen *srn = QApplication::screens().value(0);
    if (nullptr != srn) {
        m_xRes = srn->logicalDotsPerInchX(); // 获取屏幕的横纵向逻辑dpi
        m_yRes = srn->logicalDotsPerInchY();
    }
}

PDFDocument::~PDFDocument()
{
    //需要确保pages先被析构完成
    qDebug() << "正在释放当前 document ...";

    m_docMutex->lock();

    delete m_document;

    m_document = nullptr;

    m_docMutex->unlock();

    delete m_docMutex;
    qDebug() << "当前 document 已释放";
}

int PDFDocument::pageCount() const
{
    return m_document->pageCount();
}

Page *PDFDocument::page(int index) const
{
    if (DPdfPage *page = m_document->page(index, m_xRes, m_yRes)) {

        if (page->isValid())
            return new PDFPage(m_docMutex, page);
    }

    return nullptr;
}

QString PDFDocument::label(int index) const
{
    return m_document->label(index);
}

QStringList PDFDocument::saveFilter() const
{
    return QStringList() << "Portable document format (*.pdf)";
}

bool PDFDocument::save() const
{
    return m_document->save();
}

bool PDFDocument::saveAs(const QString &filePath) const
{
    return m_document->saveAs(filePath);
}

void collectOuleLine(const DPdfDoc::Outline &cOutline, Outline &outline)
{
    for (const DPdfDoc::Section &cSection : cOutline) {
        Section setction;
        setction.nIndex = cSection.nIndex;
        setction.title = cSection.title;
        setction.offsetPointF = cSection.offsetPointF;
        if (cSection.children.size() > 0) {
            collectOuleLine(cSection.children, setction.children);
        }
        outline << setction;
    }
}

Outline PDFDocument::outline() const
{
    if (m_outline.size() > 0)
        return m_outline;

    const DPdfDoc::Outline &cOutline = m_document->outline(m_xRes, m_yRes);

    collectOuleLine(cOutline, m_outline);

    return m_outline;
}

Properties PDFDocument::properties() const
{
    if (m_fileProperties.size() > 0)
        return m_fileProperties;

    m_fileProperties = m_document->proeries();

    return m_fileProperties;
}

PDFDocument *PDFDocument::loadDocument(const QString &filePath, const QString &password, deepin_reader::Document::Error &error)
{
    DPdfDoc *document = new DPdfDoc(filePath, password);

    if (document->status() == DPdfDoc::SUCCESS) {
        error = Document::NoError;
        return new PDFDocument(document);
    } else if (document->status() == DPdfDoc::PASSWORD_ERROR) {
        if (password.isEmpty())
            error = Document::NeedPassword;
        else
            error = Document::WrongPassword;
    } else
        error = Document::FileError;

    delete document;

    return nullptr;
}
}

