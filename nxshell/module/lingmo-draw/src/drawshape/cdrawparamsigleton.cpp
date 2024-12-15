// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cdrawparamsigleton.h"
#include <QGuiApplication>
#include <QFontDatabase>
#include <QImage>
#include <QPainter>
#include <QDesktopWidget>
#include <QTimer>

#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cdrawscene.h"
#include "application.h"
#include "cattributemanagerwgt.h"
#include "toptoolbar.h"
#include "cgraphicsitemselectedmgr.h"
#include "cattributemanagerwgt.h"
#include "cdrawtoolmanagersigleton.h"
#include "filehander.h"
#include "ctextedit.h"

static QString genericOneKey()
{
    static int s_pageCount = 0;
    return QString("%1").arg(++s_pageCount);
}
static void update_helper(PageContext *cxt)
{
    if (cxt->page() != nullptr) {

        auto pg = cxt->page();
        pg->setTitle(pg->title());
        if (pg->borad()->currentPage() == pg) {
            pg->borad()->attributionWidget()->setAttributions(pg->borad()->currentAttris());
        }
    }
}

PageContext::PageContext(const QString &file, QObject *parent): QObject(parent), _dirty(0)
{
    _key  = genericOneKey();

    //setFile(file);
    _file = FileHander::toLegalFile(file);

    if (file.isEmpty()) {
        _name = tr("Unnamed");
        _name += (_key == "1" ? "" : _key);
    } else {
        QFileInfo info(file);
        _name = info.completeBaseName();
    }

    _scene = new PageScene(this);

    //设置scene大小为屏幕分辨率
    //获取屏幕分辨率
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();
    //需要乘以系统缩放系数才是最终的大小
    screenRect = QRect(0, 0, qRound(screenRect.width() * desktopWidget->devicePixelRatioF()),
                       qRound(screenRect.height() * desktopWidget->devicePixelRatioF()));
    this->setPageRect(screenRect);
}

QString PageContext::key() const
{
    return _key;
}

Page *PageContext::page() const
{
    return _scene->page();
}

QString PageContext::name() const
{
    return _name;
}

void PageContext::setName(const QString &name)
{
    _name = name;
}

QString PageContext::file() const
{
    return _file;
}

void PageContext::setFile(const QString &file)
{
    auto fileTemp = FileHander::toLegalFile(file);
    if (_file != fileTemp) {
        if (file.isEmpty()) {
        } else {
            QFileInfo info(file);
            _name = info.completeBaseName();
        }
        _file = file;
        setDirty(true);
    }
}

void PageContext::setDefaultAttri(int type, const QVariant &var)
{
    _attriValues[type] = var;
}

QVariant PageContext::defaultAttri(int type) const
{
    auto itf = _attriValues.find(type);
    if (itf == _attriValues.end()) {
        return QVariant();
    }
    return itf.value();
}

bool PageContext::isEmpty() const
{
    return _scene->getBzItems().isEmpty();
}

PageScene *PageContext::scene() const
{
    return _scene;
}

void PageContext::addSceneItem(const CGraphicsUnit &var, bool record, bool releaseUnit, bool select)
{
    if (QThread::currentThread() != qApp->thread()) {
        QMetaObject::invokeMethod(this, [ = ]() {
            auto item = CGraphicsItem::creatItemInstance(var.head.dataType, var);
            if (item != nullptr) {
                _scene->addCItem(item, false, record);
            }
            if (releaseUnit) {
                const_cast<CGraphicsUnit &>(var).release();
            }
            if (select)
                _scene->selectItem(item);
        }, Qt::QueuedConnection);
    } else {
        auto item = CGraphicsItem::creatItemInstance(var.head.dataType, var);
        if (item != nullptr) {
            _scene->addCItem(item, false, record);
        }
        if (select)
            _scene->selectItem(item);
        if (releaseUnit) {
            const_cast<CGraphicsUnit &>(var).release();
        }
    }
}

void PageContext::addImage(const QImage &img, const QPointF &pos, const QRectF &rect, bool record, bool select)
{
    CGraphicsUnit unit;
    unit.head.dataType = DyLayer;

    QRectF rct = rect;

    if (rct.isNull())
        rct = img.rect();

    QPointF ps = pos.isNull() ? pageRect().center() - rct.center() : pos;
    SDynamicLayerUnitData *p = new SDynamicLayerUnitData;
    JGeomeCommand *com = new JGeomeCommand(ps.toPoint(), 0, scene()->getMaxZValue() + 1, rct, QTransform());
    p->commands.append(QSharedPointer<JDyLayerCmdBase>(com));
    p->baseImg = img;
    p->blocked = true;
    p->layerType = 1; //mean picture layer(0 is pen layer)
    unit.data.pDyLayer = p;

    this->addSceneItem(unit, record, true, select);
}

//void PageContext::addText(const QString &text, bool record, bool select)
//{
//    CGraphicsUnit unit;
//    unit.head.dataType = TextType;
//    unit.head.zValue = scene()->getMaxZValue() + 1;
//    SGraphicsTextUnitData *p = new SGraphicsTextUnitData;
//    p->content = text;
//    p->font.setFamily(defaultAttri(EFontFamily).toString());
//    p->font.setWeight(CTextEdit::toWeight(defaultAttri(EFontWeightStyle).toString()));
//    p->font.setPixelSize(defaultAttri(EFontSize).toInt());
//    unit.data.pText = p;
//    this->addSceneItem(unit, record, select);
//}

QRectF PageContext::pageRect() const
{
    return _scene->sceneRect();
}

void PageContext::setPageRect(const QRectF &rect)
{
    _scene->setSceneRect(rect);
}

QImage PageContext::renderToImage(const QColor &bgColor, const QSize &desImageSize) const
{
    return _scene->renderToImage(bgColor, desImageSize);
}

void PageContext::update()
{
    update_helper(this);
}

bool PageContext::save(const QString &file)
{
    auto filePath = file.isEmpty() ? this->file() : file;
    if (filePath.isEmpty())
        return false;

    if (filePath == this->file() && !isDirty())
        return true;

//    if (isEmpty())
//        return true;
    bool rs = false;
    if (page() != nullptr && page()->borad() != nullptr) {
        QFileInfo info(filePath);
        if ("ddf" == info.suffix().toLower()) {
            rs = page()->borad()->fileHander()->saveToDdf(this, filePath);
        } else {
            rs = page()->borad()->fileHander()->saveToImage(this, filePath);
            //保存成功
            if (rs) {
                setFile(filePath);
                setDirty(false);
            }
        }
        //return  page()->borad()->fileHander()->saveToDdf(this, filePath);
    }
    return rs;
}

bool PageContext::isDirty() const
{
    return _dirty;
}

void PageContext::setDirty(bool dirty)
{
    if (_dirty != dirty) {
        bool drawboardIsModified = (page() != nullptr) ? page()->borad()->isAnyPageModified() : false;
        _dirty = dirty;
        emit dirtyChanged(dirty);
        bool nowDrawboardIsModified = (page() != nullptr) ? page()->borad()->isAnyPageModified() : false;
        if (drawboardIsModified != nowDrawboardIsModified) {
            emit page()->borad()->modified(nowDrawboardIsModified);
        }

        update();
    }
}

SAttrisList PageContext::currentAttris() const
{
    return _scene->currentAttris();
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

