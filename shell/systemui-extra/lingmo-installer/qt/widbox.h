#ifndef WIDBOX_H
#define WIDBOX_H
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGSettings>


class widbox:public QWidget
{
    Q_OBJECT
public:
    widbox(QString text,QWidget *parent = nullptr);

    QLabel * widinfo;

    QLabel * windicon;

    QLabel * icon;

    QPushButton * widbutton;

    QPushButton * buttonclose;

    //QGSettings     *m_pGsettings;

    QVBoxLayout * Allbox;

    QHBoxLayout * layoutBox1;

    QHBoxLayout * layoutBox2;

    QHBoxLayout * layoutBox3;

    QString textname;

    QGSettings * m_pGsettingFontSize;

    QString setAutoWrap(const QFontMetrics& font, const QString& text, int nLabelSize);

    void gsettingInit();
    QFont getSystemFont(int size );




};

#endif // WIDBOX_H
