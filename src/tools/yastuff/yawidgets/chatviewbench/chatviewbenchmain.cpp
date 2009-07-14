#include <QApplication>

#include <QTime>
#include <QDebug>

#include "psitextview.h"
#include "yachatview.h"
#include "yachatviewmodel.h"

// Results:
//
// Dummy:       0msecs,      21.93 Real,  267.55 Virtual

// 10 messages
// YaChatView:  385msecs,    26.10 Real,  315.46 Virtual
//                           4.17         47.91
// PsiTextView: 161msecs,    25.59 Real,  315.00 Virtual
//                           3.66         47.45
//
// 1000 messages
// YaChatView:  13253msecs,  41.88 Real,  329.36 Virtual
//                           19.95        61.81
// PsiTextView: 5733msecs,   29.38 Real,  318.48 Virtual
//                           7.45         50.93
//
// 10000 messages
// YaChatView:  141759msecs, 214.76 Real, 503.58 Virtual
//                           192.83       236.83
// PsiTextView: 64346msecs,  64.92 Real,  366.23 Virtual
//                           42.99        98.68

static int totalCount = 500;

// #define DO_YACHATVIEW
#define DO_QTEXTEDIT

#ifdef DO_QTEXTEDIT
const QString TIMESTAMP = "#c7c7c7";

QString linkifyClever(const QString & in)
{
	QString txt = in;
	//                   1        23                             4
	QRegExp r = QRegExp("(^|\\s|>)((http|https|ftp)://[^\\s!<>]*)(<|>|\\s|$|!)");
	txt.replace(r, "\\1<a href=\"\\2\">\\2</a>\\4");
	return txt;
}

const QString formatStanza(bool needOpeningTitle, QString color, QString timestr, QString who, QString txt)
{
	QString result;
	QTextStream stream(&result, QIODevice::WriteOnly);
	if (who.isEmpty()) { // system message
		stream
			<< "<hr>"
			<< "<span style='margin: 0; padding: 0; color: "
			<< TIMESTAMP
			<< "; font-size: smaller;'>"
			<< timestr
			<< ": "
			<< txt
			<< "</span><br>";
	} else {
		if (needOpeningTitle) {
			stream
				<< "<div style='margin: 0; padding: 0; width: 100%; font-weight: bold;'>"
				<< "<span style='color: " << TIMESTAMP << ";'>" << timestr << "</span>"
				<< "&nbsp;"
				<< "<span style='color: " << color << ";'>" << who << "</span>"
				<< "</div>";
		}
		txt = linkifyClever(txt);
		stream
			<< "<div style='margin: 0; padding: 0; width: 100%; font-weight: normal; color: "
			<< QColor(color).dark().name()
			<< "; '>"
			<< "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
			<< txt
			<< "</div>";
	}
	return result;
}

QString formatTimeStamp(const QDateTime &time)
{
	return QString().sprintf("%02d:%02d", time.time().hour(), time.time().minute());
}
#endif

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyleSheet(
	"QWidget {"
	"	font-family: \"Arial\";"
	"}"
	);

	QString msgFrom(QString::fromUtf8("1. многократное повторение двоих собеседников\n2. отличать своё от несвоего\n3. много реплик в единицу времени (минуту)"));
	QString msgTo(QString::fromUtf8("вот три вещи, которые сильно отличают чат от комментов"));

	QString whoFrom(QString::fromUtf8("Николай Яремко"));
	QString whoTo(QString::fromUtf8("zharik"));

#ifdef DO_YACHATVIEW
	{
		YaChatViewModel* model = new YaChatViewModel();

		YaChatView* chatView = new YaChatView(0);
		chatView->nicksChanged(whoFrom, whoTo);
		chatView->setModel(model);
		chatView->show();

		QTime start = QTime::currentTime();
		{
			for (int i = 0; i < totalCount; ++i) {
				model->addMessage(false, msgFrom);
				model->addMessage(true, msgTo);
			}
		}
		int time1 = start.msecsTo(QTime::currentTime());

		start = QTime::currentTime();
		{
			qApp->processEvents();
		}
		int time2 = start.msecsTo(QTime::currentTime());

		qWarning("YaChatView: %d + %d = %d", time1, time2, time1 + time2);
	}
#endif

#ifdef DO_QTEXTEDIT
	{
		PsiTextView* textEdit = new PsiTextView(0);
		textEdit->show();

		QTime start = QTime::currentTime();
		{
			for (int i = 0; i < totalCount; ++i) {
				textEdit->appendText(
					formatStanza(
						true,
						TIMESTAMP,
						formatTimeStamp(QDateTime::currentDateTime()),
						whoFrom,
						msgFrom
					)
				);

				textEdit->appendText(
					formatStanza(
						true,
						TIMESTAMP,
						formatTimeStamp(QDateTime::currentDateTime()),
						whoTo,
						msgTo
					)
				);
			}
		}
		int time1 = start.msecsTo(QTime::currentTime());

		start = QTime::currentTime();
		{
			qApp->processEvents();
		}
		int time2 = start.msecsTo(QTime::currentTime());

		qWarning("QTextEdit: %d + %d = %d", time1, time2, time1 + time2);
	}
#endif

	return app.exec();
}
