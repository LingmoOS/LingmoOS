#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>

#include <ThreadWeaver/Exception>
#include <ThreadWeaver/ThreadWeaver>

#include "MainWidget.h"
#include "ViewController.h"

ViewController::ViewController(MainWidget *mainwidget)
    : QObject() // no parent
    , m_apiPostUrl(QStringLiteral("http://fickedinger.tumblr.com/api/read?id=94635924143"))
//@@snippet_begin(hellointernet-sequence)
{
    connect(this, SIGNAL(setImage(QImage)), mainwidget, SLOT(setImage(QImage)));
    connect(this, SIGNAL(setCaption(QString)), mainwidget, SLOT(setCaption(QString)));
    connect(this, SIGNAL(setStatus(QString)), mainwidget, SLOT(setStatus(QString)));

    using namespace ThreadWeaver;
    auto s = new Sequence;
    *s << make_job([this]() {
        loadPlaceholderFromResource();
    }) << make_job([this]() {
        loadPostFromTumblr();
    }) << make_job([this]() {
        loadImageFromTumblr();
    });
    stream() << s;
}
//@@snippet_end

ViewController::~ViewController()
{
    ThreadWeaver::Queue::instance()->finish();
}

//@@snippet_begin(hellointernet-loadresource)
void ViewController::loadPlaceholderFromResource()
{
    QThread::msleep(500);
    showResourceImage("IMG_20140813_004131.png");
    Q_EMIT setStatus(tr("Downloading post..."));
}
//@@snippet_end

//@@snippet_begin(hellointernet-loadpost)
void ViewController::loadPostFromTumblr()
{
    const QUrl url(m_apiPostUrl);

    auto const data = download(url);
    Q_EMIT setStatus(tr("Post downloaded..."));

    QDomDocument doc;
    if (!doc.setContent(data)) {
        error(tr("Post format not recognized!"));
    }

    auto textOfFirst = [&doc](const char *name) {
        auto const s = QString::fromLatin1(name);
        auto elements = doc.elementsByTagName(s);
        if (elements.isEmpty()) {
            return QString();
        }
        return elements.at(0).toElement().text();
    };

    auto const caption = textOfFirst("photo-caption");
    if (caption.isEmpty()) {
        error(tr("Post does not contain a caption!"));
    }
    Q_EMIT setCaption(caption);
    auto const imageUrl = textOfFirst("photo-url");
    if (imageUrl.isEmpty()) {
        error(tr("Post does not contain an image!"));
    }

    m_fullPostUrl = attributeTextFor(doc, "post", "url-with-slug");
    if (m_fullPostUrl.isEmpty()) {
        error(tr("Response does not contain URL with slug!"));
    }
    m_imageUrl = QUrl(imageUrl);
    showResourceImage("IMG_20140813_004131-colors-cubed.png");
    Q_EMIT setStatus(tr("Downloading image..."));
    QThread::msleep(500);
}
//@@snippet_end

void ViewController::loadImageFromTumblr()
{
    auto const data = download(m_imageUrl);
    Q_EMIT setStatus(tr("Image downloaded..."));
    const QImage image = QImage::fromData(data);
    if (!image.isNull()) {
        Q_EMIT setImage(image);
        Q_EMIT setStatus(tr("Download complete (see %1).").arg(m_fullPostUrl));
    } else {
        error(tr("Image format error!"));
    }
}

QByteArray ViewController::download(const QUrl &url)
{
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray data = reply->readAll();
        return data;
    } else {
        error(tr("Unable to download data for \"%1\"!").arg(url.toString()));
        return QByteArray();
    }
}

//@@snippet_begin(hellointernet-error)
void ViewController::error(const QString &message)
{
    showResourceImage("IMG_20140813_004131-colors-cubed.png");
    Q_EMIT setCaption(tr("Error"));
    Q_EMIT setStatus(tr("%1").arg(message));
    throw ThreadWeaver::JobFailed(message);
}
//@@snippet_end

void ViewController::showResourceImage(const char *file)
{
    const QString path(QStringLiteral("://resources/%1").arg(QString::fromLatin1(file)));
    Q_ASSERT(QFile::exists(path));
    const QImage i(path);
    Q_ASSERT(!i.isNull());
    Q_EMIT setImage(i);
}

QString ViewController::attributeTextFor(const QDomDocument &doc, const char *tag, const char *attribute)
{
    auto const tagString = QString::fromLatin1(tag);
    auto const attributeString = QString::fromLatin1(attribute);
    auto elements = doc.elementsByTagName(tagString);
    if (elements.isEmpty()) {
        return QString();
    }
    const QString content = elements.at(0).toElement().attribute(attributeString);
    return content;
}

#include "moc_ViewController.cpp"
