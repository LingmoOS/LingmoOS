#include "kcolorutilstest.h"

#include <QTest>

#include "../colors/kcolorspaces.cpp" // private implementation
#include "../colors/kcolorspaces_p.h" // private header
#include <kcolorutils.h>

void tst_KColorUtils::testOverlay()
{
    QColor color1(10, 10, 100);
    QColor color2(10, 10, 160);
    QColor blended = KColorUtils::overlayColors(color1, color2);
    QCOMPARE(blended, color2); // no transparency.

    QColor previous;
    // check that when altering the alpha of color2 to be less and less transparent this
    // means we are moving more and more towards color2
    for (int i = 10; i <= 255; i += 10) {
        color2.setAlpha(i);
        blended = KColorUtils::overlayColors(color1, color2);
        if (previous.isValid()) {
            QCOMPARE(previous.red(), 10);
            QCOMPARE(previous.green(), 10);
            QVERIFY(previous.blue() <= blended.blue());
        }
        previous = blended;
    }

    // only the alpha of color 2 alters the output
    color2.setAlpha(255); // opaque
    color1.setAlpha(80); // opaque
    blended = KColorUtils::overlayColors(color2, color2);
    QCOMPARE(blended.red(), color2.red());
    QCOMPARE(blended.green(), color2.green());
    QCOMPARE(blended.blue(), color2.blue());

    // merge from itself to itself gives; TADA; itself again ;)
    color2.setAlpha(127);
    blended = KColorUtils::overlayColors(color2, color2);
    QCOMPARE(blended.red(), color2.red());
    QCOMPARE(blended.green(), color2.green());
    QCOMPARE(blended.blue(), color2.blue());
}

/* clang-format off */
#define compareColors(c1, c2) \
    if (c1 != c2) { \
        fprintf(stderr, "%08x != expected value %08x\n", c1.rgb(), c2.rgb()); \
        QCOMPARE(c1, c2); \
    } \
    (void)0
/* clang-format on */

void tst_KColorUtils::testMix()
{
    int r;
    int g;
    int b;
    int k;
    for (r = 0; r < 52; r++) {
        for (g = 0; g < 52; g++) {
            for (b = 0; b < 52; b++) {
                QColor color(r * 5, g * 5, b * 5);
                // Test blend-to-black spectrum
                for (k = 5; k >= 0; k--) {
                    QColor result = KColorUtils::mix(Qt::black, color, k * 0.2);
                    compareColors(result, QColor(r * k, g * k, b * k));
                }
                // Test blend-to-white spectrum
                for (k = 5; k >= 0; k--) {
                    int n = 51 * (5 - k);
                    QColor result = KColorUtils::mix(Qt::white, color, k * 0.2);
                    compareColors(result, QColor(n + r * k, n + g * k, n + b * k));
                }
                // Test blend-to-self across a couple bias values
                for (k = 5; k >= 0; k--) {
                    QColor result = KColorUtils::mix(color, color, k * 0.2);
                    compareColors(result, color);
                }
            }
        }
    }

    const auto colorA = QColor::fromRgb(255, 255, 255, 1);
    const auto colorB = QColor::fromRgb(0, 0, 0, 255);
    const auto mixed = KColorUtils::mix(colorA, colorB, 0.5);

    QVERIFY2(mixed.redF() <= 0.01, "colorA should have little influence on colorB due to low opacity");
    QVERIFY2(mixed.greenF() <= 0.01, "colorA should have little influence on colorB due to low opacity");
    QVERIFY2(mixed.blueF() <= 0.01, "colorA should have little influence on colorB due to low opacity");
}

void tst_KColorUtils::testHCY()
{
    int r;
    int g;
    int b;
    for (r = 0; r < 256; r += 5) {
        for (g = 0; g < 256; g += 5) {
            for (b = 0; b < 256; b += 5) {
                QColor color(r, g, b);
                KColorSpaces::KHCY hcy(color);
                compareColors(hcy.qColor(), color);
            }
        }
    }
}

void tst_KColorUtils::testContrast()
{
    QCOMPARE(KColorUtils::contrastRatio(Qt::black, Qt::white), qreal(21.0));
    QCOMPARE(KColorUtils::contrastRatio(Qt::white, Qt::black), qreal(21.0));
    QCOMPARE(KColorUtils::contrastRatio(Qt::black, Qt::black), qreal(1.0));
    QCOMPARE(KColorUtils::contrastRatio(Qt::white, Qt::white), qreal(1.0));

    // TODO better tests :-)
}

void checkIsGray(const QColor &color, int line)
{
    KColorSpaces::KHCY hcy(color);
    if (hcy.c != qreal(0.0)) {
        fprintf(stderr, "%08x has chroma %g, expected gray!\n", color.rgb(), hcy.c);
    }
    QTest::qCompare(hcy.c, qreal(0.0), "hcy.c", "0.0", __FILE__, line);
}

void tst_KColorUtils::testShading()
{
    const QColor testGray(128, 128, 128); // Qt::gray isn't pure gray!

    // Test that KHCY gets chroma correct for white/black, grays
    checkIsGray(Qt::white, __LINE__);
    checkIsGray(testGray, __LINE__);
    checkIsGray(Qt::black, __LINE__);

    // Test that lighten/darken/shade don't change chroma for grays
    checkIsGray(KColorUtils::shade(Qt::white, -0.1), __LINE__);
    checkIsGray(KColorUtils::shade(Qt::black, 0.1), __LINE__);
    checkIsGray(KColorUtils::darken(Qt::white, 0.1), __LINE__);
    checkIsGray(KColorUtils::darken(testGray, 0.1), __LINE__);
    checkIsGray(KColorUtils::darken(testGray, -0.1), __LINE__);
    checkIsGray(KColorUtils::darken(Qt::black, -0.1), __LINE__);
    checkIsGray(KColorUtils::lighten(Qt::black, 0.1), __LINE__);
    checkIsGray(KColorUtils::lighten(testGray, 0.1), __LINE__);
    checkIsGray(KColorUtils::lighten(testGray, -0.1), __LINE__);
    checkIsGray(KColorUtils::lighten(Qt::white, -0.1), __LINE__);
}

QTEST_MAIN(tst_KColorUtils)

#include "moc_kcolorutilstest.cpp"
