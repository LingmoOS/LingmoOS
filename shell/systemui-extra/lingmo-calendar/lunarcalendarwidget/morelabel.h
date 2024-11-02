#ifndef SHOWMORELABEL_H
#define SHOWMORELABEL_H

#include <QLabel>
#include <QPushButton>
#include <QGSettings>
class ShowMoreLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ShowMoreLabel(QWidget *parent = nullptr);
    ~ShowMoreLabel();
    QString  m_text;
//    QGSettings *StyleSetting = nullptr;
    void settext(const QString &text);
    QColor color1;
signals:
    void clicked();
    void backClicked();
protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // SHOWMORELABEL_H
