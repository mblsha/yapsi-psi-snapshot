#include "testmodel.h"

TestModel::TestModel(QObject* parent)
	: AccountInformerModel(parent)
{
	addAccount("mblsha@ya.ru", XMPP::Status::Online, true);
	addAccount("mblsha@jabber.ru", XMPP::Status::DND, true);
	// for (int i = 0; i < 50; i++)
		addAccount("mblsha@livejournal.com", XMPP::Status::Offline, false);
}

QModelIndex TestModel::addAccount(const QString& jid, XMPP::Status::Type status, bool enabled)
{
	QStandardItem* item = new QStandardItem(jid);
	item->setData(QVariant(enabled), EnabledRole);
	item->setData(QVariant(status), StatusTypeRole);
	item->setData(QVariant(QString("123")), PasswordRole);

	int rowNum = invisibleRootItem()->rowCount();
	invisibleRootItem()->insertRow(rowNum, item);

	return item->index();
}
