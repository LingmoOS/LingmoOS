// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDRAWPARAMSIGLETON_H
#define CDRAWPARAMSIGLETON_H
#include "globaldefine.h"
#include "sitemdata.h"
#include "attributewidget.h"

#include <QtGlobal>
#include <QColor>
#include <QPen>
#include <QFont>
class Page;
class PageScene;

class DRAWLIB_EXPORT PageContext: public QObject
{
    Q_OBJECT
public:
    PageContext(const QString &file = "", QObject *parent = nullptr);
    ~PageContext();

    QString key() const;

    Page *page()const;

    QString name()const;
    void    setName(const QString &name);

    QString file()const;
    void setFile(const QString &file);

    bool save(const QString &file = "");
    bool load(const QString &file);

    bool isDirty()const;
    void setDirty(bool dirty);

    SAttrisList currentAttris() const;
    void setDefaultAttri(int type, const QVariant &var);
    QVariant defaultAttri(int type) const;
    void showAtttris(int active);

    bool isEmpty()const;
    PageScene *scene() const;

    void addItem(const Unit &ut);
    void addImageItem(const QImage &img, const QPointF &pos = QPointF(), const QRectF &rect = QRectF());
    //void addTextItem(const QString &text);

    Q_INVOKABLE void loadData(const PageContextData &data);
    Q_INVOKABLE void loadUnits(const UnitList &units, bool clear = false);

    QRectF pageRect() const;
    void   setPageRect(const QRectF &rect);

    QImage renderToImage(const QColor &bgColor = Qt::transparent, const QSize &desImageSize = QSize()) const;
signals:
    void dirtyChanged(bool isDirty);

    PRIVATECLASS(PageContext)
};


#endif // CDRAWPARAMSIGLETON_H
