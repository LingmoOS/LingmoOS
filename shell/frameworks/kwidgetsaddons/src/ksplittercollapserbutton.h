/*
    SPDX-FileCopyrightText: 2014 Montel Laurent <montel@kde.org>
    based on code:
    SPDX-FileCopyrightText: 2009 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KSPLITTERCOLLAPSERBUTTON_H
#define KSPLITTERCOLLAPSERBUTTON_H

// Qt
#include <QToolButton>
#include <kwidgetsaddons_export.h>
#include <memory>

class QSplitter;

/**
 * @class KSplitterCollapserButton ksplittercollapserbutton.h KSplitterCollapserButton
 *
 * A button which appears on the side of a splitter handle and allows easy
 * collapsing of the widget on the opposite side
 * @since 5.5
 */
class KWIDGETSADDONS_EXPORT KSplitterCollapserButton : public QToolButton
{
    Q_OBJECT
public:
    /**
     * @brief KSplitterCollapserButton create a splitter collapser
     * @param childWidget the widget, child of the splitter, whose size is controlled by this collapser
     * @param splitter the splitter which this collapser should be associated with.
     */
    explicit KSplitterCollapserButton(QWidget *childWidget, QSplitter *splitter);

    /**
     * Destructor
     */
    ~KSplitterCollapserButton() override;

    /**
     * @brief isWidgetCollapsed
     * @return true if splitter is collapsed.
     */
    bool isWidgetCollapsed() const;

    QSize sizeHint() const override;

public Q_SLOTS:
    /**
     * @brief collapse, this function collapses the splitter if splitter is not collapsed.
     */
    void collapse();
    /**
     * @brief restore, call this function to restore previous splitter position.
     */
    void restore();
    /**
     * @brief setCollapsed, this function allows to collapse or not the splitter.
     * @param collapsed if the splitter should be collapsed
     */
    void setCollapsed(bool collapsed);

private Q_SLOTS:
    KWIDGETSADDONS_NO_EXPORT void slotClicked();

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    std::unique_ptr<class KSplitterCollapserButtonPrivate> const d;
};

#endif /* KSPLITTERCOLLAPSERBUTTON_H */
