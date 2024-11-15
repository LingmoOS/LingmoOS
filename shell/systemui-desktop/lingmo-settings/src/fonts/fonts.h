#ifndef FONTS_H
#define FONTS_H

#include <QObject>
#include <QStandardItemModel>
#include <QSettings>
#include "kxftconfig.h"

class Fonts : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool antiAliasing READ antiAliasing WRITE setAntiAliasing NOTIFY antiAliasingChanged)
    Q_PROPERTY(QStandardItemModel * hintingModel READ hintingModel CONSTANT)
    Q_PROPERTY(int hintingCurrentIndex READ hintingCurrentIndex WRITE setHintingCurrentIndex NOTIFY hintingCurrentIndexChanged)

public:
    explicit Fonts(QObject *parent = nullptr);

    bool antiAliasing() const;
    void setAntiAliasing(bool antiAliasing);

    int hintingCurrentIndex() const;
    void setHintingCurrentIndex(int index);

    KXftConfig::Hint::Style hinting() const;
    void setHinting(KXftConfig::Hint::Style hinting);

    QStandardItemModel *hintingModel();

    void save();

signals:
    void antiAliasingChanged();
    void hintingChanged();
    void hintingCurrentIndexChanged();

private:
    QSettings m_settings;
    bool m_antiAliasing;
    QStandardItemModel *m_hintingModel;
    KXftConfig::Hint::Style m_hinting;
};

#endif
