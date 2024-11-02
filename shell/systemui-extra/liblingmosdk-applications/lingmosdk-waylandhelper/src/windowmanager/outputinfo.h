#ifndef OUTPUTINFO_H
#define OUTPUTINFO_H

#include <QObject>
#include <QRect>

class OutputInfo: public QObject
{
    Q_OBJECT
public:
    ~OutputInfo();

    QString outputName();
    QRect usableArea();


private:
    explicit OutputInfo(QObject *parent = nullptr);
    void setOutputName(const QString& outputName);
    void setUsableArea(const QRect& usableArea);

    friend class OutputUsableAreaManager;
    class Private;
    QScopedPointer<Private> d;
};

#endif // OUTPUTINFO_H
