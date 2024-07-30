/*
    SPDX-FileCopyrightText: 2017 by Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 by David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd. <https://www.qt.io/licensing/>

    This file is part of the Qt Quick Controls module of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial
*/

#ifndef KQUICKSTYLEITEM_P_H
#define KQUICKSTYLEITEM_P_H

#include "kquickpadding_p.h"
#include <QImage>
#include <QPointer>
#include <QQuickImageProvider>
#include <QQuickItem>

#include <qqmlregistration.h>

#include <memory>

class QWidget;
class QStyleOption;
class QStyle;

namespace Kirigami
{
namespace Platform
{
class PlatformTheme;
}
}

class QQuickTableRowImageProvider1 : public QQuickImageProvider
{
public:
    QQuickTableRowImageProvider1()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
    }
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};

class KQuickStyleItem : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(StyleItem)

    Q_PROPERTY(KQuickPadding *border READ border CONSTANT)

    Q_PROPERTY(bool sunken READ sunken WRITE setSunken NOTIFY sunkenChanged)
    Q_PROPERTY(bool raised READ raised WRITE setRaised NOTIFY raisedChanged)
    Q_PROPERTY(bool flat READ flat WRITE setFlat NOTIFY flatChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(bool hasFocus READ hasFocus WRITE setHasFocus NOTIFY hasFocusChanged)
    Q_PROPERTY(bool on READ on WRITE setOn NOTIFY onChanged)
    Q_PROPERTY(bool hover READ hover WRITE setHover NOTIFY hoverChanged)
    Q_PROPERTY(bool horizontal READ horizontal WRITE setHorizontal NOTIFY horizontalChanged)
    Q_PROPERTY(bool isTransient READ isTransient WRITE setTransient NOTIFY transientChanged)

    Q_PROPERTY(QString elementType READ elementType WRITE setElementType NOTIFY elementTypeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString activeControl READ activeControl WRITE setActiveControl NOTIFY activeControlChanged)
    Q_PROPERTY(QString styleName READ styleName NOTIFY styleNameChanged)
    Q_PROPERTY(QVariantMap hints READ hints WRITE setHints NOTIFY hintChanged RESET resetHints)
    Q_PROPERTY(QVariantMap properties READ properties WRITE setProperties NOTIFY propertiesChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont RESET resetFont NOTIFY fontChanged)

    // For range controls
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int step READ step WRITE setStep NOTIFY stepChanged)
    Q_PROPERTY(int paintMargins READ paintMargins WRITE setPaintMargins NOTIFY paintMarginsChanged)

    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY(int textureWidth READ textureWidth WRITE setTextureWidth NOTIFY textureWidthChanged)
    Q_PROPERTY(int textureHeight READ textureHeight WRITE setTextureHeight NOTIFY textureHeightChanged)

    Q_PROPERTY(int leftPadding READ leftPadding NOTIFY leftPaddingChanged)
    Q_PROPERTY(int topPadding READ topPadding NOTIFY topPaddingChanged)
    Q_PROPERTY(int rightPadding READ rightPadding NOTIFY rightPaddingChanged)
    Q_PROPERTY(int bottomPadding READ bottomPadding NOTIFY bottomPaddingChanged)

    Q_PROPERTY(QQuickItem *control READ control WRITE setControl NOTIFY controlChanged)

    KQuickPadding *border()
    {
        return &m_border;
    }

public:
    KQuickStyleItem(QQuickItem *parent = nullptr);
    ~KQuickStyleItem() override;

    enum MenuItemType {
        SeparatorType = 0,
        ItemType,
        MenuType,
        ScrollIndicatorType,
    };

    enum Type {
        Undefined,
        Button,
        RadioButton,
        CheckBox,
        ComboBox,
        ComboBoxItem,
        Dial,
        ToolBar,
        ToolButton,
        Tab,
        TabFrame,
        Frame,
        FocusFrame,
        FocusRect,
        SpinBox,
        Slider,
        ScrollBar,
        ProgressBar,
        Edit,
        GroupBox,
        Header,
        Item,
        ItemRow,
        ItemBranchIndicator,
        Splitter,
        Menu,
        MenuItem,
        Widget,
        StatusBar,
        ScrollAreaCorner,
        MacHelpButton,
        MenuBar,
        MenuBarItem,
        DelayButton,
    };

    void paint(QPainter *);

    bool sunken() const
    {
        return m_sunken;
    }
    bool raised() const
    {
        return m_raised;
    }
    bool flat() const
    {
        return m_flat;
    }
    bool active() const
    {
        return m_active;
    }
    bool selected() const
    {
        return m_selected;
    }
    bool hasFocus() const
    {
        return m_focus;
    }
    bool on() const
    {
        return m_on;
    }
    bool hover() const
    {
        return m_hover;
    }
    bool horizontal() const
    {
        return m_horizontal;
    }
    bool isTransient() const
    {
        return m_transient;
    }

    int minimum() const
    {
        return m_minimum;
    }
    int maximum() const
    {
        return m_maximum;
    }
    int step() const
    {
        return m_step;
    }
    int value() const
    {
        return m_value;
    }
    int paintMargins() const
    {
        return m_paintMargins;
    }

    QString elementType() const
    {
        return m_type;
    }
    QString text() const
    {
        return m_text;
    }
    QString activeControl() const
    {
        return m_activeControl;
    }
    QVariantMap hints() const
    {
        return m_hints;
    }
    QVariantMap properties() const
    {
        return m_properties;
    }
    QFont font() const
    {
        return m_font;
    }
    QString styleName() const;

    void setSunken(bool sunken)
    {
        if (m_sunken != sunken) {
            m_sunken = sunken;
            polish();
            Q_EMIT sunkenChanged();
        }
    }
    void setRaised(bool raised)
    {
        if (m_raised != raised) {
            m_raised = raised;
            polish();
            Q_EMIT raisedChanged();
        }
    }
    void setFlat(bool flat)
    {
        if (m_flat != flat) {
            m_flat = flat;
            polish();
            Q_EMIT flatChanged();
        }
    }
    void setActive(bool active)
    {
        if (m_active != active) {
            m_active = active;
            polish();
            Q_EMIT activeChanged();
        }
    }
    void setSelected(bool selected)
    {
        if (m_selected != selected) {
            m_selected = selected;
            polish();
            Q_EMIT selectedChanged();
        }
    }
    void setHasFocus(bool focus)
    {
        if (m_focus != focus) {
            m_focus = focus;
            polish();
            Q_EMIT hasFocusChanged();
        }
    }
    void setOn(bool on)
    {
        if (m_on != on) {
            m_on = on;
            polish();
            Q_EMIT onChanged();
        }
    }
    void setHover(bool hover)
    {
        if (m_hover != hover) {
            m_hover = hover;
            polish();
            Q_EMIT hoverChanged();
        }
    }
    void setHorizontal(bool horizontal)
    {
        if (m_horizontal != horizontal) {
            m_horizontal = horizontal;
            updateSizeHint();
            polish();
            Q_EMIT horizontalChanged();
        }
    }
    void setTransient(bool transient)
    {
        if (m_transient != transient) {
            m_transient = transient;
            polish();
            Q_EMIT transientChanged();
        }
    }
    void setMinimum(int minimum)
    {
        if (m_minimum != minimum) {
            m_minimum = minimum;
            polish();
            Q_EMIT minimumChanged();
        }
    }
    void setMaximum(int maximum)
    {
        if (m_maximum != maximum) {
            m_maximum = maximum;
            polish();
            Q_EMIT maximumChanged();
        }
    }
    void setValue(int value)
    {
        if (m_value != value) {
            m_value = value;
            polish();
            Q_EMIT valueChanged();
        }
    }
    void setStep(int step)
    {
        if (m_step != step) {
            m_step = step;
            polish();
            Q_EMIT stepChanged();
        }
    }
    void setPaintMargins(int value)
    {
        if (m_paintMargins != value) {
            m_paintMargins = value;
            polish();
            Q_EMIT paintMarginsChanged();
        }
    }
    void setElementType(const QString &str);
    void setText(const QString &str)
    {
        if (m_text != str) {
            m_text = str;
            updateSizeHint();
            polish();
            Q_EMIT textChanged();
        }
    }
    void setActiveControl(const QString &str)
    {
        if (m_activeControl != str) {
            m_activeControl = str;
            // Slider is the only type whose size hints react to active control changes.
            switch (m_itemType) {
            case Slider:
                updateSizeHint();
                break;
            default:
                break;
            }
            polish();
            Q_EMIT activeControlChanged();
        }
    }
    void setHints(const QVariantMap &hints);
    void setProperties(const QVariantMap &props)
    {
        if (m_properties != props) {
            m_properties = props;
            m_iconDirty = true;
            updateSizeHint();
            polish();
            Q_EMIT propertiesChanged();
        }
    }
    void setFont(const QFont &font);
    void resetHints();
    void resetFont();

    int contentWidth() const
    {
        return m_contentWidth;
    }
    void setContentWidth(int arg);

    int contentHeight() const
    {
        return m_contentHeight;
    }
    void setContentHeight(int arg);

    void initStyleOption();
    void resolvePalette();

    int topPadding() const;
    int leftPadding() const;
    int rightPadding() const;
    int bottomPadding() const;

    Q_INVOKABLE qreal textWidth(const QString &);
    Q_INVOKABLE qreal textHeight(const QString &);

    int textureWidth() const
    {
        return m_textureWidth;
    }
    void setTextureWidth(int w);

    int textureHeight() const
    {
        return m_textureHeight;
    }
    void setTextureHeight(int h);

    QQuickItem *control() const;
    void setControl(QQuickItem *control);

    static QStyle *style();

public Q_SLOTS:
    int pixelMetric(const QString &);
    QVariant styleHint(const QString &);
    void updateSizeHint();
    void updateRect();
    void updateBaselineOffset();
    void updateItem()
    {
        polish();
    }
    QString hitTest(int x, int y);
    QRectF subControlRect(const QString &subcontrolString);
    QSize sizeFromContents(int width, int height);
    QRect computeBoundingRect(const QList<QRect>& rects);
    QString elidedText(const QString &text, int elideMode, int width);
    bool hasThemeIcon(const QString &) const;

private Q_SLOTS:
    void updateFocusReason();

Q_SIGNALS:
    void elementTypeChanged();
    void textChanged();
    void sunkenChanged();
    void raisedChanged();
    void flatChanged();
    void activeChanged();
    void selectedChanged();
    void hasFocusChanged();
    void onChanged();
    void hoverChanged();
    void horizontalChanged();
    void transientChanged();
    void minimumChanged();
    void maximumChanged();
    void stepChanged();
    void valueChanged();
    void activeControlChanged();
    void styleNameChanged();
    void paintMarginsChanged();
    void hintChanged();
    void propertiesChanged();
    void fontChanged();
    void controlChanged();

    void contentWidthChanged(int arg);
    void contentHeightChanged(int arg);

    void textureWidthChanged(int w);
    void textureHeightChanged(int h);

    void leftPaddingChanged();
    void topPaddingChanged();
    void rightPaddingChanged();
    void bottomPaddingChanged();

protected:
    bool event(QEvent *) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void updatePolish() override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value) override;

    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    int padding(Qt::Edge edge) const;
    QIcon iconFromIconProperty() const;
    const char *classNameForItem() const;
    int handleWidth() const;
    qreal baselineOffset() const;
    void styleChanged();

protected:
    Kirigami::Platform::PlatformTheme *m_theme = nullptr;
    QStyleOption *m_styleoption;
    QPointer<QQuickItem> m_control;
    QPointer<QWindow> m_window;
    Type m_itemType;

    QString m_type;
    QString m_text;
    QString m_activeControl;
    QVariantMap m_hints;
    QVariantMap m_properties;
    QFont m_font;

    bool m_sunken;
    bool m_raised;
    bool m_flat;
    bool m_active;
    bool m_selected;
    bool m_focus;
    bool m_hover;
    bool m_on;
    bool m_horizontal;
    bool m_transient;
    bool m_sharedWidget;
    bool m_iconDirty = true;

    int m_minimum;
    int m_maximum;
    int m_value;
    int m_step;
    int m_paintMargins;

    int m_contentWidth;
    int m_contentHeight;

    int m_textureWidth;
    int m_textureHeight;

    QMetaProperty m_focusReasonProperty;
    Qt::FocusReason m_focusReason;

    QImage m_image;
    KQuickPadding m_border;

    static std::unique_ptr<QStyle> s_style;
};

#endif // QQUICKSTYLEITEM_P_H
