#include <QApplication>
#include <QDataStream>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDebug>
#include <QMap>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "yaautoreplacer.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	/*
	map["(club)"] = QString::fromUtf8("♣");
	map["(diamond)"] = QString::fromUtf8("♦");
	map["(heart)"] = QString::fromUtf8("♥");
	map["(spade)"] = QString::fromUtf8("♠");
	map["<br/>"] = QString::fromUtf8("™");
	*/

	YaAutoreplacer ya("/tmp/file.dat1");
	YaAutoreplace y = ya.process(QString::fromUtf8("Играем контракт 7(club)"));
	YaAutoreplace y1 = ya.process(QString::fromUtf8("Играем контракт 7(spade)"));
	YaAutoreplace y2 = ya.process(QString::fromUtf8("Играем контракт 7(spade)s"));
	qWarning() << y.replace() << ", " << y.pos() << ":" << y.count() << "isOK=" << ya.isOk();
	qWarning() << y1.replace() << ", " << y1.pos() << ":" << y1.count() << "isOK=" << ya.isOk();
	qWarning() << y2.replace() << ", " << y2.pos() << ":" << y2.count() << "isOK=" << ya.isOk();


	/*
	
	QFile fr("/tmp/file.dat");
	fr.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&fr);

	QString key, value;
	while (!reader.atEnd()) {
		QXmlStreamReader::TokenType type = reader.readNext();
		switch (type) {
			case QXmlStreamReader::StartDocument:
			case QXmlStreamReader::EndDocument:
			case QXmlStreamReader::EndElement:
				qWarning() << type << ": skipped";
				break;
			case QXmlStreamReader::StartElement:
				qWarning() << type << ": (!!) skipped";
				break;
			case QXmlStreamReader::Characters:
				key = reader.readElementText();
	            reader.readNext();
    	        value = reader.readElementText();
        	    qWarning() << key << "=" << value;
				break;
			default:
				qWarning() << type << " " << reader.tokenString() << " You!";
		}
	*/
		/*
		if (
			reader.readNext() == QXmlStreamReader::StartDocument &&
			reader.readNext() == QXmlStreamReader::StartDocument &&
			reader.readNext() != QXmlStreamReader::NoToken
		) {
			QString key = reader.tokenString();
			reader.readNext();
			QString value = reader.tokenString();
			qWarning() << key << "=" << value;
		}
		*/
	/*
	}
	if (reader.hasError()) {
       qWarning() << reader.errorString();
	}
	*/	



	return 0;
}

