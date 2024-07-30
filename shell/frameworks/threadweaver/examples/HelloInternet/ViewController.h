#ifndef VIEWCONTROLLER_H
#define VIEWCONTROLLER_H

#include <QByteArray>
#include <QObject>
#include <QUrl>

class MainWidget;
class QDomDocument;

class ViewController : public QObject
{
    Q_OBJECT
public:
    explicit ViewController(MainWidget *mainwidget);
    ~ViewController() override;

Q_SIGNALS:
    void setImage(QImage image);
    void setCaption(QString text);
    void setStatus(QString text);

private:
    void loadPlaceholderFromResource();
    void loadPostFromTumblr();
    void loadImageFromTumblr();

    QByteArray download(const QUrl &url);
    void error(const QString &message);
    void showResourceImage(const char *file);
    QString attributeTextFor(const QDomDocument &doc, const char *tag, const char *attribute);

    QUrl m_imageUrl;
    const QString m_apiPostUrl;
    QString m_fullPostUrl;
};

#endif // VIEWCONTROLLER_H
