#include "testmodel.h"

#include "contactlistmodel.h"
#include "xmpp_status.h"

TestModel::TestModel()
	: QStandardItemModel()
{
	addContact(0, "Николай Яремко", "Всё, поехал домой", XMPP::Status::Online);
	addContact(1, "BoBuK", "", XMPP::Status::Online);
	addContact(2, "Рома Иванов", "Хочу жить в прекрасном светлом будущем", XMPP::Status::Online);
	addContact(3, "Андрей Себрант", "опять глючит... :-)", XMPP::Status::DND);
	addContact(4, "Василий Галоперидолович", "только не мой мозг!", XMPP::Status::DND);
	addContact(5, "Галочка Цветочек", "нет на месте", XMPP::Status::Away);
	addContact(6, "urbansheep", "нет на месте", XMPP::Status::Away);
	addContact(7, "Grey Evil", "", XMPP::Status::Offline);
}

void TestModel::addContact(int index, const char* name, const char* statusText, XMPP::Status::Type statusType)
{
	QPixmap pix;
	pix = QPixmap(QString(":userpics/%1.jpg").arg(index));
	if (pix.isNull())
		pix = QPixmap(QString(":userpics/%1.png").arg(index));
	QIcon icon(pix);

	QStandardItem* parentItem = invisibleRootItem();
	QStandardItem* item = new QStandardItem(QString::fromUtf8(name));
	item->setData(QVariant(icon), ContactListModel::PictureRole);
	item->setData(QVariant(statusType), ContactListModel::StatusTypeRole);
	item->setData(QVariant(QString::fromUtf8(statusText)), ContactListModel::StatusTextRole);
	parentItem->appendRow(item);
}
