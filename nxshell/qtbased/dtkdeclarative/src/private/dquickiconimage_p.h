// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKICONIMAGE_H
#define DQUICKICONIMAGE_H

#include <dtkdeclarative_global.h>

#include <QIcon>
#ifndef emit
#define emit Q_EMIT
#include <private/qquickimage_p.h>
#undef emit
#else
#include <private/qquickimage_p.h>
#endif

DQUICK_BEGIN_NAMESPACE

class DQuickIconImagePrivate;
class DQuickIconImage : public QQuickImage
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QUrl fallbackSource READ fallbackSource WRITE setFallbackSource NOTIFY fallbackSourceChanged FINAL)

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(QtIcon)
#endif
public:
    enum class Mode {
        Invalid = QIcon::Normal - 1,
        Normal = QIcon::Normal,
        Disabled = QIcon::Disabled,
        Active = QIcon::Active,
        Selected = QIcon::Selected
    };
    enum class State {
        On = QIcon::On,
        Off = QIcon::Off
    };

    Q_ENUM(Mode)
    Q_ENUM(State)

    explicit DQuickIconImage(QQuickItem *parent = nullptr);
    ~DQuickIconImage();

    void componentComplete() override;

    QString name() const;
    State state() const;
    Mode mode() const;
    QColor color() const;

    const QUrl &fallbackSource() const;

public Q_SLOTS:
    void setName(const QString &name);
    void setState(State state);
    void setMode(Mode mode);
    void setColor(const QColor &color);
    void setFallbackSource(const QUrl &newSource);

Q_SIGNALS:
    void nameChanged();
    void stateChanged();
    void modeChanged();
    void colorChanged();
    void fallbackSourceChanged();

protected:
    DQuickIconImage(DQuickIconImagePrivate &dd, QQuickItem *parent);
    void itemChange(ItemChange change, const ItemChangeData &value) override;
    void pixmapChange() override;

private:
    Q_DISABLE_COPY(DQuickIconImage)
    Q_DECLARE_PRIVATE(DQuickIconImage)
    Q_PRIVATE_SLOT(d_func(), void maybeUpdateUrl())
    friend class DQuickDciIconImagePrivate;
};

DQUICK_END_NAMESPACE

#endif // DQUICKICONIMAGE_H
