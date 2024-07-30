/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "window.h"

#include "ui_window.h"

#include <QItemDelegate>
#include <QPainter>

#include <KX11Extras>

class Delegate : public QItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();

        const QString title = index.data().toString();

        QRect titleRect = painter->fontMetrics().boundingRect(title);
        // unused int lineHeight = titleRect.height();

        // Header background
        auto rect = option.rect;
        rect.setHeight(64);
        titleRect.moveTop(option.rect.top());
        titleRect.setWidth(option.rect.width());

        if (index.data(KActivities::ActivitiesModel::ActivityIsCurrent).toBool()) {
            painter->fillRect(rect, QColor(64, 64, 64));
        } else {
            painter->fillRect(rect, QColor(32, 32, 32));
        }

        // Painting the title
        painter->setPen(QColor(255, 255, 255));

        titleRect.moveTop(titleRect.top() + 8);
        titleRect.setLeft(64 + 8);
        titleRect.setWidth(titleRect.width() - 64 - 8);
        painter->drawText(titleRect, title);

        titleRect.moveTop(titleRect.bottom() + 16);

        const QString description = index.data(KActivities::ActivitiesModel::ActivityDescription).toString();

        if (!description.isEmpty()) {
            painter->drawText(titleRect, index.data(KActivities::ActivitiesModel::ActivityDescription).toString());
        } else {
            painter->setPen(QColor(128, 128, 128));
            painter->drawText(titleRect, index.data(KActivities::ActivitiesModel::ActivityId).toString());
        }

        const QString iconName = index.data(KActivities::ActivitiesModel::ActivityIconSource).toString();

        if (!iconName.isEmpty()) {
            painter->drawPixmap(option.rect.x(), option.rect.y(), QIcon::fromTheme(iconName).pixmap(64, 64));
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return QSize(0, 70);
    }
};

Window::Window()
    : ui(new Ui::MainWindow())
    , activities(new KActivities::Consumer(this))
    , modelRunningActivities(new KActivities::ActivitiesModel({KActivities::Info::Running, KActivities::Info::Stopping}, this))
    , modelStoppedActivities(new KActivities::ActivitiesModel({KActivities::Info::Stopped, KActivities::Info::Starting}, this))
{
    ui->setupUi(this);

    modelRunningActivities->setObjectName(QStringLiteral("RUNNING"));
    ui->listRunningActivities->setModel(modelRunningActivities);
    ui->listRunningActivities->setItemDelegate(new Delegate());

    modelStoppedActivities->setObjectName(QStringLiteral("STOPPED"));
    ui->listStoppedActivities->setModel(modelStoppedActivities);
    ui->listStoppedActivities->setItemDelegate(new Delegate());

    qDebug() << connect(activities, &KActivities::Consumer::runningActivitiesChanged, this, [](const QStringList &running) {
        qDebug() << running;
    });
}

void Window::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    KX11Extras::self()->setOnActivities(effectiveWinId(), QStringList());
    KX11Extras::self()->setOnAllDesktops(effectiveWinId(), true);
}

Window::~Window()
{
    delete ui;
}

#include "moc_window.cpp"
