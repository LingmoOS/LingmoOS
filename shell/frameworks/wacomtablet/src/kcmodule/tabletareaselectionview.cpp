/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tabletareaselectionview.h"

#include "ui_tabletareaselectionview.h"

#include <logging.h>

using namespace Wacom;

namespace Wacom
{
class TabletAreaSelectionViewPrivate
{
public:
    TabletAreaSelectionViewPrivate()
        : ui(new Ui::TabletAreaSelectionView)
    {
    }
    ~TabletAreaSelectionViewPrivate()
    {
        delete ui;
    }

    Ui::TabletAreaSelectionView *ui = nullptr;
}; // PRIVATE CLASS
} // NAMESPACE

TabletAreaSelectionView::TabletAreaSelectionView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new TabletAreaSelectionViewPrivate)
{
    setupUi();
}

TabletAreaSelectionView::~TabletAreaSelectionView()
{
    delete this->d_ptr;
}

const TabletArea TabletAreaSelectionView::getSelection() const
{
    Q_D(const TabletAreaSelectionView);
    return TabletArea(d->ui->areaWidget->getSelection());
}

void TabletAreaSelectionView::selectFullTablet()
{
    Q_D(TabletAreaSelectionView);

    setTabletAreaType(TabletAreaSelectionView::FullTabletArea);
    d->ui->areaWidget->clearSelection();
}

void TabletAreaSelectionView::selectPartOfTablet(const TabletArea &selection)
{
    Q_D(TabletAreaSelectionView);

    setTabletAreaType(TabletAreaSelectionView::PartialTabletArea);
    d->ui->areaWidget->setSelection(selection, true);
}

void TabletAreaSelectionView::select(QString output, bool isDesktop, const TabletArea &tabletSelection)
{
    Q_D(TabletAreaSelectionView);

    if (isDesktop) {
        // select full desktop
        d->ui->screenArea->clearSelection();
    } else {
        // select monitor
        d->ui->screenArea->setSelection(output);
    }

    if (isFullAreaSelection(tabletSelection)) {
        // full tablet selection
        selectFullTablet();
    } else {
        // part of tablet selection
        selectPartOfTablet(tabletSelection);
    }
}

void TabletAreaSelectionView::setTrackingModeWarning(bool doShow)
{
    Q_D(TabletAreaSelectionView);

    d->ui->warningIcon->setVisible(doShow);
    d->ui->warningLabel->setVisible(doShow);
}

void TabletAreaSelectionView::setupScreens(const QMap<QString, QRect> &screenGeometries, const QSize &widgetTargetSize)
{
    Q_D(TabletAreaSelectionView);

    // disable screen toggling by default
    d->ui->screenToggleButton->setEnabled(false);

    // setup screen area
    d->ui->screenArea->setEnabled(false);
    d->ui->screenArea->setWidgetTargetSize(widgetTargetSize);
    d->ui->screenArea->setFont(QFont(QLatin1String("sans"), 8));
    d->ui->screenArea->paintBelow = true;

    if (screenGeometries.count() > 0) {
        d->ui->screenArea->setDrawAreaCaptions(true);
        d->ui->screenArea->setDrawSelectionCaption(true);
        d->ui->screenArea->setAreas(screenGeometries, screenGeometries.keys());

        // allow screen toggling if we have more than one screen
        if (screenGeometries.count() > 1) {
            d->ui->screenToggleButton->setEnabled(true);
        }

    } else {
        // no valid parameters passed, draw error box
        d->ui->screenArea->setDrawAreaCaptions(true);
        d->ui->screenArea->setDrawSelectionCaption(false);
        d->ui->screenArea->setArea(QRect(0, 0, 1920, 1200), i18n("Internal Error"));
        // We call this intentionally with no screens from setupUI()
        qCWarning(KCM) << "Call to TabletAreaSelectionView::setupScreens made with no valid screens.";
    }

    // defaults to full selection
    d->ui->screenArea->clearSelection();
}

void TabletAreaSelectionView::setupTablet(const TabletArea &geometry, const QSize &widgetTargetSize)
{
    Q_D(TabletAreaSelectionView);

    d->ui->areaWidget->setWidgetTargetSize(widgetTargetSize);
    d->ui->areaWidget->setOutOfBoundsMargin(.1);

    if (geometry.isValid()) {
        QString caption = QString::fromLatin1("%1x%2").arg(geometry.width()).arg(geometry.height());

        d->ui->areaWidget->setDrawAreaCaptions(true);
        d->ui->areaWidget->setDrawSelectionCaption(true);
        d->ui->areaWidget->setArea(geometry, caption);

    } else {
        // draw error message
        d->ui->areaWidget->setDrawAreaCaptions(true);
        d->ui->areaWidget->setDrawSelectionCaption(false);
        d->ui->areaWidget->setArea(QRect(0, 0, 1920, 1200), i18n("Internal Error"));
        qCWarning(KCM) << "Internal error, invalid tablet geometry -" << geometry.toString();
    }

    // defaults to full selection
    setTabletAreaType(TabletAreaSelectionView::FullTabletArea);
}

void TabletAreaSelectionView::onCalibrateClicked()
{
    emit signalCalibrateClicked();
}

void TabletAreaSelectionView::onForceProportionsClicked()
{
    emit signalSetScreenProportions();
}

void TabletAreaSelectionView::onFullTabletSelected(bool checked)
{
    if (!checked) {
        return;
    }
    setTabletAreaType(TabletAreaSelectionView::FullTabletArea);
}

void TabletAreaSelectionView::onScreenToggle()
{
    emit signalScreenToggle();
}

void TabletAreaSelectionView::onTabletAreaSelected(bool checked)
{
    if (!checked) {
        return;
    }
    setTabletAreaType(TabletAreaSelectionView::PartialTabletArea);
}

void TabletAreaSelectionView::onLockProportionsToggled(bool enabled)
{
    Q_D(TabletAreaSelectionView);

    d->ui->areaWidget->lockProportions(enabled);
}

void TabletAreaSelectionView::setSelection(const TabletArea &selection)
{
    if (selection.isValid()) {
        if (isFullAreaSelection(selection)) {
            selectFullTablet();
        } else {
            selectPartOfTablet(selection);
        }
    } else {
        selectFullTablet();
    }
}

void TabletAreaSelectionView::setTabletAreaType(TabletAreaSelectionView::TabletAreaType type)
{
    Q_D(TabletAreaSelectionView);

    d->ui->fullTabletRadioButton->blockSignals(true);
    d->ui->tabletAreaRadioButton->blockSignals(true);

    if (type == TabletAreaSelectionView::FullTabletArea) {
        d->ui->fullTabletRadioButton->setChecked(true);
        d->ui->tabletAreaRadioButton->setChecked(false);

        d->ui->areaWidget->clearSelection();
        d->ui->areaWidget->setEnabled(false);

        emit signalFullTabletSelection();

    } else {
        d->ui->tabletAreaRadioButton->setChecked(true);
        d->ui->fullTabletRadioButton->setChecked(false);

        d->ui->areaWidget->setEnabled(true);

        emit signalTabletAreaSelection();
    }

    d->ui->fullTabletRadioButton->blockSignals(false);
    d->ui->tabletAreaRadioButton->blockSignals(false);
}

void TabletAreaSelectionView::onSelectionChanged()
{
    Q_D(TabletAreaSelectionView);

    const auto selection = d->ui->areaWidget->getSelection();

    d->ui->lineEditX->setText(QString::number(selection.x()));
    d->ui->lineEditY->setText(QString::number(selection.y()));
    d->ui->lineEditWidth->setText(QString::number(selection.width()));
    d->ui->lineEditHeight->setText(QString::number(selection.height()));
}

void TabletAreaSelectionView::onFineTuneValuesChanged(QString)
{
    Q_D(TabletAreaSelectionView);

    bool xvalid = true;
    bool yvalid = true;
    bool wvalid = true;
    bool hvalid = true;

    const QRect newSelection(d->ui->lineEditX->text().toInt(&xvalid),
                             d->ui->lineEditY->text().toInt(&yvalid),
                             d->ui->lineEditWidth->text().toInt(&wvalid),
                             d->ui->lineEditHeight->text().toInt(&hvalid));

    if (newSelection.isEmpty() || !newSelection.isValid() || !xvalid || !yvalid || !wvalid || !hvalid) {
        return;
    }

    d->ui->areaWidget->setSelection(newSelection, false);
}

bool TabletAreaSelectionView::isFullAreaSelection(const TabletArea &selection) const
{
    Q_D(const TabletAreaSelectionView);

    return (selection.isEmpty() || selection == d->ui->areaWidget->getVirtualArea());
}

void TabletAreaSelectionView::setupUi()
{
    Q_D(TabletAreaSelectionView);

    d->ui->setupUi(this);
    d->ui->iconLabel->setPixmap(QIcon::fromTheme(QLatin1String("help-about")).pixmap(QSize(16, 16)));

    d->ui->warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(QSize(16, 16)));
    d->ui->warningIcon->setVisible(true);
    d->ui->warningLabel->setVisible(false);

    // FIXME: signal-slot editor can't see this signal for some reason
    // TODO: rename areaWidget and screenArea, this is confusing
    connect(d->ui->areaWidget, &AreaSelectionWidget::selectionChanged, this, &TabletAreaSelectionView::onSelectionChanged);

    connect(d->ui->lineEditX, &QLineEdit::textChanged, this, &TabletAreaSelectionView::onFineTuneValuesChanged);
    connect(d->ui->lineEditY, &QLineEdit::textChanged, this, &TabletAreaSelectionView::onFineTuneValuesChanged);
    connect(d->ui->lineEditHeight, &QLineEdit::textChanged, this, &TabletAreaSelectionView::onFineTuneValuesChanged);
    connect(d->ui->lineEditWidth, &QLineEdit::textChanged, this, &TabletAreaSelectionView::onFineTuneValuesChanged);

    // Is this next call, like, fake?
    setupScreens(QMap<QString, QRect>(), QSize(200, 200));
    setupTablet(TabletArea(), QSize(400, 400));
}

#include "moc_tabletareaselectionview.cpp"
