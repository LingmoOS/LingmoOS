// Copyright (C) 2024 rewine <luhongxu@lingmo.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QGuiApplication>
#include <QRasterWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "personalization_manager.h"

class TestWindow : public QRasterWindow
{
    Q_OBJECT
public:
    TestWindow()
        : m_manager(new PersonalizationManager)
        , rect1(50, 50, 200, 50)
        , rect2(50, 200, 200, 50)
        , rect3(50, 350, 200, 50)
        , rect4(50, 500, 200, 50)
    {
        setTitle(tr("C++ Client"));

        connect(m_manager, &PersonalizationManager::activeChanged, this, [this] {
            qDebug() << "personalization manager active changed";

            if (m_manager->isActive()) {
                cursor_context = new PersonalizationCursor(m_manager->get_cursor_context());
            }
        });
    }

    ~TestWindow()
    {
        if (m_manager != nullptr)
        {
            delete m_manager;
            m_manager = nullptr;
        }
    }

    void mousePressEvent(QMouseEvent *ev) override
    {
        if (cursor_context == nullptr)
            return;

        if (rect1.contains(ev->position())) {
            QString config = "test_config.ini";
            QSettings settings(config, QSettings::IniFormat);
            QString theme = settings.value("Cursor/theme", "Breeze_Light").toString();
            qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << theme;
            cursor_context->set_theme(theme);
            cursor_context->commit();
        }
        else if (rect2.contains(ev->position())) {
            QString config = "test_config.ini";
            QSettings settings(config, QSettings::IniFormat);
            int cursor_size = settings.value("Cursor/size", 32).toInt();
            qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << cursor_size;
            cursor_context->set_size(cursor_size);
            cursor_context->commit();
        }
        else if (rect3.contains(ev->position()))
            cursor_context->get_theme();
        else if (rect4.contains(ev->position()))
            cursor_context->get_size();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setFont(m_font);
        p.fillRect(QRect(0, 0, width(), height()), Qt::gray);
        p.fillRect(rect1, QColor::fromString("#C0FFEE"));
        p.drawText(rect1, Qt::TextWordWrap, "set cursor theme");
        p.fillRect(rect2, QColor::fromString("#decaff"));
        p.drawText(rect2, Qt::TextWordWrap, "set cursor size");
        p.fillRect(rect3, QColor::fromString("#7EA"));
        p.drawText(rect3, Qt::TextWordWrap, "get cursor theme");
        p.fillRect(rect4, QColor::fromString("#7EB"));
        p.drawText(rect4, Qt::TextWordWrap, "get cursor size");
    }

private:
    PersonalizationManager *m_manager = nullptr;
    PersonalizationCursor* cursor_context = nullptr;

    QRectF rect1;
    QRectF rect2;
    QRectF rect3;
    QRectF rect4;
    QFont m_font;
};

int main (int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    TestWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
