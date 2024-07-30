/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007-2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QAbstractItemView>
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QRadialGradient>
#include <QStringListModel>
#include <QToolButton>

#include <kwidgetitemdelegate.h>

#define HARDCODED_BORDER 10
#define EQUALLY_SIZED_TOOLBUTTONS 1

#if EQUALLY_SIZED_TOOLBUTTONS
#include <QStyleOptionToolButton>
#endif

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    TestWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMouseTracking(true);
        setAttribute(Qt::WA_Hover);
    }

    ~TestWidget() override
    {
    }

    QSize sizeHint() const override
    {
        return QSize(30, 30);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter p(this);

        QRadialGradient radialGrad(QPointF(event->rect().width() / 2, event->rect().height() / 2), qMin(event->rect().width() / 2, event->rect().height() / 2));

        if (underMouse()) {
            radialGrad.setColorAt(0, Qt::green);
        } else {
            radialGrad.setColorAt(0, Qt::red);
        }

        radialGrad.setColorAt(1, Qt::transparent);

        p.fillRect(event->rect(), radialGrad);

        p.setPen(Qt::black);
        p.drawLine(0, 15, 30, 15);
        p.drawLine(15, 0, 15, 30);

        p.end();
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->pos().x() > 15 //
                && mouseEvent->pos().y() < 15) {
                qDebug() << "First quarter";
            } else if (mouseEvent->pos().x() < 15 //
                       && mouseEvent->pos().y() < 15) {
                qDebug() << "Second quarter";
            } else if (mouseEvent->pos().x() < 15 //
                       && mouseEvent->pos().y() > 15) {
                qDebug() << "Third quarter";
            } else {
                qDebug() << "Forth quarter";
            }
        }

        return QWidget::event(event);
    }
};

class MyDelegate : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    MyDelegate(QAbstractItemView *itemView, QObject *parent = nullptr)
        : KWidgetItemDelegate(itemView, parent)
    {
        for (int i = 0; i < 100; i++) {
            installed[i] = i % 5;
        }
    }

    ~MyDelegate() override
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);

        return sizeHint();
    }

    QSize sizeHint() const
    {
        return QSize(600, 60);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(index);
        painter->save();

        itemView()->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);

        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        }

        painter->restore();
    }

    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override
    {
        Q_UNUSED(index);
        QPushButton *button = new QPushButton();
        QToolButton *toolButton = new QToolButton();

        setBlockedEventTypes(button, QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);

        setBlockedEventTypes(toolButton, QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);

        connect(button, SIGNAL(clicked(bool)), this, SLOT(mySlot()));
        connect(toolButton, SIGNAL(triggered(QAction *)), this, SLOT(mySlot2()));
        connect(toolButton, SIGNAL(clicked(bool)), this, SLOT(mySlot3()));

        return QList<QWidget *>() << button << new TestWidget() << new QLineEdit() << toolButton;
    }

    void updateItemWidgets(const QList<QWidget *> &widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const override
    {
        QPushButton *button = static_cast<QPushButton *>(widgets[0]);
        button->setText(QStringLiteral("Test me"));
        button->setIcon(QIcon::fromTheme(QStringLiteral("kde")));
        button->resize(button->sizeHint());
        button->move(HARDCODED_BORDER, sizeHint().height() / 2 - button->height() / 2);

        TestWidget *testWidget = static_cast<TestWidget *>(widgets[1]);

        testWidget->resize(testWidget->sizeHint());
        testWidget->move(2 * HARDCODED_BORDER + button->sizeHint().width(), //
                         sizeHint().height() / 2 - testWidget->size().height() / 2);

        // Hide the test widget when row can be divided by three
        testWidget->setVisible((index.row() % 3) != 0);

        QLineEdit *lineEdit = static_cast<QLineEdit *>(widgets[2]);

        lineEdit->setClearButtonEnabled(true);
        lineEdit->resize(lineEdit->sizeHint());
        lineEdit->move(3 * HARDCODED_BORDER + button->sizeHint().width() + testWidget->sizeHint().width(),
                       sizeHint().height() / 2 - lineEdit->size().height() / 2);

        QToolButton *toolButton = static_cast<QToolButton *>(widgets[3]);

        if (!toolButton->menu()) {
            QMenu *myMenu = new QMenu(toolButton);
            myMenu->addAction(QStringLiteral("Save"));
            myMenu->addAction(QStringLiteral("Load"));
            myMenu->addSeparator();
            myMenu->addAction(QStringLiteral("Close"));
            toolButton->setMenu(myMenu);
        }

        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolButton->setPopupMode(QToolButton::MenuButtonPopup);

        if (installed[index.row()]) {
            toolButton->setText(QStringLiteral("Uninstall"));
        } else {
            toolButton->setText(QStringLiteral("Install"));
        }

        toolButton->resize(toolButton->sizeHint());
#if EQUALLY_SIZED_TOOLBUTTONS
        QStyleOptionToolButton toolButtonOpt;
        toolButtonOpt.initFrom(toolButton);
        toolButtonOpt.features = QStyleOptionToolButton::MenuButtonPopup;
        toolButtonOpt.arrowType = Qt::DownArrow;
        toolButtonOpt.toolButtonStyle = Qt::ToolButtonTextBesideIcon;

        toolButtonOpt.text = "Install";
        int widthInstall = QApplication::style()
                               ->sizeFromContents(QStyle::CT_ToolButton,
                                                  &toolButtonOpt,
                                                  QSize(option.fontMetrics.boundingRect(QStringLiteral("Install")).width() + HARDCODED_BORDER * 3,
                                                        option.fontMetrics.height()),
                                                  toolButton)
                               .width();
        toolButtonOpt.text = "Uninstall";
        int widthUninstall = QApplication::style()
                                 ->sizeFromContents(QStyle::CT_ToolButton,
                                                    &toolButtonOpt,
                                                    QSize(option.fontMetrics.boundingRect(QStringLiteral("Uninstall")).width() + HARDCODED_BORDER * 3,
                                                          option.fontMetrics.height()),
                                                    toolButton)
                                 .width();

        QSize size = toolButton->sizeHint();
        size.setWidth(qMax(widthInstall, widthUninstall));
        toolButton->resize(size);
#endif
        toolButton->move(option.rect.width() - toolButton->size().width() - HARDCODED_BORDER, //
                         sizeHint().height() / 2 - toolButton->size().height() / 2);

        // Eat more space
        lineEdit->resize(option.rect.width() - toolButton->width() - testWidget->width() - button->width() - 5 * HARDCODED_BORDER, lineEdit->height());
    }

private Q_SLOTS:
    void mySlot()
    {
        QMessageBox::information(nullptr, QStringLiteral("Button clicked"), QStringLiteral("The button in row %1 was clicked").arg(focusedIndex().row()));
    }

    void mySlot2()
    {
        QMessageBox::information(nullptr,
                                 QStringLiteral("Toolbutton menu item clicked"),
                                 QStringLiteral("A menu item was triggered in row %1").arg(focusedIndex().row()));
    }

    void mySlot3()
    {
        bool isInstalled = installed[focusedIndex().row()];
        installed[focusedIndex().row()] = !isInstalled;
        const_cast<QAbstractItemModel *>(focusedIndex().model())->setData(focusedIndex(), QStringLiteral("makemodelbeupdated"));
    }

private:
    bool installed[100];
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(640, 480);
    QListView *listView = new QListView();
    QStringListModel *model = new QStringListModel();

    model->insertRows(0, 100);
    for (int i = 0; i < 100; ++i) {
        model->setData(model->index(i, 0), QString("Test " + QString::number(i)), Qt::DisplayRole);
    }

    listView->setModel(model);
    MyDelegate *myDelegate = new MyDelegate(listView);
    listView->setItemDelegate(myDelegate);
    listView->setVerticalScrollMode(QListView::ScrollPerPixel);

    mainWindow->setCentralWidget(listView);

    mainWindow->show();

    model->removeRows(0, 95);

    return app.exec();
}

#include "kwidgetitemdelegatetest.moc"
