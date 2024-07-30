/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KBUSYINDICATORWIDGET_H
#define KBUSYINDICATORWIDGET_H

#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

/**
 * @class KBusyIndicatorWidget kbusyindicatorwidget.h KBusyIndicatorWidget
 *
 * @brief Rotating spinning icon to indicate busyness
 *
 * When you need to communicate to the user that your application is busy with
 * something you'll want to use a KBusyIndicatorWidget to display an infinitely
 * spinnning indicator icon.
 *
 * A way of using this widget is to combine it with a QLabel to construct a
 * status line:
 *
 * ```
 * auto layout = new QHBoxLayout;
 * layout->addWidget(new KBusyIndicatorWidget);
 * layout->addWidget(new QLabel(QStringLiteral("Watering the flowers...")));
 * ```
 *
 * @image html kbusyindicatorwidget.png "KBusyIndicatorWidget with label"
 *
 * KBusyIndicatorWidget is set apart from KPixmapSequenceWidget in that it
 * does not render a pixmap sequence but rather animates a scaled Icon.
 * It can support multiple semi-arbitrary sizes and quality is only limited
 * by the resolution of available icons. It is also easier to use as its use
 * is more specific.
 *
 * @since 5.61.0
 */
class KWIDGETSADDONS_EXPORT KBusyIndicatorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KBusyIndicatorWidget(QWidget *parent = nullptr);
    ~KBusyIndicatorWidget() override;

    QSize minimumSizeHint() const override;

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    bool event(QEvent *event) override;

private:
    std::unique_ptr<class KBusyIndicatorWidgetPrivate> const d;
};

#endif // KBUSYINDICATORWIDGET_H
