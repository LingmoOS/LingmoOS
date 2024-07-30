#ifndef KCOMPLETIONUITEST_H
#define KCOMPLETIONUITEST_H

#include <QStringList>
#include <QWidget>
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;
class QLabel;
class QListWidget;
class QPushButton;

class KHistoryComboBox;
class KLineEdit;

class Form1 : public QWidget
{
    Q_OBJECT

public:
    Form1(QWidget *parent = nullptr);
    ~Form1() override;

    QGroupBox *GroupBox1;
    QLabel *TextLabel1;
    KLineEdit *LineEdit1;
    QPushButton *PushButton1;
    QPushButton *PushButton1_4;
    QListWidget *ListBox1;
    QPushButton *PushButton1_3;
    QPushButton *PushButton1_2;

    KLineEdit *edit;
    KHistoryComboBox *combo;

protected Q_SLOTS:
    void slotList();
    void slotAdd();
    void slotRemove();
    void slotHighlighted(int);

protected:
    QStringList defaultItems() const;

    QVBoxLayout *Form1Layout;
    QVBoxLayout *GroupBox1Layout;
    QVBoxLayout *Layout9;
    QHBoxLayout *Layout1;
    QHBoxLayout *Layout2;
    QHBoxLayout *Layout3;
    QHBoxLayout *Layout8;
    QVBoxLayout *Layout7;
};

#endif // KCOMPLETIONUITEST_H
