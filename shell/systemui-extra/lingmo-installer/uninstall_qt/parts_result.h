#ifndef PARTS_RESULT_H
#define PARTS_RESULT_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class parts_result : public QWidget
{
   Q_OBJECT

public:
   explicit parts_result(int , QWidget *parent = nullptr);
   void init();
   void create_interface(int);
   QLabel *p_result_txt;

private:
   QLabel *p_icon;
   QHBoxLayout *p_hlayout_1;

signals:

public slots:
};

#endif // PARTS_RESULT_H
