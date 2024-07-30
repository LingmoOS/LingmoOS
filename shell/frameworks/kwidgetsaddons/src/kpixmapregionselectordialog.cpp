/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpixmapregionselectordialog.h"

#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QScreen>
#include <QVBoxLayout>

#include <kpixmapregionselectorwidget.h>

class KPixmapRegionSelectorDialogPrivate
{
public:
    KPixmapRegionSelectorDialogPrivate(KPixmapRegionSelectorDialog *parent)
        : q(parent)
    {
    }

    KPixmapRegionSelectorWidget *pixmapSelectorWidget = nullptr;
    KPixmapRegionSelectorDialog *const q;

    void init()
    {
        //When the image is rotated we need to enforce the maximum width&height into the
        //KPixmapRegionSelectorWidget; in order to avoid the dialog to get out of the screen
        q->connect(pixmapSelectorWidget, &KPixmapRegionSelectorWidget::pixmapRotated, q, [this]() {
            adjustPixmapSize();
        });
    }

    void adjustPixmapSize()
    {
        if (pixmapSelectorWidget) {
            // Set maximum size for picture
            QScreen *screen = pixmapSelectorWidget->screen();
            if (screen) {
                const QRect screenGeometry = screen->availableGeometry();
                pixmapSelectorWidget->setMaximumWidgetSize((int)(screenGeometry.width() * 4.0 / 5), (int)(screenGeometry.height() * 4.0 / 5));
            }
        }
    }
};

KPixmapRegionSelectorDialog::KPixmapRegionSelectorDialog(QWidget *parent)
    : QDialog(parent)
    , d(new KPixmapRegionSelectorDialogPrivate(this))
{
    setWindowTitle(tr("Select Region of Image", "@title:window"));

    QVBoxLayout *boxLayout = new QVBoxLayout(this);

    QLabel *label = new QLabel(tr("Please click and drag on the image to select the region of interest:", "@label:chooser"), this);
    d->pixmapSelectorWidget = new KPixmapRegionSelectorWidget(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    boxLayout->addWidget(label);
    boxLayout->addWidget(d->pixmapSelectorWidget);
    boxLayout->addWidget(buttonBox);

    d->init();
}

KPixmapRegionSelectorDialog::~KPixmapRegionSelectorDialog() = default;

KPixmapRegionSelectorWidget *KPixmapRegionSelectorDialog::pixmapRegionSelectorWidget() const
{
    return d->pixmapSelectorWidget;
}

void KPixmapRegionSelectorDialog::adjustRegionSelectorWidgetSizeToFitScreen()
{
    d->adjustPixmapSize();
}

QRect KPixmapRegionSelectorDialog::getSelectedRegion(const QPixmap &pixmap, QWidget *parent)
{
    KPixmapRegionSelectorDialog dialog(parent);

    dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
    dialog.adjustRegionSelectorWidgetSizeToFitScreen();

    int result = dialog.exec();

    QRect rect;

    if (result == QDialog::Accepted) {
        rect = dialog.pixmapRegionSelectorWidget()->unzoomedSelectedRegion();
    }

    return rect;
}

QRect KPixmapRegionSelectorDialog::getSelectedRegion(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent)
{
    KPixmapRegionSelectorDialog dialog(parent);

    dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
    dialog.pixmapRegionSelectorWidget()->setSelectionAspectRatio(aspectRatioWidth, aspectRatioHeight);
    dialog.adjustRegionSelectorWidgetSizeToFitScreen();

    int result = dialog.exec();

    QRect rect;

    if (result == QDialog::Accepted) {
        rect = dialog.pixmapRegionSelectorWidget()->unzoomedSelectedRegion();
    }

    return rect;
}

QImage KPixmapRegionSelectorDialog::getSelectedImage(const QPixmap &pixmap, QWidget *parent)
{
    KPixmapRegionSelectorDialog dialog(parent);

    dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
    dialog.adjustRegionSelectorWidgetSizeToFitScreen();

    int result = dialog.exec();

    QImage image;

    if (result == QDialog::Accepted) {
        image = dialog.pixmapRegionSelectorWidget()->selectedImage();
    }

    return image;
}

QImage KPixmapRegionSelectorDialog::getSelectedImage(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent)
{
    KPixmapRegionSelectorDialog dialog(parent);

    dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
    dialog.pixmapRegionSelectorWidget()->setSelectionAspectRatio(aspectRatioWidth, aspectRatioHeight);
    dialog.adjustRegionSelectorWidgetSizeToFitScreen();

    int result = dialog.exec();

    QImage image;

    if (result == QDialog::Accepted) {
        image = dialog.pixmapRegionSelectorWidget()->selectedImage();
    }

    return image;
}

#include "moc_kpixmapregionselectordialog.cpp"
