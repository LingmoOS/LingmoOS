#ifndef KICONEFFECTTEST_H
#define KICONEFFECTTEST_H

#include <QImage>
#include <QScrollArea>

class QLabel;

class KIconEffectTestWidget : public QScrollArea
{
    Q_OBJECT
public:
    KIconEffectTestWidget(QWidget *parent = nullptr);
private Q_SLOTS:
    void slotGray(int);
    void slotMonochrome(int);
    void slotDesaturate(int);
    void slotGamma(int);
    void slotColorizeColor(const QColor &);
    void slotColorizeValue(int);

private:
    QImage img;
    QLabel *lbl[6];
    QColor colorizedColor;
    float colorizedValue;
};

#endif
