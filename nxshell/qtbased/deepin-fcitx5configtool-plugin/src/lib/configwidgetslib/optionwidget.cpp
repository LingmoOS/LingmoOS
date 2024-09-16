/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "optionwidget.h"
#include "config.h"
#include "configwidget.h"
#include "font.h"
#include "fontbutton.h"
#include "keylistwidget.h"
#include "listoptionwidget.h"
#include "logging.h"
#include "varianthelper.h"
#include <KColorButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QLineEdit>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <fcitx-utils/color.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitxqtkeysequencewidget.h>

namespace fcitx {
namespace kcm {

namespace {

class IntegerOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    IntegerOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent), spinBox_(new QSpinBox),
          defaultValue_(option.defaultValue().variant().toString().toInt()) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        spinBox_ = new QSpinBox;
        spinBox_->setMaximum(INT_MAX);
        spinBox_->setMinimum(INT_MIN);
        if (option.properties().contains("IntMax")) {
            auto max = option.properties().value("IntMax");
            if (max.type() == QVariant::String) {
                spinBox_->setMaximum(max.toInt());
            }
        }
        if (option.properties().contains("IntMin")) {
            auto min = option.properties().value("IntMin");
            if (min.type() == QVariant::String) {
                spinBox_->setMinimum(min.toInt());
            }
        }
        connect(spinBox_, qOverload<int>(&QSpinBox::valueChanged), this,
                &OptionWidget::valueChanged);
        layout->addWidget(spinBox_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = readString(map, path());
        if (value.isNull()) {
            spinBox_->setValue(defaultValue_);
        }
        spinBox_->setValue(value.toInt());
    }

    void writeValueTo(QVariantMap &map) override {
        writeVariant(map, path(), QString::number(spinBox_->value()));
    }

    void restoreToDefault() override { spinBox_->setValue(defaultValue_); }

private:
    QSpinBox *spinBox_;
    int defaultValue_;
};

class StringOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    StringOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                       QWidget *parent)
        : OptionWidget(path, parent), lineEdit_(new QLineEdit),
          defaultValue_(option.defaultValue().variant().toString()) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        lineEdit_ = new QLineEdit;
        connect(lineEdit_, &QLineEdit::textChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(lineEdit_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = readString(map, path());
        lineEdit_->setText(value);
    }

    void writeValueTo(QVariantMap &map) override {
        writeVariant(map, path(), lineEdit_->text());
    }

    void restoreToDefault() override { lineEdit_->setText(defaultValue_); }

private:
    QLineEdit *lineEdit_;
    QString defaultValue_;
};

class FontOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    FontOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                     QWidget *parent)
        : OptionWidget(path, parent), fontButton_(new FontButton),
          defaultValue_(option.defaultValue().variant().toString()) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        connect(fontButton_, &FontButton::fontChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(fontButton_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = readString(map, path());
        fontButton_->setFont(parseFont(value));
    }

    void writeValueTo(QVariantMap &map) override {
        writeVariant(map, path(), fontButton_->fontName());
    }

    void restoreToDefault() override {
        fontButton_->setFont(parseFont(defaultValue_));
    }

private:
    FontButton *fontButton_;
    QString defaultValue_;
};

class BooleanOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    BooleanOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent), checkBox_(new QCheckBox),
          defaultValue_(option.defaultValue().variant().toString() == "True") {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        connect(checkBox_, &QCheckBox::clicked, this,
                &OptionWidget::valueChanged);
        checkBox_->setText(option.description());
        layout->addWidget(checkBox_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        checkBox_->setChecked(readBool(map, path()));
    }

    void writeValueTo(QVariantMap &map) override {
        QString value = checkBox_->isChecked() ? "True" : "False";
        writeVariant(map, path(), value);
    }

    void restoreToDefault() override { checkBox_->setChecked(defaultValue_); }

private:
    QCheckBox *checkBox_;
    bool defaultValue_;
};

class KeyListOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    KeyListOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent), keyListWidget_(new KeyListWidget) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        keyListWidget_ = new KeyListWidget(this);

        keyListWidget_->setAllowModifierLess(
            readString(option.properties(),
                       "ListConstrain/AllowModifierLess") == "True");
        keyListWidget_->setAllowModifierOnly(
            readString(option.properties(),
                       "ListConstrain/AllowModifierOnly") == "True");
        connect(keyListWidget_, &KeyListWidget::keyChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(keyListWidget_);

        auto variant = option.defaultValue().variant();
        QVariantMap map;
        if (variant.canConvert<QDBusArgument>()) {
            auto argument = qvariant_cast<QDBusArgument>(variant);
            argument >> map;
        }
        defaultValue_ = readValue(map, "");
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        keyListWidget_->setKeys(readValue(map, path()));
    }

    void writeValueTo(QVariantMap &map) override {
        auto keys = keyListWidget_->keys();
        int i = 0;
        for (auto &key : keys) {
            auto value = QString::fromUtf8(key.toString().data());
            writeVariant(map, QString("%1/%2").arg(path()).arg(i), value);
            i++;
        }
        if (keys.empty()) {
            writeVariant(map, path(), QVariantMap());
        }
    }

    void restoreToDefault() override { keyListWidget_->setKeys(defaultValue_); }

private:
    QList<fcitx::Key> readValue(const QVariantMap &map, const QString &path) {
        int i = 0;
        QList<Key> keys;
        while (true) {
            auto value = readString(map, QString("%1%2%3")
                                             .arg(path)
                                             .arg(path.isEmpty() ? "" : "/")
                                             .arg(i));
            if (value.isNull()) {
                break;
            }
            keys << Key(value.toUtf8().constData());
            i++;
        }
        return keys;
    }

    KeyListWidget *keyListWidget_;
    QList<fcitx::Key> defaultValue_;
};

class KeyOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    KeyOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                    QWidget *parent)
        : OptionWidget(path, parent),
          keyWidget_(new FcitxQtKeySequenceWidget(this)),
          defaultValue_(
              option.defaultValue().variant().toString().toUtf8().constData()) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        keyWidget_->setModifierlessAllowed(
            readBool(option.properties(), "AllowModifierLess"));
        keyWidget_->setModifierOnlyAllowed(
            readBool(option.properties(), "AllowModifierOnly"));

        connect(keyWidget_, &FcitxQtKeySequenceWidget::keySequenceChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(keyWidget_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        Key key;
        auto value = readString(map, path());
        key = Key(value.toUtf8().constData());
        keyWidget_->setKeySequence({key});
    }

    void writeValueTo(QVariantMap &map) override {
        auto keys = keyWidget_->keySequence();
        Key key;
        if (keys.size()) {
            key = keys[0];
        }
        auto value = QString::fromUtf8(key.toString().data());
        writeVariant(map, path(), value);
    }

    void restoreToDefault() override {
        keyWidget_->setKeySequence({defaultValue_});
    }

private:
    FcitxQtKeySequenceWidget *keyWidget_;
    fcitx::Key defaultValue_;
};

class EnumOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    EnumOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                     QWidget *parent)
        : OptionWidget(path, parent), comboBox_(new QComboBox),
          toolButton_(new QToolButton) {
        auto *layout = new QHBoxLayout;
        toolButton_->setIcon(QIcon::fromTheme("preferences-system-symbolic"));
        layout->setMargin(0);

        int i = 0;
        while (true) {
            auto value =
                readString(option.properties(), QString("Enum/%1").arg(i));
            if (value.isNull()) {
                break;
            }
            auto text =
                readString(option.properties(), QString("EnumI18n/%1").arg(i));
            if (text.isEmpty()) {
                text = value;
            }
            auto subConfigPath = readString(option.properties(),
                                            QString("SubConfigPath/%1").arg(i));
            comboBox_->addItem(text, value);
            comboBox_->setItemData(i, subConfigPath, subConfigPathRole);
            i++;
        }
        layout->addWidget(comboBox_);
        layout->addWidget(toolButton_);
        setLayout(layout);

        connect(comboBox_, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &OptionWidget::valueChanged);

        connect(comboBox_, qOverload<int>(&QComboBox::currentIndexChanged),
                this, [this]() {
                    toolButton_->setVisible(
                        !comboBox_->currentData(subConfigPathRole)
                             .toString()
                             .isEmpty());
                });

        connect(toolButton_, &QToolButton::clicked, this, [this]() {
            ConfigWidget *configWidget = getConfigWidget(this);
            if (!configWidget) {
                return;
            }
            QPointer<QDialog> dialog = ConfigWidget::configDialog(
                this, configWidget->dbus(),
                comboBox_->currentData(subConfigPathRole).toString(),
                comboBox_->currentText());
            dialog->exec();
            delete dialog;
        });

        defaultValue_ = option.defaultValue().variant().toString();
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = readString(map, path());
        auto idx = comboBox_->findData(value);
        if (idx < 0) {
            idx = comboBox_->findData(defaultValue_);
        }
        comboBox_->setCurrentIndex(idx);
        toolButton_->setVisible(
            !comboBox_->currentData(subConfigPathRole).toString().isEmpty());
    }

    void writeValueTo(QVariantMap &map) override {
        writeVariant(map, path(), comboBox_->currentData().toString());
    }

    void restoreToDefault() override {
        auto idx = comboBox_->findData(defaultValue_);
        comboBox_->setCurrentIndex(idx);
    }

private:
    QComboBox *comboBox_;
    QToolButton *toolButton_;
    QString defaultValue_;
    inline static constexpr int subConfigPathRole = Qt::UserRole + 1;
};

class ColorOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    ColorOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                      QWidget *parent)
        : OptionWidget(path, parent), colorButton_(new KColorButton) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->addWidget(colorButton_);
        colorButton_->setAlphaChannelEnabled(true);
        setLayout(layout);
        connect(colorButton_, &KColorButton::changed, this,
                &OptionWidget::valueChanged);

        try {
            defaultValue_.setFromString(
                option.defaultValue().variant().toString().toStdString());
        } catch (...) {
        }
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = readString(map, path());
        Color color;
        try {
            color.setFromString(value.toStdString());
        } catch (...) {
            color = defaultValue_;
        }
        QColor qcolor;
        qcolor.setRedF(color.redF());
        qcolor.setGreenF(color.greenF());
        qcolor.setBlueF(color.blueF());
        qcolor.setAlphaF(color.alphaF());
        colorButton_->setColor(qcolor);
    }

    void writeValueTo(QVariantMap &map) override {
        auto color = colorButton_->color();
        Color fcitxColor;
        fcitxColor.setRedF(color.redF());
        fcitxColor.setGreenF(color.greenF());
        fcitxColor.setBlueF(color.blueF());
        fcitxColor.setAlphaF(color.alphaF());
        writeVariant(map, path(),
                     QString::fromStdString(fcitxColor.toString()));
    }

    void restoreToDefault() override {
        QColor qcolor;
        qcolor.setRedF(defaultValue_.redF());
        qcolor.setGreenF(defaultValue_.greenF());
        qcolor.setBlueF(defaultValue_.blueF());
        qcolor.setAlphaF(defaultValue_.alphaF());
        colorButton_->setColor(qcolor);
    }

private:
    KColorButton *colorButton_;
    Color defaultValue_;
};

class ExternalOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    ExternalOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                         QWidget *parent)
        : OptionWidget(path, parent),
          uri_(readString(option.properties(), "External")),
          launchSubConfig_(readBool(option.properties(), "LaunchSubConfig")) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        button_ = new QToolButton(this);
        button_->setIcon(QIcon::fromTheme("preferences-system-symbolic"));
        button_->setText(_("Configure"));
        layout->addWidget(button_);
        setLayout(layout);

        connect(
            button_, &QPushButton::clicked, this,
            [this, parent, name = option.name()]() {
                if (launchSubConfig_) {
                    ConfigWidget *configWidget = getConfigWidget(this);
                    if (!configWidget) {
                        return;
                    }
                    QPointer<QDialog> dialog = ConfigWidget::configDialog(
                        this, configWidget->dbus(), uri_, name);
                    dialog->exec();
                    delete dialog;
                } else if (uri_.startsWith("fcitx://config/addon/")) {
                    QString wrapperPath = FCITX5_QT5_GUI_WRAPPER;
                    if (!QFileInfo(wrapperPath).isExecutable()) {
                        wrapperPath =
                            QString::fromStdString(stringutils::joinPath(
                                StandardPath::global().fcitxPath("libexecdir"),
                                "fcitx5-qt5-gui-wrapper"));
                    }
                    QStringList args;
                    if (QGuiApplication::platformName() == "xcb") {
                        auto wid = parent->winId();
                        if (wid) {
                            args << "-w";
                            args << QString::number(wid);
                        }
                    }
                    args << uri_;
                    qCDebug(KCM_FCITX5) << "Launch: " << wrapperPath << args;
                    QProcess::startDetached(wrapperPath, args);
                } else {
                // Assume this is a program path.
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                    QStringList args = QProcess::splitCommand(uri_);
                    QString program = args.takeFirst();
                    QProcess::startDetached(program, args);
#else
                    QProcess::startDetached(uri_);
#endif
                }
            });
    }

    void readValueFrom(const QVariantMap &) override {}
    void writeValueTo(QVariantMap &) override {}
    void restoreToDefault() override {}

private:
    QToolButton *button_;
    const QString uri_;
    const bool launchSubConfig_;
};
} // namespace

OptionWidget *OptionWidget::addWidget(QFormLayout *layout,
                                      const fcitx::FcitxQtConfigOption &option,
                                      const QString &path, QWidget *parent) {
    OptionWidget *widget = nullptr;
    if (option.type() == "Integer") {
        widget = new IntegerOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "String") {
        const auto isFont = readBool(option.properties(), "Font");
        const auto isEnum = readBool(option.properties(), "IsEnum");
        if (isFont) {
            widget = new FontOptionWidget(option, path, parent);
        } else if (isEnum) {
            widget = new EnumOptionWidget(option, path, parent);
        } else {
            widget = new StringOptionWidget(option, path, parent);
        }
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Boolean") {
        widget = new BooleanOptionWidget(option, path, parent);
        layout->addRow("", widget);
    } else if (option.type() == "Key") {
        widget = new KeyOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "List|Key") {
        widget = new KeyListOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Enum") {
        widget = new EnumOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Color") {
        widget = new ColorOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type().startsWith("List|")) {
        widget = new ListOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "External") {
        widget = new ExternalOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    }
    if (widget) {
        if (option.properties().contains("Tooltip")) {
            widget->setToolTip(option.properties().value("Tooltip").toString());
        }
    }
    return widget;
}

bool OptionWidget::execOptionDialog(QWidget *parent,
                                    const fcitx::FcitxQtConfigOption &option,
                                    QVariant &result) {
    QPointer<QDialog> dialog = new QDialog(parent);
    dialog->setWindowIcon(QIcon::fromTheme("fcitx"));
    dialog->setWindowTitle(option.description());
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialog->setLayout(dialogLayout);

    ConfigWidget *parentConfigWidget = getConfigWidget(parent);
    OptionWidget *optionWidget = nullptr;
    ConfigWidget *configWidget = nullptr;
    if (parentConfigWidget->description().contains(option.type())) {
        configWidget =
            new ConfigWidget(parentConfigWidget->description(), option.type(),
                             parentConfigWidget->dbus());
        configWidget->setValue(result);
        dialogLayout->addWidget(configWidget);
    } else {
        QFormLayout *subLayout = new QFormLayout;
        dialogLayout->addLayout(subLayout);
        optionWidget =
            addWidget(subLayout, option, QString("Value"), dialog.data());
        if (!optionWidget) {
            return false;
        }
        QVariantMap origin;
        origin["Value"] = result;
        optionWidget->readValueFrom(origin);
    }

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Cancel"));
    dialogLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    auto ret = dialog->exec();
    bool dialogResult = false;
    if (ret && dialog) {
        if (optionWidget) {
            if (optionWidget->isValid()) {
                QVariantMap map;
                optionWidget->writeValueTo(map);
                result = map.value("Value");
                dialogResult = true;
            }
        } else {
            result = configWidget->value();
            dialogResult = true;
        }
    }
    delete dialog;
    return dialogResult;
}

QString OptionWidget::prettify(const fcitx::FcitxQtConfigOption &option,
                               const QVariant &value) {
    if (option.type() == "Integer") {
        return value.toString();
    } else if (option.type() == "String") {
        return value.toString();
    } else if (option.type() == "Boolean") {
        return value.toString() == "True" ? _("Yes") : _("No");
    } else if (option.type() == "Key") {
        return value.toString();
    } else if (option.type() == "Enum") {
        QMap<QString, QString> enumMap;
        int i = 0;
        while (true) {
            auto value =
                readString(option.properties(), QString("Enum/%1").arg(i));
            if (value.isNull()) {
                break;
            }
            auto text =
                readString(option.properties(), QString("EnumI18n/%1").arg(i));
            if (text.isEmpty()) {
                text = value;
            }
            enumMap[value] = text;
            i++;
        }
        return enumMap.value(value.toString());
    } else if (option.type().startsWith("List|")) {

        int i = 0;
        QStringList strs;
        strs.clear();
        auto subOption = option;
        subOption.setType(option.type().mid(5)); // Remove List|
        while (true) {
            auto subValue = readVariant(value, QString(i));
            strs << prettify(subOption, subValue);
            i++;
        }
        return QString(_("[%1]")).arg(strs.join(" "));
    } else {
        auto *configWidget = getConfigWidget(this);
        if (configWidget &&
            configWidget->description().contains(option.type())) {
            if (auto key =
                    option.properties().value("ListDisplayOption").toString();
                !key.isEmpty()) {
                const auto &options =
                    *configWidget->description().find(option.type());
                for (const auto &option : options) {
                    if (option.name() == key) {
                        return prettify(option, readVariant(value, key));
                    }
                }
            }
        }
    }
    return QString();
}

} // namespace kcm
} // namespace fcitx

#include "optionwidget.moc"
