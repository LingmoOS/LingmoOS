// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKCONTROLPALETTE_P_H
#define DQUICKCONTROLPALETTE_P_H

#include <dtkdeclarative_global.h>
#include "dqmlglobalobject_p.h"

#include <QColor>
#include <QObject>
#include <QQmlListProperty>
#include <QQmlEngine>
#include <QVector>
#include <QPointer>

#include <private/qqmldata_p.h>

QT_BEGIN_NAMESPACE
class QQuickItem;
class QQuickWindow;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE
class DQuickControlColor
{
    friend class DQuickControlPalette;
    Q_GADGET
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DColor common READ common WRITE setCommon FINAL)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DColor crystal READ crystal WRITE setCrystal FINAL)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ANONYMOUS
#endif

public:
    DQuickControlColor();
    DQuickControlColor(const DColor &color);
    DQuickControlColor(DQuickControlColor &&other)
        {data = std::move(other.data); other.data = nullptr;
         changed = std::move(other.changed);
         isSingleColor = std::move(other.isSingleColor);}
    DQuickControlColor(const DQuickControlColor &other) {
        isSingleColor = other.isSingleColor;
        if (other.isSingleColor) {
            data = new DColor(*other.data);
        } else {
            data = other.data;
            changed = other.changed;
        }
    }
    inline DQuickControlColor &operator=(const DQuickControlColor &other) {
        if (isSingleColor) {
            if (other.isSingleColor) {
                *data = *other.data;
            } else {
                delete data;
                data = other.data;
                isSingleColor = false;
            }
        } else {
            if (other.isSingleColor) {
                if (data) {
                    setCommon(*other.data);
                } else {
                    data = new DColor(*other.data);
                }
            } else {
                data = other.data;
            }
        }
        changed = other.changed;
        return *this;
    }
    inline DQuickControlColor &operator=(DQuickControlColor &&other) {
        if (data && isSingleColor) {
            delete data;
        }

        data = std::move(other.data);
        other.data = nullptr;
        changed = other.changed;
        isSingleColor = other.isSingleColor;
        return *this;
    }
    ~DQuickControlColor();

    const DColor &common() const;
    void setCommon(const DColor &newCommon);
    const DColor &crystal() const;
    void setCrystal(const DColor &newCrystal);

private:
    DQuickControlColor(DColor *colors);

    DColor *data = nullptr;
    bool changed = false;
    bool isSingleColor = false;
};
DQUICK_END_NAMESPACE
Q_DECLARE_METATYPE(DTK_QUICK_NAMESPACE::DQuickControlColor)

DQUICK_BEGIN_NAMESPACE

class DQuickControlPalette : public QObject
{
    friend class DQuickControlColorSelector;
    Q_OBJECT
    Q_DISABLE_COPY(DQuickControlPalette)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor normal READ normal WRITE setNormal NOTIFY normalChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor normalDark READ normalDark WRITE setNormalDark NOTIFY normalDarkChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor hovered READ hovered WRITE setHovered NOTIFY hoveredChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor hoveredDark READ hoveredDark WRITE setHoveredDark NOTIFY hoveredDarkChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor pressed READ pressed WRITE setPressed NOTIFY pressedChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor pressedDark READ pressedDark WRITE setPressedDark NOTIFY pressedDarkChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor disabled READ disabled WRITE setDisabled NOTIFY disabledChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlColor disabledDark READ disabledDark WRITE setDisabledDark NOTIFY disabledDarkChanged)

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(Palette)
#endif
public:
    enum ColorFamily {
        CommonColor = 0,
        CrystalColor = 1,
        ColorFamilyCount
    };
    Q_ENUM(ColorFamily)

    enum ColorGroup {
        Light = 0,
        Dark = 1 * ColorFamilyCount,
        ThemeTypeCount = 2,

        Normal = 0,
        Hovered = 1 * (ThemeTypeCount + ColorFamilyCount),
        Pressed = 2 * (ThemeTypeCount + ColorFamilyCount),
        Disabled = 3 * (ThemeTypeCount + ColorFamilyCount),
        ColorTypeCount = 4
    };

    explicit DQuickControlPalette(QObject *parent = nullptr);
    ~DQuickControlPalette();

    bool enabled() const;
    void setEnabled(bool newEnabled);

    inline DColor *colorPointer(int colorPropertyIndex) {
        return colors.data() + colorPropertyIndex * ColorFamilyCount;
    }
    inline bool setTo(const DQuickControlColor &color, int colorPropertyIndex) {
        auto colors = colorPointer(colorPropertyIndex);
        Q_ASSERT(colors);
        if (color.data == colors)
            return color.changed;
        bool changed = false;
        const int colorCount = color.isSingleColor ? 1 : ColorFamilyCount;
        for (int i = 0; i < colorCount; ++i) {
            if (color.data[i] == colors[i])
                continue;
            colors[i] = color.data[i];
            changed = true;
        }
        return changed;
    }
    void setNormal(const DQuickControlColor &color) {
        if (!setTo(color, 0))
            return;
        Q_EMIT normalChanged();
        Q_EMIT changed();
    }
    DQuickControlColor normal() {
        return DQuickControlColor(colorPointer(0));
    }
    void setNormalDark(const DQuickControlColor &color) {
        if (!setTo(color, 1))
            return;
        Q_EMIT normalDarkChanged();
        Q_EMIT changed();
    }
    DQuickControlColor normalDark() {
        return DQuickControlColor(colorPointer(1));
    }
    void setHovered(const DQuickControlColor &color) {
        if (!setTo(color, 2))
            return;
        Q_EMIT hoveredChanged();
        Q_EMIT changed();
    }
    DQuickControlColor hovered() {
        return DQuickControlColor(colorPointer(2));
    }
    void setHoveredDark(const DQuickControlColor &color) {
        if (!setTo(color, 3))
            return;
        Q_EMIT hoveredDarkChanged();
        Q_EMIT changed();
    }
    DQuickControlColor hoveredDark() {
        return DQuickControlColor(colorPointer(3));
    }
    void setPressed(const DQuickControlColor &color) {
        if (!setTo(color, 4))
            return;
        Q_EMIT pressedChanged();
        Q_EMIT changed();
    }
    DQuickControlColor pressed() {
        return DQuickControlColor(colorPointer(4));
    }
    void setPressedDark(const DQuickControlColor &color) {
        if (!setTo(color, 5))
            return;
        Q_EMIT pressedDarkChanged();
        Q_EMIT changed();
    }
    DQuickControlColor pressedDark() {
        return DQuickControlColor(colorPointer(5));
    }
    void setDisabled(const DQuickControlColor &color) {
        if (!setTo(color, 6))
            return;
        Q_EMIT disabledChanged();
        Q_EMIT changed();
    }
    DQuickControlColor disabled() {
        return DQuickControlColor(colorPointer(6));
    }
    void setDisabledDark(const DQuickControlColor &color) {
        if (!setTo(color, 7))
            return;
        Q_EMIT disabledDarkChanged();
        Q_EMIT changed();
    }
    DQuickControlColor disabledDark() {
        return DQuickControlColor(colorPointer(7));
    }

Q_SIGNALS:
    void enabledChanged();
    void normalChanged();
    void normalDarkChanged();
    void hoveredChanged();
    void hoveredDarkChanged();
    void pressedChanged();
    void pressedDarkChanged();
    void disabledChanged();
    void disabledDarkChanged();
    void changed();

public:
    QVector<DColor> colors;

private:
    bool m_enabled = true;
};
DQUICK_END_NAMESPACE
QML_DECLARE_TYPE(DTK_QUICK_NAMESPACE::DQuickControlPalette)

DQUICK_BEGIN_NAMESPACE
class CustomMetaObject;
class DQuickControlColorSelector : public QObject
{
    friend class CustomMetaObject;
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_UNCREATABLE("ColorSelector is only available as an attached property.")
    QML_NAMED_ELEMENT(ColorSelector)
    QML_ATTACHED(DQuickControlColorSelector)
#endif
    Q_DISABLE_COPY(DQuickControlColorSelector)
public:
    Q_PROPERTY(QQuickItem *control READ control NOTIFY controlChanged)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType controlTheme READ controlTheme NOTIFY controlThemeChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQMLGlobalObject::ControlState controlState READ controlState NOTIFY controlStateChanged)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickControlPalette::ColorFamily family READ family WRITE setFamily RESET resetFamily NOTIFY familyChanged)
    Q_PROPERTY(bool hovered READ hovered WRITE setHovered RESET resetHovered NOTIFY hoveredChanged)
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed RESET resetPressed NOTIFY pressedChanged)
    Q_PROPERTY(bool disabled READ disabled WRITE setDisabled RESET resetDisabled NOTIFY disabledChanged)
    Q_PROPERTY(bool inactived READ inactived WRITE setInactived RESET resetInactived NOTIFY inactivedChanged)
    Q_CLASSINFO("DefaultProperty", "palettes")

    explicit DQuickControlColorSelector(QQuickItem *parent);
    ~DQuickControlColorSelector();

    static DQuickControlColorSelector *qmlAttachedProperties(QObject *object);

    QQuickItem *control() const;
    void setControl(QQuickItem *newControl);

    DQMLGlobalObject::ControlState controlState() const;
    DGuiApplicationHelper::ColorType controlTheme() const;
    Dtk::Quick::DQuickControlPalette::ColorFamily family() const;
    void setFamily(const DQuickControlPalette::ColorFamily &newFamily);
    void resetFamily();

    bool hovered() const;
    void setHovered(bool newHovered);
    void resetHovered();

    bool pressed() const;
    void setPressed(bool newPressed);
    void resetPressed();

    bool disabled() const;
    void setDisabled(bool newDisabled);
    void resetDisabled();

    bool inactived() const;
    void setInactived(bool newInactived);
    void resetInactived();

    static QStringList specialObjectNameItems();

Q_SIGNALS:
    void controlThemeChanged();
    void controlStateChanged();
    void controlChanged();
    void hoveredChanged();
    void pressedChanged();
    void disabledChanged();
    void inactivedChanged();
    void colorPropertyChanged(const QByteArray &name);
    void colorPropertiesChanged();
    void familyChanged();

private:
    bool doGetHoveredRecu(bool *value) const;
    bool doGetPressedRecu(bool *value) const;
    bool doGetDisabledRecu(bool *value) const;
    bool doGetInactivedRecu(bool *value) const;
    void doSetHovered(bool newHovered, bool isUserSet);
    void doSetPressed(bool newPressed, bool isUserSet);
    void doSetDisabled(bool newDisabled, bool isUserSet);
    void doSetInactived(bool newInactived, bool isUserSet);

    void ensureMetaObject();
    void setupMetaPropertyPalettes(QQuickItem *object);
    struct PaletteState;
    QColor getColorOf(const DQuickControlPalette *palette, const PaletteState *state) const;
    QColor getColorOf(const QByteArray &propertyName, const PaletteState *state) const;

    int indexOfPalette(const QByteArray &name) const;
    void setPalette(const QByteArray &name, DQuickControlPalette *palette);

    void updatePaletteFromMetaProperty(const QMetaProperty &mp, const QObject *obj);
    void updatePropertyFromName(const QByteArray &name, const DQuickControlPalette *palette = nullptr);
    void updateFamilyForChildrenRecu(QQuickItem *parent);

    void findAndSetControlParent();
    QByteArray findPalettePropertyName(const DQuickControlPalette *palette) const;
    void clearAndInheritParentProperties();

    Q_SLOT void updateControlTheme();
    Q_SLOT bool updateControlState();
    Q_SLOT void updateAllColorProperties();
    Q_SLOT void recvPaletteColorChanged();
    Q_SLOT void onPaletteDestroyed();
    Q_SLOT void updateControlWindow();
    Q_SLOT void resolveMetaPropertyChanged();
    Q_SLOT void notifyColorPropertyChanged();

    bool setControlTheme(DGuiApplicationHelper::ColorType theme);
    bool setControlState(DQMLGlobalObject::ControlState state);
    bool doSetFamily(DQuickControlPalette::ColorFamily newFamily);
    void doResetFamily();

    void tryDestroyPalette(DQuickControlPalette *palette);
    void setSuperColorSelector(DQuickControlColorSelector *parent);
    void setFamilyPropertyParent(DQuickControlColorSelector *parent);

    QQuickItem *m_control = nullptr;
    QQuickWindow *m_controlWindow = nullptr;
    QPointer<DQuickControlColorSelector> m_superColorSelector;
    QPointer<DQuickControlColorSelector> m_parentOfFamilyProperty;
    typedef QPair<QByteArray, DQuickControlPalette*> ControlPaletteData;
    QList<ControlPaletteData> m_palettes;
    CustomMetaObject *m_metaObject = nullptr;
#if QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
    QQmlPropertyCache *m_propertyCache = nullptr;
#else
    QQmlPropertyCache::ConstPtr m_propertyCache;
#endif
    struct PaletteState {
        PaletteState(DQuickControlColorSelector *owner)
            : owner(owner)
            , controlState(DQMLGlobalObject::NormalState)
            , family(DQuickControlPalette::CommonColor)
            , familyIsUserSet(false)
            , hovered(false)
            , hoveredValueValid(false)
            , pressed(false)
            , pressedValueValid(false)
            , disabled(false)
            , disabledValueValid(false)
            , inactived(false)
            , inactivedValueValid(false)
        {
            if (auto appHelper = DGuiApplicationHelper::instance()) {
                controlTheme = appHelper->themeType();
            } else {
                controlTheme = DGuiApplicationHelper::LightType;
            }
        }

        DQuickControlColorSelector *owner = nullptr;
        DGuiApplicationHelper::ColorType controlTheme;
        DQMLGlobalObject::ControlState controlState;
        DQuickControlPalette::ColorFamily family;
        uint familyIsUserSet:1;
        uint hovered:1;
        uint hoveredValueValid:1;
        uint pressed:1;
        uint pressedValueValid:1;
        uint disabled:1;
        uint disabledValueValid:1;
        uint inactived:1;
        uint inactivedValueValid:1;
    };
    QScopedPointer<PaletteState> m_state;
    QList<QMetaObject::Connection> m_itemParentChangeConnections;
};

DQUICK_END_NAMESPACE
QML_DECLARE_TYPE(DTK_QUICK_NAMESPACE::DQuickControlColorSelector)
QML_DECLARE_TYPEINFO(DTK_QUICK_NAMESPACE::DQuickControlColorSelector, QML_HAS_ATTACHED_PROPERTIES)

#endif // DQUICKCONTROLPALETTE_P_H
