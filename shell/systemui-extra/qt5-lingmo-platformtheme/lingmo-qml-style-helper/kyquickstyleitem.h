/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#ifndef KYQUICKSTYLEITEM_H
#define KYQUICKSTYLEITEM_H

#include "kyquickpadding_p.h"
#include <PlatformTheme>

#include <QQuickItem>
#include <QQuickImageProvider>

#include <QPixmap>
#include <QPointer>
#include <QImage>


class QStyle;
class QStyleOption;

class QQuickTableRowImageProvider1 : public QQuickImageProvider
{
public:
    QQuickTableRowImageProvider1()
        : QQuickImageProvider(QQuickImageProvider::Pixmap) {}
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};

class KyQuickStyleItem: public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(KyQuickPadding* border READ border CONSTANT)

    Q_PROPERTY( bool sunken READ sunken WRITE setSunken NOTIFY sunkenChanged)
    Q_PROPERTY( bool raised READ raised WRITE setRaised NOTIFY raisedChanged)
    Q_PROPERTY( bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY( bool selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY( bool hasFocus READ hasFocus WRITE sethasFocus NOTIFY hasFocusChanged)
    Q_PROPERTY( bool on READ on WRITE setOn NOTIFY onChanged)
    Q_PROPERTY( bool hover READ hover WRITE setHover NOTIFY hoverChanged)
    Q_PROPERTY( bool horizontal READ horizontal WRITE setHorizontal NOTIFY horizontalChanged)
    Q_PROPERTY( bool isTransient READ isTransient WRITE setTransient NOTIFY transientChanged)

    Q_PROPERTY( QString elementType READ elementType WRITE setElementType NOTIFY elementTypeChanged)
    Q_PROPERTY( QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY( QString activeControl READ activeControl WRITE setActiveControl NOTIFY activeControlChanged)
    Q_PROPERTY( QString styleName READ styleName NOTIFY styleNameChanged)
    Q_PROPERTY( QVariantMap hints READ hints WRITE setHints NOTIFY hintChanged RESET resetHints)
    Q_PROPERTY( QVariantMap properties READ properties WRITE setProperties NOTIFY propertiesChanged)
    Q_PROPERTY( QFont font READ font NOTIFY fontChanged)

    // For range controls
    Q_PROPERTY( int minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY( int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
    Q_PROPERTY( int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY( int step READ step WRITE setStep NOTIFY stepChanged)
    Q_PROPERTY( int paintMargins READ paintMargins WRITE setPaintMargins NOTIFY paintMarginsChanged)

    Q_PROPERTY( int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY( int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY( int textureWidth READ textureWidth WRITE setTextureWidth NOTIFY textureWidthChanged)
    Q_PROPERTY( int textureHeight READ textureHeight WRITE setTextureHeight NOTIFY textureHeightChanged)

    Q_PROPERTY( int leftPadding READ leftPadding NOTIFY leftPaddingChanged)
    Q_PROPERTY( int topPadding READ topPadding NOTIFY topPaddingChanged)
    Q_PROPERTY( int rightPadding READ rightPadding NOTIFY rightPaddingChanged)
    Q_PROPERTY( int bottomPadding READ bottomPadding NOTIFY bottomPaddingChanged)

    Q_PROPERTY( QString buttonType READ buttonType WRITE setbuttonType NOTIFY buttonTypeChanged)
    Q_PROPERTY( QString roundButton READ roundButton WRITE setroundButton NOTIFY roundButtonChanged)

    Q_PROPERTY( QQuickItem *control READ control WRITE setControl NOTIFY controlChanged)

    KyQuickPadding* border() { return &m_border; }

public:
    KyQuickStyleItem(QQuickItem *parent = nullptr);
    ~KyQuickStyleItem() override;

    enum MenuItemType {
        SeparatorType = 0,
        ItemType,
        MenuType,
        ScrollIndicatorType
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
        MenuBarItem
    };

    void paint(QPainter *);

    bool sunken() const { return m_sunken; }
    bool raised() const { return m_raised; }
    bool active() const { return m_active; }
    bool selected() const { return m_selected; }
    bool hasFocus() const { return m_focus; }
    bool on() const { return m_on; }
    bool hover() const { return m_hover; }
    bool horizontal() const { return m_horizontal; }
    bool isTransient() const { return m_transient; }

    int minimum() const { return m_minimum; }
    int maximum() const { return m_maximum; }
    int step() const { return m_step; }
    int value() const { return m_value; }
    int paintMargins() const { return m_paintMargins; }

    QString elementType() const { return m_type; }
    QString text() const { return m_text; }
    QString activeControl() const { return m_activeControl; }
    QVariantMap hints() const { return m_hints; }
    QVariantMap properties() const { return m_properties; }
    QFont font() const { return m_font;}
    QString styleName() const;

    void setSunken(bool sunken) { if (m_sunken != sunken) {m_sunken = sunken; emit sunkenChanged();}}
    void setRaised(bool raised) { if (m_raised!= raised) {m_raised = raised; emit raisedChanged();}}
    void setActive(bool active) { if (m_active!= active) {m_active = active; emit activeChanged();}}
    void setSelected(bool selected) { if (m_selected!= selected) {m_selected = selected; emit selectedChanged();}}
    void sethasFocus(bool focus) { if (m_focus != focus) {m_focus = focus; emit hasFocusChanged();}}
    void setOn(bool on) { if (m_on != on) {m_on = on ; emit onChanged();}}
    void setHover(bool hover) { if (m_hover != hover) {m_hover = hover ; emit hoverChanged();}}
    void setHorizontal(bool horizontal) { if (m_horizontal != horizontal) {m_horizontal = horizontal; emit horizontalChanged();}}
    void setTransient(bool transient) { if (m_transient != transient) {m_transient = transient; emit transientChanged();}}
    void setMinimum(int minimum) { if (m_minimum!= minimum) {m_minimum = minimum; emit minimumChanged();}}
    void setMaximum(int maximum) { if (m_maximum != maximum) {m_maximum = maximum; emit maximumChanged();}}
    void setValue(int value) { if (m_value!= value) {m_value = value; emit valueChanged();}}
    void setStep(int step) { if (m_step != step) { m_step = step; emit stepChanged(); }}
    void setPaintMargins(int value) { if (m_paintMargins!= value) {m_paintMargins = value; emit paintMarginsChanged(); } }
    void setElementType(const QString &str);
    void setText(const QString &str) { if (m_text != str) {m_text = str; emit textChanged();}}
    void setActiveControl(const QString &str) { if (m_activeControl != str) {m_activeControl = str; emit activeControlChanged();}}
    void setHints(const QVariantMap &str);
    void setProperties(const QVariantMap &props) { if (m_properties != props) { m_properties = props; emit propertiesChanged(); } }
    void resetHints();

    int contentWidth() const { return m_contentWidth; }
    void setContentWidth(int arg);

    int contentHeight() const { return m_contentHeight; }
    void setContentHeight(int arg);

    virtual void initStyleOption ();
    void resolvePalette();

    int leftPadding() const;
    int topPadding() const;
    int rightPadding() const;
    int bottomPadding() const;

    Q_INVOKABLE qreal textWidth(const QString &);
    Q_INVOKABLE qreal textHeight(const QString &);

    int textureWidth() const { return m_textureWidth; }
    void setTextureWidth(int w);

    int textureHeight() const { return m_textureHeight; }
    void setTextureHeight(int h);

    QQuickItem *control() const;
    void setControl(QQuickItem *control);

    static QStyle *style();

    QString buttonType() const { return m_buttonType;}
    void setbuttonType(QString buttonType) {
            m_buttonType = buttonType ;
            emit buttonTypeChanged();
    }

    QString roundButton() const { return m_roundButton;}
    void setroundButton(QString roundButton) {
            m_roundButton = roundButton ;
            emit roundButtonChanged();
    }

public Q_SLOTS:
    int pixelMetric(const QString&);
    QVariant styleHint(const QString&);
    void updateSizeHint();
    void updateRect();
    void updateBaselineOffset();
    void updateItem(){polish();}
    QString hitTest(int x, int y);
    QRectF subControlRect(const QString &subcontrolString);
    QString elidedText(const QString &text, int elideMode, int width);
    bool hasThemeIcon(const QString &) const;

Q_SIGNALS:
    void elementTypeChanged();
    void textChanged();
    void sunkenChanged();
    void raisedChanged();
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
    void infoChanged();
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

    void buttonTypeChanged();
    void roundButtonChanged();

protected:
    bool event(QEvent *) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void updatePolish() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    const char* classNameForItem() const;
    QSize sizeFromContents(int width, int height);
    qreal baselineOffset();
    void styleChanged();

protected:
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
    bool m_active;
    bool m_selected;
    bool m_focus;
    bool m_hover;
    bool m_on;
    bool m_horizontal;
    bool m_transient;
    bool m_sharedWidget;

    int m_minimum;
    int m_maximum;
    int m_value;
    int m_step;
    int m_paintMargins;

    int m_contentWidth;
    int m_contentHeight;

    int m_textureWidth;
    int m_textureHeight;

    Qt::FocusReason m_lastFocusReason;

    QImage m_image;
    KyQuickPadding m_border;

    static QStyle *s_style;

    QString m_buttonType;
    QString m_roundButton;
};

#endif // KYQUICKSTYLEITEM_H
