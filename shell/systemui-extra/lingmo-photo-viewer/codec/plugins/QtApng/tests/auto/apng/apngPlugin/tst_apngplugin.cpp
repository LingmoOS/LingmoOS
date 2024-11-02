#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QImageReader>
#include <QMovie>

class ApngPluginTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void testFormats();

	void testImageReading_data();
	void testImageReading();
	void testAnimation_data();
	void testAnimation();
	void testFrameDelays();
};

void ApngPluginTest::testFormats()
{
	QVERIFY(QImageReader::supportedImageFormats().contains("apng"));
	QVERIFY(QMovie::supportedFormats().contains("apng"));
}

void ApngPluginTest::testImageReading_data()
{
	QTest::addColumn<QString>("path");
	QTest::addColumn<bool>("valid");
	QTest::addColumn<QSize>("size");
	QTest::addColumn<QPoint>("controlPixel");
	QTest::addColumn<QColor>("controlPixelColor");

	QTest::newRow("sample-1") << QStringLiteral(":/testdata/sample-1.apng")
							  << true
							  << QSize(320, 240)
							  << QPoint(211, 25)
							  << QColor(0xe8, 0x00, 0x00);

	QTest::newRow("sample-2") << QStringLiteral(":/testdata/sample-2.apng")
							  << true
							  << QSize(150, 150)
							  << QPoint(74, 34)
							  << QColor(0xd4, 0x00, 0x00);

	QTest::newRow("sample-3") << QStringLiteral(":/testdata/sample-3.apng")
							  << true
							  << QSize(150, 150)
							  << QPoint(74, 34)
							  << QColor(0xd4, 0x00, 0x00);

	QTest::newRow("sample-4") << QStringLiteral(":/testdata/sample-4.apng")
							  << false
							  << QSize()
							  << QPoint()
							  << QColor();
}

void ApngPluginTest::testImageReading()
{
	QFETCH(QString, path);
	QFETCH(bool, valid);
	QFETCH(QSize, size);
	QFETCH(QPoint, controlPixel);
	QFETCH(QColor, controlPixelColor);

	QImageReader reader(path, "apng");

	QCOMPARE(reader.canRead(), valid);
	QCOMPARE(reader.error(), valid ? QImageReader::UnknownError : QImageReader::UnsupportedFormatError);

	if(!valid)//test ends here for invalid
		return;

	QCOMPARE(reader.format(), QByteArray("apng"));
	auto image = reader.read();
	QVERIFY(!image.isNull());
	QCOMPARE(image.size(), size);
	QCOMPARE(image.pixelColor(controlPixel), controlPixelColor);
}

void ApngPluginTest::testAnimation_data()
{
	QTest::addColumn<QString>("path");
	QTest::addColumn<bool>("animated");
	QTest::addColumn<int>("frames");
	QTest::addColumn<int>("frame0Delay");
	QTest::addColumn<int>("loops");

	QTest::newRow("sample-1") << QStringLiteral(":/testdata/sample-1.apng")
							  << true
							  << 101
							  << 30
							  << -1;

	QTest::newRow("sample-2") << QStringLiteral(":/testdata/sample-2.apng")
							  << true
							  << 4
							  << 500
							  << -1;

	QTest::newRow("sample-3") << QStringLiteral(":/testdata/sample-3.apng")
							  << false
							  << 1
							  << 0
							  << 0;
}

void ApngPluginTest::testAnimation()
{
	QFETCH(QString, path);
	QFETCH(bool, animated);
	QFETCH(int, frames);
	QFETCH(int, frame0Delay);
	QFETCH(int, loops);

	QImageReader reader(path, "apng");

	//test general stuff
	QCOMPARE(reader.supportsAnimation(), animated);
	QCOMPARE(reader.imageCount(), frames);
	QCOMPARE(reader.currentImageNumber(), 0);
	QCOMPARE(reader.nextImageDelay(), 0);
	QCOMPARE(reader.loopCount(), loops);

	if(!animated)
		return;

	//test frame jumping
	QVERIFY(reader.jumpToNextImage());
	QCOMPARE(reader.nextImageDelay(), frame0Delay);
	QVERIFY(reader.jumpToImage(frames - 1));
	QVERIFY(!reader.jumpToNextImage());
	QVERIFY(!reader.jumpToImage(frames));
	QVERIFY(!reader.jumpToImage(-1));
	QVERIFY(reader.jumpToImage(0));

	//test incremental reading
	QImage image;
	auto cnt = 0;
	forever {
		image = reader.read();
		if(image.isNull())
			break;
		auto c = ++cnt;
		if(c >= frames)
			c = -1;
		QCOMPARE(reader.currentImageNumber(), c);
	};
	QCOMPARE(cnt, frames);
}

void ApngPluginTest::testFrameDelays()
{
	QImageReader reader(QStringLiteral(":/testdata/sample-5.apng"), "apng");

	//test general stuff
	QVERIFY(reader.supportsAnimation());
	QCOMPARE(reader.imageCount(), 4);
	QCOMPARE(reader.currentImageNumber(), 0);
	QCOMPARE(reader.nextImageDelay(), 0);
	QCOMPARE(reader.loopCount(), -1);

	QList<QPair<int, bool>> delays {
		{2000, true},
		{500, true},
		{1000, true},
		{500, false},
		{0, false}
	};
	while(!delays.isEmpty()) {
		const auto dInfo = delays.takeFirst();
		QCOMPARE(reader.jumpToNextImage(), dInfo.second);
		QCOMPARE(reader.nextImageDelay(), dInfo.first);
	}
}

QTEST_MAIN(ApngPluginTest)

#include "tst_apngplugin.moc"
