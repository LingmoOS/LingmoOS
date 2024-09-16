// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKSYSTEMPALETTE_H
#define DQUICKSYSTEMPALETTE_H

#include <dtkdeclarative_global.h>

#include <QColor>
#include <DPalette>

#include <DObject>

DQUICK_BEGIN_NAMESPACE

class DQuickSystemPalettePrivate;
class D_DECL_DEPRECATED_X("Use DQMLGlobalObject::palette.") DQuickSystemPalette : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(DGUI_NAMESPACE::DPalette palette READ palette NOTIFY paletteChanged)
    Q_PROPERTY(DQuickSystemPalette::ColorGroup colorGroup READ colorGroup WRITE setColorGroup NOTIFY paletteChanged)
    // QPalette
    Q_PROPERTY(QColor window READ window NOTIFY paletteChanged)
    Q_PROPERTY(QColor windowText READ windowText NOTIFY paletteChanged)
    Q_PROPERTY(QColor base READ base NOTIFY paletteChanged)
    Q_PROPERTY(QColor text READ text NOTIFY paletteChanged)
    Q_PROPERTY(QColor alternateBase READ alternateBase NOTIFY paletteChanged)
    Q_PROPERTY(QColor button READ button NOTIFY paletteChanged)
    Q_PROPERTY(QColor buttonText READ buttonText NOTIFY paletteChanged)
    Q_PROPERTY(QColor light READ light NOTIFY paletteChanged)
    Q_PROPERTY(QColor midlight READ midlight NOTIFY paletteChanged)
    Q_PROPERTY(QColor dark READ dark NOTIFY paletteChanged)
    Q_PROPERTY(QColor mid READ mid NOTIFY paletteChanged)
    Q_PROPERTY(QColor shadow READ shadow NOTIFY paletteChanged)
    Q_PROPERTY(QColor highlight READ highlight NOTIFY paletteChanged)
    Q_PROPERTY(QColor highlightedText READ highlightedText NOTIFY paletteChanged)

    // DPalette
    Q_PROPERTY(QColor itemBackground READ itemBackground NOTIFY paletteChanged)
    Q_PROPERTY(QColor textTitle READ textTitle NOTIFY paletteChanged)
    Q_PROPERTY(QColor textTips READ textTips NOTIFY paletteChanged)
    Q_PROPERTY(QColor textWarning READ textWarning NOTIFY paletteChanged)
    Q_PROPERTY(QColor textLively READ textLively NOTIFY paletteChanged)
    Q_PROPERTY(QColor lightLively READ lightLively NOTIFY paletteChanged)
    Q_PROPERTY(QColor darkLively READ darkLively NOTIFY paletteChanged)
    Q_PROPERTY(QColor frameBorder READ frameBorder NOTIFY paletteChanged)
    Q_PROPERTY(QColor placeholderText READ placeholderText NOTIFY paletteChanged)
    Q_PROPERTY(QColor frameShadowBorder READ frameShadowBorder NOTIFY paletteChanged)
    Q_PROPERTY(QColor obviousBackground READ obviousBackground NOTIFY paletteChanged)

public:
    DQuickSystemPalette(QObject *parent = Q_NULLPTR);
    ~DQuickSystemPalette() override;

    enum ColorGroup { Active = QPalette::Active, Inactive = QPalette::Inactive, Disabled = QPalette::Disabled };
    Q_ENUM(ColorGroup)

    const DGUI_NAMESPACE::DPalette &palette() const;
    DQuickSystemPalette::ColorGroup colorGroup() const;

    QColor window() const;
    QColor windowText() const;
    QColor base() const;
    QColor text() const;
    QColor alternateBase() const;
    QColor button() const;
    QColor buttonText() const;
    QColor light() const;
    QColor midlight() const;
    QColor dark() const;
    QColor mid() const;
    QColor shadow() const;
    QColor highlight() const;
    QColor highlightedText() const;

    QColor itemBackground() const;
    QColor textTitle() const;
    QColor textTips() const;
    QColor textWarning() const;
    QColor textLively() const;
    QColor lightLively() const;
    QColor darkLively() const;
    QColor frameBorder() const;
    QColor placeholderText() const;
    QColor frameShadowBorder() const;
    QColor obviousBackground() const;

public Q_SLOTS:
    void setColorGroup(DQuickSystemPalette::ColorGroup colorGroup);

Q_SIGNALS:
    void paletteChanged();

private:
    D_DECLARE_PRIVATE(DQuickSystemPalette)
};

DQUICK_END_NAMESPACE

#endif // DQUICKSYSTEMPALETTE_H
