#include "outputinfo.h"

class Q_DECL_HIDDEN OutputInfo::Private
{
public:
    Private(OutputInfo *q);
    ~Private();

    QRect m_usableArea;
    QString m_name;
    OutputInfo* q;
};

OutputInfo::OutputInfo(QObject *parent)
    :QObject(parent),
      d(new Private(this))
{

}

void OutputInfo::setOutputName(const QString &outputName)
{
    d->m_name = outputName;
}

void OutputInfo::setUsableArea(const QRect &usableArea)
{
    d->m_usableArea = usableArea;
}


OutputInfo::~OutputInfo()
{

}

QString OutputInfo::outputName()
{
    return d->m_name;
}

QRect OutputInfo::usableArea()
{
    return d->m_usableArea;
}

OutputInfo::Private::Private(OutputInfo *q)
    :q(q)
{
}

OutputInfo::Private::~Private()
{
}
