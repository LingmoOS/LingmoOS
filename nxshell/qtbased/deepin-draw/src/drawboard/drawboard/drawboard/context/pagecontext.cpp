// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pagecontext.h"
#include "pageview.h"
#include "pagescene.h"
#include "itemgroup.h"
#include "rasteritem.h"
#include "attributemanager.h"
#include "DataHanderInterface.h"
#include "ctextedit.h"
#include "ddfHander/ddfhander.h"
#include "global.h"

#include <QGuiApplication>
#include <QFontDatabase>
#include <QImage>
#include <QPainter>
#include <QDesktopWidget>
#include <QTimer>
#include <QApplication>

static QString genericOneKey()
{
    static int s_pageCount = 0;
    return QString("%1").arg(++s_pageCount);
}
static void update_helper(PageContext *cxt, int active)
{
    if (cxt->page() != nullptr) {
        auto pg = cxt->page();
        pg->setTitle(pg->title());
        pg->showAttributions(pg->borad()->currentAttris(), active);
    }
}

class PageContext::PageContext_private
{
public:
    explicit PageContext_private(PageContext *qq, const QString &file):
        q(qq), _dirty(0), _key(genericOneKey()), _file(FileHander::toLegalFile(file))
    {

    }
    void init()
    {
        _scene = new PageScene(q);
        //设置scene大小为屏幕分辨率
        //获取屏幕分辨率
        QDesktopWidget *desktopWidget = QApplication::desktop();
        QRect screenRect = desktopWidget->screenGeometry();
        //需要乘以系统缩放系数才是最终的大小
        screenRect = QRect(0, 0, qRound(screenRect.width() * desktopWidget->devicePixelRatioF()),
                           qRound(screenRect.height() * desktopWidget->devicePixelRatioF()));
        q->setPageRect(screenRect);

        mustInit();

        if (!q->load(_file)) {
            _file = "";
            _name = tr("Unnamed");
            _name += (_key == "1" ? "" : _key);
        }
    }

    void mustInit()
    {
        q->setDefaultAttri(EPenWidth, 2);
        q->setDefaultAttri(EPenColor, QColor(0, 0, 0));
        q->setDefaultAttri(EBrushColor, QColor(0, 0, 0, 0));
        q->setDefaultAttri(EBorderWidth, 10);
        q->setDefaultAttri(EPenStyle, 1);
        QFontDatabase b;
        q->setDefaultAttri(EFontFamily, getDefaultSystemFont().family());
        q->setDefaultAttri(EFontSize, getDefaultSystemFont().pointSize());
        q->setDefaultAttri(EFontColor, QColor(0, 0, 0));
        q->setDefaultAttri(EFontSize, 14);
        q->setDefaultAttri(EEraserWidth, 20);
        q->setDefaultAttri(ERectRadius, QVariantList() << 5);
        q->setDefaultAttri(EPolygonSides, 5);
        q->setDefaultAttri(EStarAnchor, 5);
        q->setDefaultAttri(EStarInnerOuterRadio, 50);
        q->setDefaultAttri(EStreakEndStyle, noneLine);
        q->setDefaultAttri(EPenBrushColor, QColor(0, 0, 0));
        q->setDefaultAttri(EBlurAttri, 20);
        q->setDefaultAttri(ECutToolAttri, QVariantList() << 1 << QSize(qRound(q->pageRect().width()), qRound(q->pageRect().height())));
        //调整scene大小，需要更新裁剪默认值。
        connect(_scene, &PageScene::sceneRectChanged, q, [ = ](const QRectF & rect) {
            q->setDefaultAttri(ECutToolAttri, QVariantList() << 1 << QSize(qRound(rect.width()), qRound(rect.height())));

        });

        extern void drawboard_ShowAttributions(DrawBoard * board, const SAttrisList & attris);
        connect(_scene, &PageScene::selectionChanged, q, [ = ](const QList<PageItem *> &_t1) {
            if (q->page() != nullptr) {
                q->page()->showAttributions(q->currentAttris(), (q->page()->scene()->selectedItemCount() > 0 ? AttributionManager::ItemSelected : AttributionManager::ForceShow));
            }
        });
    }

    PageContext *q;

    QMap<int, QVariant> _attriValues;

    QString _key;
    QString _file;
    QString _name;

    PageScene *_scene = nullptr;
    Page *_page = nullptr;

    int _dirty: 1;
};

void PageContext_setContextFilePath(PageContext *context, const QString &file)
{
    auto privateContext = context->d_PageContext();
    auto fileTemp = FileHander::toLegalFile(file);
    if (privateContext->_file != fileTemp) {
        if (file.isEmpty()) {
        } else {
            QFileInfo info(file);
            context->setName(info.completeBaseName());
        }
        privateContext->_file = file;
    }
}

PageContext::PageContext(const QString &file, QObject *parent):
    QObject(parent), PageContext_d(new PageContext_private(this, file))
{
    d_PageContext()->init();
}

PageContext::~PageContext()
{
}

QString PageContext::key() const
{
    return d_PageContext()->_key;
}

Page *PageContext::page() const
{
    return d_PageContext()->_scene->page();
}

QString PageContext::name() const
{
    return d_PageContext()->_name;
}

void PageContext::setName(const QString &name)
{
    d_PageContext()->_name = name;
}

QString PageContext::file() const
{
    return d_PageContext()->_file;
}

void PageContext::setFile(const QString &file)
{
    load(file);
}

void PageContext::setDefaultAttri(int type, const QVariant &var)
{
    d_PageContext()->_attriValues[type] = var;
}

QVariant PageContext::defaultAttri(int type) const
{
    auto itf = d_PageContext()->_attriValues.find(type);
    if (itf == d_PageContext()->_attriValues.end()) {
        return QVariant();
    }
    return itf.value();
}

bool PageContext::isEmpty() const
{
    return d_PageContext()->_scene->allPageItems().isEmpty();
}

PageScene *PageContext::scene() const
{
    return d_PageContext()->_scene;
}

void PageContext::addItem(const Unit &ut)
{
    auto item = PageItem::creatItemInstance(ut.head.dataType, ut);
    if (item != nullptr) {
        d_PageContext()->_scene->addPageItem(item);
        d_PageContext()->_scene->selectPageItem(item);
    }
}

void PageContext::addImageItem(const QImage &img, const QPointF &pos, const QRectF &rect)
{
    Unit unit;
    unit.head.dataType = RasterItemType;

    QRectF rct = rect;

    if (rct.isNull())
        rct = img.rect();

    RasterUnitData i;
    QPointF ps = pos.isNull() ? pageRect().center() - rct.center() : pos;

    unit.head.pos = ps;
    unit.head.rect = rct;

    i.baseImg = img;
    i.blocked = false;
    i.baseRect = rct;
    i.layerType = 1; //mean picture layer(0 is pen layer)
    unit.data.data.setValue<RasterUnitData>(i);

    this->addItem(unit);
}

//void PageContext::addTextItem(const QString &text)
//{
//    Unit unit;
//    unit.head.dataType = TextType;
//    unit.head.zValue = scene()->getMaxZValue() + 1;

//    TextUnitData i;
//    TextUnitData *p = &i;
//    p->content = text;
//    p->font.setFamily(defaultAttri(EFontFamily).toString());
//    p->font.setWeight(TextEdit::toWeight(defaultAttri(EFontWeightStyle).toString()));
//    p->font.setPixelSize(defaultAttri(EFontSize).toInt());
//    unit.data.data.setValue<TextUnitData>(i);
//    this->addItem(unit);
//}

void PageContext::loadData(const PageContextData &data)
{
    setPageRect(data.head.rect);
    loadUnits(data.units, true);
}

void PageContext::loadUnits(const UnitList &units, bool clear)
{
    auto scene = this->scene();

    if (clear)
        scene->clearAll();

    //禁止选中和自动赋予z值的操作(z值可以通过数据加载确定)
    extern bool globel_LayerItem_ZAssignBLocked;
    scene->blockAssignZValue(true);
    scene->blockSelect(true);
    globel_LayerItem_ZAssignBLocked = true;

    foreach (auto ut, units) {
        this->addItem(ut);
    }
    scene->blockSelect(false);
    scene->blockAssignZValue(false);
    globel_LayerItem_ZAssignBLocked = false;
}

QRectF PageContext::pageRect() const
{
    return d_PageContext()->_scene->sceneRect();
}

void PageContext::setPageRect(const QRectF &rect)
{
    d_PageContext()->_scene->setSceneRect(rect);
}

QImage PageContext::renderToImage(const QColor &bgColor, const QSize &desImageSize) const
{
    return d_PageContext()->_scene->renderToImage(bgColor, desImageSize);
}

void PageContext::showAtttris(int active)
{
    update_helper(this, active);
}

bool PageContext::save(const QString &file)
{
    auto filePath = file.isEmpty() ? this->file() : file;
    if (filePath.isEmpty())
        return false;

    if (filePath == this->file() && !isDirty())
        return true;

    if (page() != nullptr && page()->borad() != nullptr) {
        QFileInfo info(filePath);
        if ("ddf" == info.suffix().toLower()) {
            d_PageContext()->_dirty = 0;
            return page()->borad()->fileHander()->saveToDdf(this, filePath);
        } else {
            d_PageContext()->_dirty = 0;
            return page()->borad()->fileHander()->saveToImage(this, filePath);
        }
    }
    return false;
}

bool PageContext::load(const QString &file)
{
    auto fileTemp = FileHander::toLegalFile(file);
    if (d_PageContext()->_file != fileTemp) {
        DdfHander hander(file, this);
        return hander.load();
    }
    return false;
}

bool PageContext::isDirty() const
{
    return d_PageContext()->_dirty;
}

void PageContext::setDirty(bool dirty)
{
    if (d_PageContext()->_dirty != dirty) {
        bool currenAnyPageIsModified = (page() == nullptr ? false : page()->borad()->isAnyPageModified());

        d_PageContext()->_dirty = dirty;
        emit dirtyChanged(dirty);

        bool newAnyPageIsModified = (page() == nullptr ? false : page()->borad()->isAnyPageModified());
        if (currenAnyPageIsModified != newAnyPageIsModified) {
            emit page()->borad()->modified(newAnyPageIsModified);
        }
        showAtttris(AttributionManager::ItemAttriChanged);
    }
}

SAttrisList PageContext::currentAttris() const
{
    return d_PageContext()->_scene->currentAttris();
}

//QImage PageContext::toImage(const QSize &size) const
//{
//    QImage image(size.isValid() ? size : _scene->sceneRect().size().toSize(), QImage::Format_ARGB32);

//    QPainter painter(&image);
//    _scene->render(&painter, QRectF(0, 0, image.width(), image.height()), _scene->sceneRect());
//    painter.end();

//    return image;

//    return QImage();
//}

