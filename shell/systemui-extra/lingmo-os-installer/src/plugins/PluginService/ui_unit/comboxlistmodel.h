#ifndef COMBOXLISTMODEL_H
#define COMBOXLISTMODEL_H

#include <QAbstractListModel>
namespace KServer {
struct ItemLanguage
{
    QString systemlocal;
    QString itemname;
    QString country;
    QString timezone;
};
typedef QList<ItemLanguage> ItemLanguageList;

class ComboxListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ComboxListModel(QObject *parent = nullptr);
    ~ComboxListModel();
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    ItemLanguage getItemLanguage(const QModelIndex& index);

    QModelIndex getIndex(const QString& itemname);
private:
    ItemLanguageList m_languageList;
};
ItemLanguageList ReadLanguageFile();
}
#endif // COMBOXLISTMODEL_H
