#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <KMainWindow>
#include <kateconfig.h>
#include <katedocument.h>
#include <katesearchbar.h>
#include <kateview.h>

static constexpr int lines = 100000;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser p;
    p.setApplicationDescription(QStringLiteral("Performance benchmark for search"));
    p.addHelpOption();
    // number of iterations
    QCommandLineOption iterOpt(QStringLiteral("i"),
                               QStringLiteral("Number of lines of text in which search will happen"),
                               QStringLiteral("iters"),
                               QStringLiteral("0"));
    p.addOption(iterOpt);

    p.process(app);
    bool ok = false;
    int iters = p.value(iterOpt).toInt(&ok);

    KMainWindow *w = new KMainWindow;
    w->activateWindow();

    KTextEditor::DocumentPrivate doc;
    KTextEditor::ViewPrivate view(&doc, nullptr);
    KateViewConfig config(&view);
    KateSearchBar bar(true, &view, &config);

    int linesInText = ok ? (iters > 0 ? iters : lines) : lines;

    QStringList l;
    l.reserve(linesInText);
    for (int i = 0; i < linesInText; ++i) {
        l.append(QStringLiteral("This is a long long long sentence."));
    }
    doc.setText(l);

    QObject::connect(&bar, &KateSearchBar::findOrReplaceAllFinished, [&w]() {
        w->close();
    });

    bar.setSearchMode(KateSearchBar::SearchMode::MODE_PLAIN_TEXT);
    bar.setSearchPattern(QStringLiteral("long"));

    bar.findAll();

    return app.exec();
}
