/*******************************************************************
 * productmapping.h
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#ifndef PRODUCTMAPPING__H
#define PRODUCTMAPPING__H

#include <QObject>
#include <QString>
#include <QStringList>

#include "bugzillaintegration/libbugzilla/clients/productclient.h"

class Product;
class BugzillaManager;
class CrashedApplication;

/**
 * Maps our crashed entity to a bugzilla product/component/version.
 */
class ProductMapping : public QObject
{
    Q_OBJECT
public:
    explicit ProductMapping(const CrashedApplication *, BugzillaManager *, QObject *parent = nullptr);

    QString bugzillaProduct() const;
    // If bugzillaProduct is a fallback product, then this is non-empty original product string we tried to find.
    QString bugzillaProductOriginal() const;
    QString bugzillaComponent() const;
    QString bugzillaVersion() const;
    QStringList relatedBugzillaProducts() const;

    bool bugzillaProductDisabled() const;
    bool bugzillaVersionDisabled() const;

    Q_SIGNAL void resolved();

private Q_SLOTS:
    void checkProductInfo(const Bugzilla::Product::Ptr);
    // Fall back to generic product because the product failed to resolve.
    void fallBackToKDE();

private:
    void map(const QString &);
    void mapUsingInternalFile(const QString &);
    void getRelatedProductsUsingInternalFile(const QString &);

    QStringList m_relatedBugzillaProducts;
    QString m_bugzillaProduct;
    QString m_bugzillaProductOriginal;
    QString m_bugzillaComponent;

    QString m_bugzillaVersionString;

    const CrashedApplication *m_crashedAppPtr = nullptr;
    BugzillaManager *m_bugzillaManagerPtr = nullptr;

    bool m_bugzillaProductDisabled;
    bool m_bugzillaVersionDisabled;
    bool m_hasExternallyProvidedProductName = false;

    QMetaObject::Connection m_productInfoErrorConnection;
};

#endif
