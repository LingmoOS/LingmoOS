#ifndef WIDGET_H
#define WIDGET_H

#include <QMutex>
#include <QApplication>
#include <QClipboard>

#include "paddleocr-ncnn/paddleocr.h"

class Frame;
class QThread;
class QGridLayout;
class QHBoxLayout;
class ImageView;
class loadingWidget;
class QShortcut;

class Ocr : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString imgName READ imgName NOTIFY imgNameChanged)
    Q_PROPERTY(QString ocrText READ ocrText NOTIFY ocrTextChanged)

public:
    explicit Ocr(QWidget *parent = nullptr);
    ~Ocr();

    void setupUi();
    void setupConnect();
    void retranslateUi(QWidget *Widget);

    void createLoadingUi();
    void deleteLoadingUi();
//    void loadingUi();

    QString imgName() const;
    QString ocrText() const;

    Q_INVOKABLE bool openImage(const QString &path);
    Q_INVOKABLE void setTextType(const QString & text);
    Q_INVOKABLE void copy();

    void openImage(const QImage &img, const QString &name = "");

    void loadHtml(const QString &html);
    void loadString(const QString &string);
    void resultEmpty();

    //缩放显示label
    void initScaleLabel();

signals:
    void imgNameChanged();
    void ocrTextChanged();

private slots:
    void slotExport();

//    void change()
private:
    QString m_imgName;  //当前图片绝对路径
    QString m_imgPath = "";
    bool m_isLoading{false};

    QThread *m_loadImagethread{nullptr};
    QMutex m_mutex;
    QString m_result;
    QImage *m_currentImg{nullptr};

    QShortcut *m_scAddView = nullptr;
    QShortcut *m_scReduceView = nullptr;

    int m_isEndThread = 1;
signals:
    void sigResult(const QString &);

};

#endif // WIDGET_H
