/*
 * SPDX-FileCopyrightText: 2022~2022 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "config.h"
#include <KIconLoader>
#include <KLocalizedString>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <QBitmap>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QSocketNotifier>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/color.h>
#include <fcitx-utils/standardpath.h>
#include <fcntl.h>
#include <memory>
#include <qmath.h>

namespace {
bool fd_is_valid(int fd) { return fcntl(fd, F_GETFD) != -1 || errno != EBADF; }

fcitx::Color toFcitxColor(const QColor &color) {
    fcitx::Color fcitxColor;
    fcitxColor.setRedF(color.redF());
    fcitxColor.setGreenF(color.greenF());
    fcitxColor.setBlueF(color.blueF());
    fcitxColor.setAlphaF(color.alphaF());
    return fcitxColor;
}

void setMarginsToConfig(fcitx::RawConfig &config, const std::string &name,
                        qreal left, qreal top, qreal right, qreal bottom) {
    auto &subConfig = config[name];
    subConfig["Left"] = std::to_string(qRound(left));
    subConfig["Top"] = std::to_string(qRound(top));
    subConfig["Right"] = std::to_string(qRound(right));
    subConfig["Bottom"] = std::to_string(qRound(bottom));
}

template <typename ImageType>
bool safeSaveImage(const ImageType &image, const QString &path) {
    return fcitx::StandardPath::global().safeSave(
        fcitx::StandardPath::Type::Data, path.toLocal8Bit().constData(),
        [&image](int fd) {
            QFile file;
            if (!file.open(fd, QIODevice::WriteOnly)) {
                qDebug() << "FAILED TO OPEN QFILE";
                return false;
            }
            return image.save(&file, "png");
        });
}

} // namespace

class WatcherApp : public QGuiApplication {
    Q_OBJECT
public:
    WatcherApp(int &argc, char **argv) : QGuiApplication(argc, argv) {
        setApplicationName(QLatin1String("fcitx5-plasma-theme-watcher"));
        setApplicationVersion(QLatin1String(PROJECT_VERSION));
    }

    bool init() {
        QCommandLineParser parser;
        parser.setApplicationDescription(
            i18n("Generate Fcitx 5 Classic UI Theme based on Plasma theme"));
        parser.addHelpOption();
        parser.addOptions(
            {{{"t", "theme"}, i18n("Plasma theme name <name> "), i18n("name")},
             {{"o", "output"}, i18n("Output path <output> "), i18n("output")}});
        QCommandLineOption option{"fd", i18n("File descriptor <fd> "),
                                  i18n("fd")};
        option.setFlags(QCommandLineOption::HiddenFromHelp);
        parser.addOption(option);
        parser.process(*this);

        if (parser.isSet("fd")) {
            int fd = -1;
            bool ok = false;
            fd = parser.value("fd").toInt(&ok);
            if (!ok || !fd_is_valid(fd)) {
                return false;
            }
            fd_ = fd;
        }

        if (parser.isSet("output")) {
            outputPath_ = parser.value("output");
        } else {
            outputPath_ = QString::fromStdString(fcitx::stringutils::joinPath(
                fcitx::StandardPath::global().userDirectory(
                    fcitx::StandardPath::Type::PkgData),
                "themes/plasma"));
        }

        qDebug() << "Will write new themes to: " << outputPath_;

        if (parser.isSet("theme")) {
            theme_ = std::make_unique<Plasma::Theme>(parser.value("theme"));
        } else {
            theme_ = std::make_unique<Plasma::Theme>();
        }

        if (monitorMode()) {
            socketNotifier_ =
                new QSocketNotifier(fd_, QSocketNotifier::Read, this);
            connect(socketNotifier_, &QSocketNotifier::activated, this,
                    [this]() {
                        char buf;
                        if (fcitx::fs::safeRead(fd_, &buf, 1) <= 0) {
                            quit();
                        }
                    });
            connect(theme_.get(), &Plasma::Theme::themeChanged, this, [this]() {
                if (!generateTheme()) {
                    qDebug() << "Failed to generate theme.";
                }
            });
        }
        return true;
    }

    bool monitorMode() const { return fd_ >= 0; }

    bool generateTheme() {
        QDir dir(outputPath_);
        if (!dir.mkpath(".")) {
            return false;
        }
        // Same logic from plasma-frameworks
        const int gridUnit = QFontMetrics(QGuiApplication::font())
                                 .boundingRect(QStringLiteral("M"))
                                 .height();
        const int smallSpacing =
            qMax(2, (gridUnit / 4)); // 1/4 of gridUnit, at least 2
        const qreal textMargin = smallSpacing / 2.0f;
        fcitx::RawConfig config;
        // Write metadata to theme config
        auto &metadata = config["Metadata"];
        metadata["Name"] = "Plasma";
        metadata["Version"] = "1";
        metadata["Author"] = "Fcitx";
        metadata["Description"] =
            i18n("Theme generated from Plasma Theme %1", theme_->themeName())
                .toStdString();

        auto &inputPanel = config["InputPanel"];
        inputPanel["NormalColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();
        inputPanel["HighlightCandidateColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();
        inputPanel["HighlightColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::HighlightedTextColor))
                .toString();
        inputPanel["HighlightBackgroundColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::HighlightColor))
                .toString();
        inputPanel["PageButtonAlignment"] = "Last Candidate";

        auto &menu = config["Menu"];
        inputPanel["NormalColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();
        inputPanel["HighlightCandidateColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();

        QImage background(QSize(200, 200), QImage::Format_ARGB32);
        background.fill(Qt::transparent);

        {
            qreal shadowLeft = 0, shadowRight = 0, shadowTop = 0,
                  shadowBottom = 0;
            qreal bgLeft = 0, bgRight = 0, bgTop = 0, bgBottom = 0;
            Plasma::FrameSvg shadowSvg;
            shadowSvg.setTheme(theme_.get());
            shadowSvg.setImagePath("dialogs/background");
            const bool hasShadow = shadowSvg.hasElementPrefix("shadow");
            if (hasShadow) {
                shadowSvg.setElementPrefix("shadow");
                shadowSvg.resizeFrame(QSize(200, 200));
                shadowSvg.getMargins(shadowLeft, shadowTop, shadowRight,
                                     shadowBottom);
            }

            Plasma::FrameSvg svg;
            svg.setTheme(theme_.get());
            svg.setImagePath("dialogs/background");
            svg.resizeFrame(
                QSizeF(200, 200) -
                QSizeF(shadowLeft + shadowRight, shadowTop + shadowBottom));
            svg.getMargins(bgLeft, bgTop, bgRight, bgBottom);
            {
                QPainter p(&background);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.save();
                svg.paintFrame(&p, QPointF(shadowLeft, shadowTop));
                p.restore();
                if (hasShadow) {
                    p.save();
                    shadowSvg.paintFrame(&p);
                    p.restore();
                }
                p.end();
            }
            bgLeft += shadowLeft;
            bgTop += shadowTop;
            bgRight += shadowRight;
            bgBottom += shadowBottom;
            if (!safeSaveImage(background, dir.filePath("panel.png"))) {
                return false;
            }
            svg.resizeFrame(
                QSizeF(200, 200) -
                QSizeF(shadowLeft + shadowRight, shadowTop + shadowBottom) -
                QSizeF(2, 2));
            if (theme_->blurBehindEnabled()) {
                QImage mask(QSize(200, 200), QImage::Format_ARGB32);
                mask.fill(Qt::transparent);
                QPainter p(&mask);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.drawPixmap(QPointF(shadowLeft + 1, shadowTop + 1),
                             svg.alphaMask().mask());
                p.end();
                if (!safeSaveImage(mask, dir.filePath("mask.png"))) {
                    return false;
                }
            }

            menu["Spacing"] = std::to_string(textMargin);
            setMarginsToConfig(inputPanel, "ContentMargin", bgLeft, bgTop,
                               bgRight, bgBottom);
            setMarginsToConfig(menu, "ContentMargin", bgLeft, bgTop, bgRight,
                               bgBottom);
            setMarginsToConfig(inputPanel, "ShadowMargin", shadowLeft,
                               shadowTop, shadowRight, shadowBottom);
            inputPanel["Background"]["Image"] = "panel.png";
            if (theme_->blurBehindEnabled()) {
                inputPanel["BlurMask"] = "mask.png";
                inputPanel["EnableBlur"] = "True";
            }
            menu["Background"]["Image"] = "panel.png";
            setMarginsToConfig(inputPanel["Background"], "Margin", bgLeft,
                               bgTop, bgRight, bgBottom);
            setMarginsToConfig(menu["Background"], "Margin", bgLeft, bgTop,
                               bgRight, bgBottom);
        }

        {
            Plasma::FrameSvg highlightSvg;
            highlightSvg.setTheme(theme_.get());
            highlightSvg.setImagePath("widgets/viewitem");
            if (highlightSvg.hasElementPrefix("hover")) {
                highlightSvg.setElementPrefix("hover");
            } else if (highlightSvg.hasElementPrefix("selected")) {
                highlightSvg.setElementPrefix("selected");
            }
            highlightSvg.resizeFrame(QSize(200, 200));
            if (!safeSaveImage(highlightSvg.framePixmap(),
                               dir.filePath("highlight.png"))) {
                return false;
            }
            qreal bgLeft = 0, bgRight = 0, bgTop = 0, bgBottom = 0;
            highlightSvg.getMargins(bgLeft, bgTop, bgRight, bgBottom);
            bgLeft = qMax(textMargin, bgLeft);
            bgTop = qMax(textMargin, bgTop);
            bgRight = qMax(textMargin, bgRight);
            bgBottom = qMax(textMargin, bgBottom);

            inputPanel["Highlight"]["Image"] = "highlight.png";
            menu["Highlight"]["Image"] = "highlight.png";
            setMarginsToConfig(inputPanel["Highlight"], "Margin", bgLeft, bgTop,
                               bgRight, bgBottom);
            setMarginsToConfig(menu["Highlight"], "Margin", bgLeft, bgTop,
                               bgRight, bgBottom);
            setMarginsToConfig(inputPanel, "TextMargin", bgLeft,
                               bgTop + textMargin, bgRight,
                               bgBottom + textMargin);
            setMarginsToConfig(menu, "TextMargin", bgLeft, bgTop, bgRight,
                               bgBottom);
        }

        {
            Plasma::Svg icon;
            icon.setContainsMultipleImages(true);
            icon.setTheme(theme_.get());
            icon.setImagePath("widgets/arrows");
            icon.resize(KIconLoader::SizeSmallMedium,
                        KIconLoader::SizeSmallMedium);
            if (icon.hasElement("left-arrow") &&
                icon.hasElement("right-arrow")) {
                inputPanel["PrevPage/Image"] = "prev.png";
                if (!safeSaveImage(icon.pixmap("left-arrow"),
                                   dir.filePath("prev.png"))) {
                    return false;
                }
                inputPanel["NextPage/Image"] = "next.png";
                if (!safeSaveImage(icon.pixmap("right-arrow"),
                                   dir.filePath("next.png"))) {
                    return false;
                }
            }
            icon.resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
            if (icon.hasElement("right-arrow")) {
                menu["SubMenu/Image"] = "arrow.png";
                if (!safeSaveImage(icon.pixmap("right-arrow"),
                                   dir.filePath("arrow.png"))) {
                    return false;
                }
            }

            Plasma::Svg radio;
            radio.setContainsMultipleImages(true);
            radio.setTheme(theme_.get());
            radio.setImagePath("widgets/checkmarks");
            radio.resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
            if (radio.hasElement("radiobutton")) {
                menu["CheckBox/Image"] = "radio.png";
                if (!safeSaveImage(radio.pixmap("radiobutton"),
                                   dir.filePath("radio.png"))) {
                    return false;
                }
            }
            Plasma::Svg line;
            line.setContainsMultipleImages(true);
            line.setTheme(theme_.get());
            line.setImagePath("widgets/line");
            if (line.hasElement("horizontal-line")) {
                if (!safeSaveImage(line.pixmap("horizontal-line"),
                                   dir.filePath("line.png"))) {
                    return false;
                }
                menu["Separator/Image"] = "line.png";
            }
        }

        auto ret = fcitx::safeSaveAsIni(
            config, dir.filePath("theme.conf").toLocal8Bit().constData());

        if (monitorMode()) {
            char buf = 0;
            fcitx::fs::safeWrite(fd_, &buf, 1);
            qDebug() << "Notify theme reloading.";
        }
        return ret;
    }

private:
    QSocketNotifier *socketNotifier_ = nullptr;
    int fd_ = -1;
    std::unique_ptr<Plasma::Theme> theme_;
    QString outputPath_;
};

int main(int argc, char *argv[]) {
    WatcherApp app(argc, argv);
    app.setApplicationName(QLatin1String("fcitx5-plasma-theme-generator"));
    app.setApplicationVersion(QLatin1String(PROJECT_VERSION));
    if (!app.init()) {
        return 1;
    }

    auto ret = app.generateTheme();
    if (!app.monitorMode()) {
        return ret ? 0 : 1;
    }
    return app.exec();
}

#include "main.moc"
