#ifndef DateTimeEdit_H
#define DateTimeEdit_H
#include <QDateTimeEdit>

class DateTimeEdit : public QDateTimeEdit {
    Q_OBJECT
public :
    DateTimeEdit(QWidget *parent = nullptr);
    ~DateTimeEdit();
    QPixmap loadSvg(const QString &path, int size);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    bool hoverFlag = false;
    bool focusFlag = false;
Q_SIGNALS:
    void changeDate();
};

#endif // DateTimeEdit_H
