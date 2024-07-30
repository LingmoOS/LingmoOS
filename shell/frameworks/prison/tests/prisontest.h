#ifndef prison_H
#define prison_H

#include <QWidget>

class BarcodeExampleWidget;

class QLineEdit;
class main_window : public QWidget
{
    Q_OBJECT
public:
    main_window();
public Q_SLOTS:
    void data_changed();

private:
    QLineEdit *m_lineedit = nullptr;
    BarcodeExampleWidget *m_dmw = nullptr;
    BarcodeExampleWidget *m_qrw = nullptr;
    BarcodeExampleWidget *m_39w = nullptr;
    BarcodeExampleWidget *m_93w = nullptr;
    BarcodeExampleWidget *m_dmcolor = nullptr;
    BarcodeExampleWidget *m_qrcolor = nullptr;
    BarcodeExampleWidget *m_39color = nullptr;
    BarcodeExampleWidget *m_93color = nullptr;
    BarcodeExampleWidget *m_nullw = nullptr;
};

#endif // prison_H
