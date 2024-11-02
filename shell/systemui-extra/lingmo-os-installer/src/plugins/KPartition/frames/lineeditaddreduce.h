#ifndef LINEEDITADDREDUCE_H
#define LINEEDITADDREDUCE_H

#include <QObject>
#include <QLineEdit>
#include <QWidget>
#include <QPushButton>
namespace KInstaller {

class LineEditAddReduce : public QWidget
{
    Q_OBJECT
public:
    explicit LineEditAddReduce( QWidget* parent = nullptr );
    ~LineEditAddReduce();
    void initUI();
    void initAllConnect();
    void addStyleSheet();
    qint64 getValue();
    void setValue(QString sizestr);
public slots:
    void clickAddBtn();
    void clickReduceBtn();
    void getTextChanged();
signals:
    void signalAdd();
    void signalReduce();
protected:
    bool eventFilter(QObject *object, QEvent *event);
public:

    QPushButton *sizeAddBtn, *sizeReduceBtn;
    QString m_sizeStr;
    QLineEdit* m_ledit;
    qint64 sizevalue, maxSizevalue;
};

}
#endif // LINEEDITADDREDUCE_H
