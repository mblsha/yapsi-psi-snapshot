#include "testmodel.h"

#include "contactlistmodel.h"
#include "xmpp_status.h"

TestModel::TestModel()
	: QStandardItemModel()
{
	addInformer("Погода", "0.png", "Москва, +5°С", XMPP::Status::Online, "+5", "weather.ya.ru");
	addInformer("Почта",  "7.jpg", "42 непрочитанных сообщения", XMPP::Status::DND, "42", "mail.ya.ru");
	addInformer("Пробки", "2.jpg", "Нас тут нет", XMPP::Status::Offline, "", "probki.ya.ru");

	for (int i = 0; i < 5; ++i)
		addInformer("Погода - Бобруйск", "3.jpg", "Бобруйск, -15°С", XMPP::Status::Online, "-15", "weather.ya.ru");
}

void TestModel::addInformer(const char* name, const char* picture, const char* statusText, XMPP::Status::Type statusType, const char* value, const char* jid)
{
	QPixmap pix;
	pix = QPixmap(QString(":rostertest/userpics/%1").arg(picture));
	QIcon icon(pix);

	QStandardItem* parentItem = invisibleRootItem();
	QStandardItem* item = new QStandardItem(QString::fromUtf8(name));
	item->setData(QVariant(icon), ContactListModel::PictureRole);
	item->setData(QVariant(statusType), ContactListModel::StatusTypeRole);
	item->setData(QVariant(QString::fromUtf8(statusText)), ContactListModel::StatusTextRole);
	item->setData(QVariant(QString::fromUtf8(jid)), ContactListModel::JidRole);
	parentItem->appendRow(item);
}
