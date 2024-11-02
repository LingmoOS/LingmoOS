#ifndef OCRRESULTWIDGET_H
#define OCRRESULTWIDGET_H

#include <QObject>
#include <QTextEdit>

class OCRResultWidget : public QTextEdit
{
    Q_OBJECT
public:
    OCRResultWidget(QWidget *parent = nullptr);
    //    void setAlignment(Qt::Alignment alignment)override;
public Q_SLOTS:

    void setFormatedText(QVector<QString>);
    void setFormatedText(QString);

private Q_SLOTS:

    void setThemeStyle();

private:
};

#endif // OCRRESULTWIDGET_H
