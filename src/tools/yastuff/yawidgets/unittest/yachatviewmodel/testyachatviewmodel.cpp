#include <QDebug>

#include <QtTest/QtTest>
#include <math.h>

#include "yachatviewmodel.h"
using namespace XMPP;

enum Type {
	DateHeader = 0,
	MessageNormal,
	MessageHistory,
	MessageSync,
	MessageOfflineStorage
};

class Message {
public:
	Message(Type _type, QString _time, bool _local = false, const char* _text = 0)
	: type(_type)
	, time(_time)
	, local(_local)
	, text(QString::fromUtf8(_text))
	{}

	Type type;
	QString time;
	bool local;
	QString text;
};

bool messageSortAsc(const Message& m1, const Message& m2)
{
	return m1.time < m2.time;
}

bool messageSortDesc(const Message& m1, const Message& m2)
{
	return m1.time > m2.time;
}

bool messageSortRandom(const Message& m1, const Message& m2)
{
	Q_UNUSED(m1);
	Q_UNUSED(m2);
	return (rand() % 10) > 5;
}

namespace QTest
{
	template<>
	char* toString(const XMPP::YaDateTime &yaDateTime)
	{
		QByteArray ba = "YaDateTime(";
		ba += yaDateTime.toYaIsoTime();
		ba += ")";
		return qstrdup(ba.data());
	}
}

class TestYaChatViewModel: public QObject
{
	Q_OBJECT
private:
	// ^\<message originLocal=\"(true|false)\" timeStamp=\"(.+)\" isMood=\"false\" \>(.+)\</message\>$
	// result << Message(MessageNormal, "$2", $1, "$3");
	QList<Message> testData()
	{
		QList<Message> result;
		// <contact account="xtulhu@ya.ru" jid="chikarabon@ya.ru" >
		result << Message(DateHeader,    "2009-05-01 00:36:34.977400");
		result << Message(MessageNormal, "2009-05-01 00:36:34.977400", true, "pyschch!");
		result << Message(MessageNormal, "2009-05-01 00:40:13.940500", true, "neko-neko");
		result << Message(MessageNormal, "2009-05-01 00:40:25.052400", true, "fooz");
		result << Message(MessageNormal, "2009-05-01 00:51:33.602500", true, "mam");
		result << Message(DateHeader,    "2009-05-09 03:02:13.149200");
		result << Message(MessageNormal, "2009-05-09 03:02:13.149200", false, "bdyisch!");
		result << Message(MessageNormal, "2009-05-09 03:03:16.286900", false, "seychas kak dupanet!");
		result << Message(MessageNormal, "2009-05-09 03:03:21.946899", false, "seychas kak dupanet2!");
		result << Message(MessageNormal, "2009-05-09 03:06:50.798500", false, "bdyisch! bdyisch!");
		result << Message(MessageNormal, "2009-05-09 03:06:59.615000", false, "pararam!");
		result << Message(MessageNormal, "2009-05-09 03:10:31.336999", false, "lyalyalya");
		result << Message(MessageNormal, "2009-05-09 03:10:37.540499", false, "trulyalyal");
		result << Message(MessageNormal, "2009-05-09 03:14:02.240299", false, "bababam!");
		result << Message(MessageNormal, "2009-05-09 03:18:05.537200", false, "novyiy den");
		if (YaChatViewModel::canRelyOnTimestamps()) {
			result << Message(MessageNormal, "2009-05-09 03:18:05.549300", false, "novyiy den");
		}
		result << Message(MessageNormal, "2009-05-09 03:18:08.240700", false, "novyiy dup!");
		if (YaChatViewModel::canRelyOnTimestamps()) {
			result << Message(MessageNormal, "2009-05-09 03:18:08.252899", false, "novyiy dup!");
		}
		result << Message(MessageNormal, "2009-05-09 03:22:07.470000", false, "Solnyishko!");
		result << Message(MessageNormal, "2009-05-09 03:22:12.312700", false, "svetit i prihodit v nashdom");
		// result << Message(MessageNormal, "2009-05-10 00:52:26.571700", false, "a segodnya tuchki");
		// result << Message(MessageNormal, "2009-05-10 00:52:33.501499", false, "a tuchki kak lyudi");

{
#if 0
		// <contact account="negi@siruba.local" jid="asuna@siruba.local" >
		result << Message(MessageNormal, "2009-03-05 17:43:16.196100", true, "asuna2-2");
		result << Message(MessageNormal, "2009-03-05 17:50:17.223700", true, "4");
		result << Message(MessageNormal, "2009-04-28 08:13:43.000000", false, "trulyalya");
		result << Message(MessageNormal, "2009-04-30 08:24:46.000000", false, "privet, tyi negi");
		result << Message(MessageNormal, "2009-03-05 17:43:15.574600", true, "asuna2-1");
		result << Message(MessageNormal, "2009-04-28 08:04:33.329999", false, "yo?");
		result << Message(MessageNormal, "2009-04-30 08:24:47.000000", false, "nya!");
		result << Message(MessageNormal, "2009-04-30 08:59:35.000000", false, "esche tebe nya ot asunyi");
		result << Message(MessageNormal, "2009-03-05 17:43:41.627199", true, "asuna2-1");
		result << Message(MessageNormal, "2009-03-05 17:45:48.291699", true, "b");
		result << Message(MessageNormal, "2009-03-05 17:44:16.856100", true, "asdf");
		result << Message(MessageNormal, "2009-03-05 17:53:13.728499", true, "asdf");
		result << Message(MessageNormal, "2009-04-28 08:15:49.755000", false, "pyischyipyischpyisch tyits tyits tyits");
		result << Message(MessageNormal, "2009-03-05 17:53:54.746900", true, "asd");
		result << Message(MessageNormal, "2009-03-05 17:45:37.873800", true, "a");
		result << Message(MessageNormal, "2009-03-05 17:43:17.950799", true, "asuna2-3");
		result << Message(MessageNormal, "2009-03-05 17:56:09.668400", true, "s");
		result << Message(MessageNormal, "2009-03-05 17:50:46.902499", true, "5");
		result << Message(MessageNormal, "2009-04-30 08:24:51.000000", true, "hryu-hryu!");
		result << Message(MessageNormal, "2009-04-30 08:59:38.000000", false, "pyischpyischpyisch :)");

		// <contact account="chikarabon@ya.ru" jid="xtulhu@ya.ru" >
		// result << Message(MessageNormal, "2009-04-30 23:01:03.000000", false, "privetik, segodnya uzhe may mesyats");
		// result << Message(MessageNormal, "2009-04-30 23:01:06.000000", false, "kak u tebya dela?");
		// result << Message(MessageNormal, "2009-04-30 23:01:10.000000", false, "skuchaesh li tyi po mne?");
		// result << Message(MessageNormal, "2009-04-30 23:04:31.000000", false, "tyits");
		// result << Message(MessageNormal, "2009-04-30 23:20:35.000000", false, "i esche odin tyits");
		// result << Message(MessageNormal, "2009-04-30 23:27:07.000000", true, "+3");
		// result << Message(MessageNormal, "2009-04-30 23:28:17.000000", false, "+4");
		// result << Message(MessageNormal, "2009-04-30 23:28:30.000000", true, "+5");
		// result << Message(MessageNormal, "2009-04-30 23:28:41.000000", false, "blam");
		// result << Message(MessageNormal, "2009-04-30 23:28:44.000000", true, "bloom");
		// result << Message(MessageNormal, "2009-04-30 23:29:52.000000", false, "pam pam pam");
		// result << Message(MessageNormal, "2009-04-30 23:32:56.000000", false, "tyits");
		// result << Message(MessageNormal, "2009-04-30 23:46:13.000000", false, "tyits tyits");
		// result << Message(MessageNormal, "2009-04-30 23:49:08.000000", false, "nya");
		// result << Message(MessageNormal, "2009-04-30 23:50:11.000000", false, "nya-nya");
		// result << Message(MessageNormal, "2009-05-01 00:36:17.000000", false, "hey-hop");
		// result << Message(MessageNormal, "2009-05-01 00:36:34.000000", false, "pyschch!");
		// result << Message(MessageNormal, "2009-05-01 00:40:13.000000", false, "neko-neko");
		// result << Message(MessageNormal, "2009-05-01 00:40:24.000000", false, "fooz");
		// result << Message(MessageNormal, "2009-05-01 00:51:33.000000", false, "mam");
#endif
}
		return result;
	}

	void appendData(YaChatViewModel& model, const QList<Message>& data, Type addAsType, bool omitYaDateTime = false)
	{
		foreach(const Message& m, data) {
			if (m.type == DateHeader)
				continue;

			YaChatViewModel::SpooledType spooled;
			switch (addAsType) {
			case MessageNormal:
				spooled = YaChatViewModel::Spooled_None;
				break;
			case MessageHistory:
				spooled = YaChatViewModel::Spooled_History;
				break;
			case MessageSync:
				spooled = YaChatViewModel::Spooled_Sync;
				break;
			case MessageOfflineStorage:
				spooled = YaChatViewModel::Spooled_OfflineStorage;
				break;
			case DateHeader:
				Q_ASSERT(false);
			}

			XMPP::YaDateTime yaDateTime = YaDateTime::fromYaIsoTime(m.time);
			model.addMessage(spooled, yaDateTime, m.local, 0, QString(), XMPP::ReceiptNone, m.text, omitYaDateTime ? XMPP::YaDateTime() : yaDateTime);
		}
	}

	void debugData(const QList<Message>& data)
	{
		QStringList str;
		foreach(const Message& m, data) {
			// result << Message(DateHeader,    "2009-05-09 03:02:13.149200");
			// result << Message(MessageNormal, "2009-05-09 03:02:13.149200", false, "бдыщ!");
			QString time = m.time;
			if (m.type == DateHeader) {
				str += QString("Message(DateHeader, \"%1\")").arg(time);
				continue;
			}

			QString spooled;
			switch (m.type) {
			case MessageNormal:
				spooled = "MessageNormal";
				break;
			case MessageHistory:
				spooled = "MessageHistory";
				break;
			case MessageSync:
				spooled = "MessageSync";
				break;
			case MessageOfflineStorage:
				spooled = "MessageOfflineStorage";
				break;
			case DateHeader:
				Q_ASSERT(false);
			}
			str += QString("Message(%1, \"%2\"), %3, \"%4\"")
			       .arg(spooled)
			       .arg(time)
			       .arg(m.local ? "true" : "false")
			       .arg(m.text);
		}

		qWarning("----------------- debugData");
		foreach(const QString& s, str) {
			qWarning("%s", qPrintable(s));
		}
		qWarning("-----------------");
		// qWarning("\n%s\n\n", qPrintable(str.join("\n")));
	}

	void debugModel(YaChatViewModel& model)
	{
		QStringList str;
		for (int row = 0; row < model.invisibleRootItem()->rowCount(); ++row) {
			QStandardItem* item = model.invisibleRootItem()->child(row);
			if (row == 0) {
				Q_ASSERT(item->data(YaChatViewModel::TypeRole) == YaChatViewModel::DummyHeader);
				continue;
			}

			// result << Message(DateHeader,    "2009-05-09 03:02:13.149200");
			// result << Message(MessageNormal, "2009-05-09 03:02:13.149200", false, "бдыщ!");
			QString time = item->data(YaChatViewModel::YaDateTimeRole).value<XMPP::YaDateTime>().toYaIsoTime();
			if (item->data(YaChatViewModel::YaDateTimeRole).value<XMPP::YaDateTime>().isNull()) {
				time = item->data(YaChatViewModel::DateTimeRole).toDateTime().toString(Qt::ISODate);
			}
			if (item->data(YaChatViewModel::TypeRole) == YaChatViewModel::DateHeader) {
				str += QString("Message(DateHeader, \"%1\")").arg(time);
				continue;
			}

			QString spooled;
			switch (item->data(YaChatViewModel::SpooledRole).toInt()) {
			case YaChatViewModel::Spooled_None:
				spooled = "MessageNormal";
				break;
			case YaChatViewModel::Spooled_History:
				spooled = "MessageHistory";
				break;
			case YaChatViewModel::Spooled_Sync:
				spooled = "MessageSync";
				break;
			case YaChatViewModel::Spooled_OfflineStorage:
				spooled = "MessageOfflineStorage";
				break;
			}
			str += QString("Message(%1, \"%2\"), %3, \"%4\"")
			       .arg(spooled)
			       .arg(time)
			       .arg((!item->data(YaChatViewModel::IncomingRole).toBool()) ? "true" : "false")
			       .arg(item->data(Qt::DisplayRole).toString());
		}

		qWarning("----------------- debugModel");
		foreach(const QString& s, str) {
			qWarning("%s", qPrintable(s));
		}
		qWarning("-----------------");
		// qWarning("\n%s\n\n", qPrintable(str.join("\n")));
	}

	void verifyData(YaChatViewModel& model, const QList<Message>& data, bool omitYaDateTime = false)
	{
		QCOMPARE(model.invisibleRootItem()->rowCount(), data.count() + 1);
		for (int row = 0; row < model.invisibleRootItem()->rowCount(); ++row) {
			QStandardItem* item = model.invisibleRootItem()->child(row);
			if (row == 0) {
				Q_ASSERT(item->data(YaChatViewModel::TypeRole) == YaChatViewModel::DummyHeader);
				continue;
			}

			int i = row - 1;
			XMPP::YaDateTime yaDateTime = YaDateTime::fromYaIsoTime(data[i].time);
			QCOMPARE(item->data(YaChatViewModel::DateTimeRole).toDateTime(), QDateTime(yaDateTime));
			if (omitYaDateTime) {
				Q_ASSERT(item->data(YaChatViewModel::YaDateTimeRole).value<XMPP::YaDateTime>().isNull());
			}
			else {
				QCOMPARE(item->data(YaChatViewModel::YaDateTimeRole).value<XMPP::YaDateTime>(), yaDateTime);
			}
			if (item->data(YaChatViewModel::TypeRole) == YaChatViewModel::DateHeader) {
				Q_ASSERT(data[i].type == DateHeader);
				continue;
			}

			Q_ASSERT(item->data(YaChatViewModel::TypeRole) == YaChatViewModel::Message);
			QCOMPARE(item->data(Qt::DisplayRole).toString(), data[i].text);
			QCOMPARE(item->data(YaChatViewModel::IncomingRole).toBool(), !data[i].local);
			QCOMPARE(item->data(YaChatViewModel::EmoteRole).toBool(), false);
		}
	}

	void doTest(Type type, bool omitYaDateTime = false)
	{
		YaChatViewModel model(0);
		QList<Message> data;
		verifyData(model, QList<Message>());

		model.doClear();
		appendData(model, testData(), MessageNormal, omitYaDateTime);
		// debugModel(model);
		verifyData(model, testData(), omitYaDateTime);

		model.doClear();
		appendData(model, testData(), type, omitYaDateTime);
		// debugModel(model);
		verifyData(model, testData(), omitYaDateTime);

		model.doClear();
		verifyData(model, QList<Message>());

		model.doClear();
		data = testData();
		qStableSort(data.begin(), data.end(), messageSortAsc);
		// debugData(data);
		appendData(model, data, type, omitYaDateTime);
		verifyData(model, testData(), omitYaDateTime);

		model.doClear();
		data = testData();
		qStableSort(data.begin(), data.end(), messageSortDesc);
		// debugData(data);
		appendData(model, data, type, omitYaDateTime);
		verifyData(model, testData(), omitYaDateTime);

		model.doClear();
		data = testData();
		qSort(data.begin(), data.end(), messageSortRandom);
		// debugData(data);
		appendData(model, data, type, omitYaDateTime);
		verifyData(model, testData(), omitYaDateTime);

		model.doClear();
		data = testData();
		qSort(data.begin(), data.end(), messageSortRandom);
		appendData(model, data, type, omitYaDateTime);
		qStableSort(data.begin(), data.end(), messageSortDesc);
		appendData(model, data, type, omitYaDateTime);
		qStableSort(data.begin(), data.end(), messageSortAsc);
		appendData(model, data, type, omitYaDateTime);
		verifyData(model, testData(), omitYaDateTime);

	}

private slots:
	void testModel()
	{
		doTest(MessageHistory);
		doTest(MessageSync);
		doTest(MessageOfflineStorage);

		doTest(MessageHistory, true);
		doTest(MessageSync, true);
		doTest(MessageOfflineStorage, true);
	}

	void testDifferentDateTime()
	{
		if (YaChatViewModel::canRelyOnTimestamps()) {
			return;
		}

		YaChatViewModel model(0);
		QList<Message> normalMessages;
		normalMessages << Message(DateHeader, "2009-05-10 00:52:26.571700");
		normalMessages << Message(MessageNormal, "2009-05-10 00:52:26.571700", false, "a segodnya tuchki");
		normalMessages << Message(MessageNormal, "2009-05-10 00:52:33.501499", false, "a tuchki kak lyudi");
		appendData(model, normalMessages, MessageNormal);
		verifyData(model, normalMessages);

		QList<Message> data = testData();
		QList<Message> toVerify = testData();
		toVerify += normalMessages;

		// note the different timestamps
		data << Message(DateHeader, "2009-05-10 00:52:06.571700");
		data << Message(MessageNormal, "2009-05-10 00:52:06.571700", false, "a segodnya tuchki");
		data << Message(MessageNormal, "2009-05-10 00:52:13.501499", false, "a tuchki kak lyudi");

		qStableSort(data.begin(), data.end(), messageSortDesc);
		appendData(model, data, MessageHistory);
		verifyData(model, toVerify);

		model.doClear();
		appendData(model, normalMessages, MessageNormal);
		verifyData(model, normalMessages);
		appendData(model, data, MessageSync);
		appendData(model, toVerify, MessageSync);
		appendData(model, toVerify, MessageHistory);
		verifyData(model, toVerify);
	}

	void testNoYaDateTime()
	{
		YaChatViewModel model(0);
		QList<Message> normalMessages;
		normalMessages << Message(DateHeader, "2009-05-10 00:52:26");
		normalMessages << Message(MessageNormal, "2009-05-10 00:52:26", false, "a segodnya tuchki");
		normalMessages << Message(MessageNormal, "2009-05-10 00:52:33", false, "a tuchki kak lyudi");
		appendData(model, normalMessages, MessageNormal, true);
		verifyData(model, normalMessages, true);

		appendData(model, testData(), MessageHistory, true);
		appendData(model, testData(), MessageSync, true);

		QList<Message> toVerify = testData();
		toVerify += normalMessages;
		verifyData(model, toVerify, true);
	}
};

QTEST_MAIN(TestYaChatViewModel)
#include "testyachatviewmodel.moc"
