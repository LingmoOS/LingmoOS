#ifndef __KLABEL_H__
#define __KLABEL_H__

#include <QLabel>

class KLabel : public QLabel
{
    Q_OBJECT
public:
    KLabel(QWidget *parent = nullptr);
    KLabel(QString strText, QWidget *parent = nullptr);

public slots:
    void setText(const QString &);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString m_strText;
};

#endif // __KLABEL_H__
