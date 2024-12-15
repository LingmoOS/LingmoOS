// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QEvent>
#include <QDebug>
#include <QDateTime>
#include <QMainWindow>
#include <QStatusBar>
#include <QMenuBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>

#include <DDciIcon>
#include <DDciIconPlayer>
#include <QDropEvent>
#include <QMimeData>

DGUI_USE_NAMESPACE

static QString iconModeToString(DDciIcon::Mode mode)
{
    switch (mode) {
    case DDciIcon::Normal:
        return "Normal";
    case DDciIcon::Hover:
        return "Hover";
    case DDciIcon::Pressed:
        return "Pressed";
    case DDciIcon::Disabled:
        return "Disabled";
    default:
        break;
    }

    return nullptr;
}

class IconWidget : public QMainWindow
{
public:
    IconWidget(const QString &iconName)
    {
        setCentralWidget(label = new QLabel(this));
        label->setAcceptDrops(true);
        centralWidget()->setAttribute(Qt::WA_MouseTracking);
        centralWidget()->installEventFilter(this);
        statusBar()->addWidget(message = new QLabel());

        auto normal = new QPushButton("Normal");
        connect(normal, &QPushButton::clicked, this, [this] {
            player.setMode(DDciIcon::Normal);
        });
        auto hover = new QPushButton("Hover");
        connect(hover, &QPushButton::clicked, this, [this] {
            player.setMode(DDciIcon::Hover);
        });
        auto pressed = new QPushButton("Pressed");
        connect(pressed, &QPushButton::clicked, this, [this] {
            player.setMode(DDciIcon::Pressed);
        });
        auto disabled = new QPushButton("Disabled");
        connect(disabled, &QPushButton::clicked, this, [this] {
            player.setMode(DDciIcon::Disabled);
        });

        auto open = new QPushButton("...");
        connect(open, &QPushButton::clicked, this, [this] {
            QString dciFile = QFileDialog::getOpenFileName(this, "select a dci icon file", "", "*.dci");
            player.setIcon(DDciIcon(dciFile));
            player.setMode(DDciIcon::Normal);
        });

        QWidget *menuWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(menuWidget);
        layout->addWidget(normal);
        layout->addWidget(hover);
        layout->addWidget(pressed);
        layout->addWidget(disabled);
        layout->addWidget(open);
        setMenuWidget(menuWidget);

        icon = DDciIcon::fromTheme(iconName);

        connect(&player, &DDciIconPlayer::updated, this, [this] {
            auto image = player.currentImage();
            label->setPixmap(QPixmap::fromImage(image));
        });
        connect(&player, &DDciIconPlayer::modeChanged, this, [this] (DDciIcon::Mode old, DDciIcon::Mode newMode) {
            message->setText(QString("Old State: %1, Current State: %2")
                             .arg(iconModeToString(old))
                             .arg(iconModeToString(newMode)));
        });

        DDciIconPalette p("pink");
        player.setPalette(p);
        player.setDevicePixelRatio(devicePixelRatioF());
        player.setIcon(icon);
    }

private:
    bool event(QEvent *event) override {
        if (event->type() == QEvent::WindowActivate) {
            player.play(DDciIcon::Normal);
        }
        return QMainWindow::event(event);
    }
    bool eventFilter(QObject *watched, QEvent *event) {
        if (watched != label)
            return QMainWindow::eventFilter(watched, event);

        if (event->type() == QEvent::Enter) {
            player.setMode(DDciIcon::Hover);
        } else if (event->type() == QEvent::Leave) {
            if (player.mode() == DDciIcon::Hover)
                player.setMode(DDciIcon::Normal);
        } else if (event->type() == QEvent::MouseButtonPress) {
            player.setMode(DDciIcon::Pressed);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            player.setMode(label->hasMouseTracking() ? DDciIcon::Hover : DDciIcon::Normal);
        } else if (event->type() == QEvent::DragEnter) {
            event->accept();
        } else if (event->type() == QEvent::Drop) {
            auto de = static_cast<QDropEvent *>(event);
            if (de->mimeData()->hasFormat("text/uri-list")) {
                QString uris = de->mimeData()->data("text/uri-list");
                QUrl url(uris.split("\r\n").value(0));

                DDciIcon icon(url.path());
                if (!icon.isNull()) {
                    player.setIcon(icon);
                    player.setMode(DDciIcon::Normal);
                }
            }
        } else {
            return false;
        }

        return true;
    }

    DDciIcon icon;
    DDciIconPlayer player;

    QLabel *message = nullptr;
    QLabel *label = nullptr;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    IconWidget *window = new IconWidget(QStringLiteral("test_heart"));

    window->setMinimumSize(300, 300);
    window->show();

    return app.exec();
}
