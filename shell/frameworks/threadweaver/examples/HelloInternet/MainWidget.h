#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QImage>
#include <QLabel>
#include <QWidget>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

protected:
    void resizeEvent(QResizeEvent *) override;

public Q_SLOTS:
    void setImage(QImage image);
    void setCaption(QString text);
    void setStatus(QString text);

private:
    QLabel *m_image;
    QLabel *m_caption;
    QLabel *m_status;
};

#endif // MAINWIDGET_H
