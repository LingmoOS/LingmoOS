#ifndef _K_PIXMAPSEQUENCE_OVERLAY_PAINTER_TEST_H_
#define _K_PIXMAPSEQUENCE_OVERLAY_PAINTER_TEST_H_

#include <QWidget>

class KPixmapSequenceOverlayPainter;
class QComboBox;
class QSpinBox;
class QEvent;

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    TestWidget();
    ~TestWidget() override;

    bool eventFilter(QObject *, QEvent *) override;

private Q_SLOTS:
    void alignementChanged(int);
    void offsetChanged();

private:
    KPixmapSequenceOverlayPainter *m_painter;

    QWidget *m_widget;
    QComboBox *m_alignment;
    QSpinBox *m_offsetX;
    QSpinBox *m_offsetY;

    bool m_draggingLeft;
    bool m_draggingRight;
    bool m_draggingTop;
    bool m_draggingBottom;
};

#endif
