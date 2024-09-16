// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfiledragserver.h"
#include "dfiledrag.h"

#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <QMimeData>
#include <QApplication>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QDebug>
#include <QRandomGenerator>

DGUI_USE_NAMESPACE

static int p;
static DFileDragServer *s = nullptr;
static QLabel *lbr = nullptr;

class DraggableLabel : public QLabel
{
    Q_OBJECT
public:
    using QLabel::QLabel;

Q_SIGNALS:
    void dragFinished();

protected:
    void mousePressEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::MouseButton::LeftButton) {
            dragpos = e->pos();
        }
    }

    void mouseMoveEvent(QMouseEvent *e)
    {
        if (!(e->buttons() & Qt::MouseButton::LeftButton) || s) {
            return;
        }
        if ((e->pos() - dragpos).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }

        s = new DFileDragServer();
        DFileDrag *drag = new DFileDrag(this, s);
        QMimeData *m = new QMimeData();
        m->setText("your stuff here");
        drag->setMimeData(m);

        connect(drag, &DFileDrag::targetUrlChanged, [drag] {
            lbr->setText(drag->targetUrl().toString());
        });

        Qt::DropAction res = drag->exec(Qt::MoveAction);
        if (res!= Qt::IgnoreAction)
            Q_EMIT dragFinished();
        else {
            s->deleteLater();
            s = nullptr;
        }
    }

private:
    QPoint dragpos;
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    QDialog d;
    d.show();

    DraggableLabel l("Drag me", &d);
    l.setAlignment(Qt::AlignCenter);

    QTimer t;
    t.setInterval(50);
    t.setSingleShot(false);

    QProgressBar pg;
    pg.setMinimum(0);
    pg.setMaximum(100);

    QLabel lb;
    lbr = &lb;
    lb.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QObject::connect(&l, &DraggableLabel::dragFinished, [&t] {
        p = -1;
        t.start();
    });

    QObject::connect(&t, &QTimer::timeout, [&t, &pg] {
        if (QRandomGenerator::global()->generate() & 1) {
            s->setProgress(++p);
            pg.setValue(p);
            if(p == 100) {
                t.stop();
                s->deleteLater();
                s = nullptr;
            }
        }
    });

    QVBoxLayout lo;
    lo.addWidget(&l);
    lo.addWidget(&pg);
    lo.addWidget(&lb);
    d.setLayout(&lo);

    a.exec();
}

#include "dnd-test-server.moc"
