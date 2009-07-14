#include <QDebug>

#include <QtTest/QtTest>

#include "xmpp_yadatetime.h"
using namespace XMPP;

class TestYaDateTime: public QObject
{
	Q_OBJECT
private:

private slots:
	void initTestCase()
	{
	}

	void cleanupTestCase()
	{
	}

// FIXME: deal with UTC / local timezones
	void testYaTime_t()
	{
		QString orig = "1240920516720315";
		YaDateTime ts = YaDateTime::fromYaTime_t(orig);
		QCOMPARE(ts.toYaTime_t(), orig);
		QCOMPARE(ts.toYaIsoTime(), QString("2009-04-28 12:08:36.720315"));
	}

	void testYaIsoTime()
	{
		QString orig = "2009-04-28 12:08:36.720315";
		YaDateTime ts = YaDateTime::fromYaIsoTime(orig);
		QCOMPARE(ts.toYaTime_t(), QString("1240920516720315"));
		QCOMPARE(ts.toYaIsoTime(), QString("2009-04-28 12:08:36.720315"));
	}

	void testCompare()
	{
		YaDateTime ts1 = YaDateTime::fromYaTime_t("1240920516720315");
		YaDateTime ts2 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720315");
		YaDateTime ts3 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720314");
		YaDateTime ts4 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:30.720315");
		Q_ASSERT(ts1 == ts2);
		Q_ASSERT(!(ts1 == ts3));
		Q_ASSERT(ts1.date() == ts3.date());
		Q_ASSERT(ts1.time() == ts3.time());
		Q_ASSERT(ts1.microsec() != ts3.microsec());
		QCOMPARE(ts1 == ts3, false);

		Q_ASSERT(ts1 != ts3);

		Q_ASSERT(ts2 != ts3);
		Q_ASSERT(ts1 != ts4);
		Q_ASSERT(ts2 != ts4);
		Q_ASSERT(ts3 != ts4);
	}

	void testAssignment()
	{
		YaDateTime ts1 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720315");
		YaDateTime ts2 = YaDateTime::fromYaIsoTime("2009-04-28 12:01:36.720314");
		Q_ASSERT(ts1 != ts2);
		ts1 = ts2;
		Q_ASSERT(ts1 == ts2);
		YaDateTime ts3(ts1);
		Q_ASSERT(ts1 == ts3);
		YaDateTime ts4(YaDateTime::fromYaIsoTime("2009-04-28 10:01:36.720314"));
		Q_ASSERT(ts1 != ts4);
	}

	void testComparisons()
	{
		YaDateTime ts1 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720314");
		YaDateTime ts2 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720315");
		YaDateTime ts3 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720316");
		Q_ASSERT(ts1 < ts2);
		Q_ASSERT(ts1 < ts3);
		Q_ASSERT(ts2 <= ts2);
		Q_ASSERT(ts3 > ts2);
		Q_ASSERT(ts3 >= ts2);
		Q_ASSERT(ts3 > ts1);

		ts1 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:35.720315");
		ts2 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720315");
		ts3 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:37.720315");
		Q_ASSERT(ts1 < ts2);
		Q_ASSERT(ts1 < ts3);
		Q_ASSERT(ts2 <= ts2);
		Q_ASSERT(ts3 > ts2);
		Q_ASSERT(ts3 >= ts2);
		Q_ASSERT(ts3 > ts1);

		ts1 = YaDateTime::fromYaIsoTime("2009-04-27 12:08:36.720315");
		ts2 = YaDateTime::fromYaIsoTime("2009-04-28 12:08:36.720315");
		ts3 = YaDateTime::fromYaIsoTime("2009-04-29 12:08:36.720315");
		Q_ASSERT(ts1 < ts2);
		Q_ASSERT(ts1 < ts3);
		Q_ASSERT(ts2 <= ts2);
		Q_ASSERT(ts3 > ts2);
		Q_ASSERT(ts3 >= ts2);
		Q_ASSERT(ts3 > ts1);
	}

	void testMoreStuff()
	{
		YaDateTime ts0 = YaDateTime::fromYaIsoTime("1970-01-01 00:00:00");
		QCOMPARE(ts0.toString(Qt::ISODate), QString("1970-01-01T03:00:00"));
		QCOMPARE(ts0.toYaIsoTime(), QString("1970-01-01 00:00:00.000000"));
		QCOMPARE(ts0.toYaTime_t(), QString("0000000"));
		QCOMPARE(YaDateTime::fromYaTime_t(ts0.toYaTime_t()).toYaIsoTime(), QString("1970-01-01 00:00:00.000000"));

		YaDateTime ts1 = YaDateTime::fromYaIsoTime("2009-04-30 08:59:38");
		QCOMPARE(ts1.toUTC().toString(Qt::ISODate), QString("2009-04-30T08:59:38"));
		QCOMPARE(ts1.toString(Qt::ISODate), QString("2009-04-30T12:59:38"));
		QString id = ts1.toYaTime_t();
		YaDateTime ts2 = YaDateTime::fromYaTime_t(id);
		QCOMPARE(ts2.toUTC().toString(Qt::ISODate), QString("2009-04-30T08:59:38"));
		QCOMPARE(ts2.toString(Qt::ISODate), QString("2009-04-30T12:59:38"));
		QCOMPARE(ts2.toYaIsoTime(), ts1.toYaIsoTime());
		Q_ASSERT(ts1 == ts2);
	}
};

QTEST_MAIN(TestYaDateTime)
#include "testyadatetime.moc"
