#ifndef TESTMODEL_H
#define TESTMODEL_H

#include "accountinformermodel.h"

#include <QStandardItemModel>

#include "xmpp_status.h"

class TestModel : public AccountInformerModel
{
	Q_OBJECT
public:
	TestModel(QObject* parent);

private:
	QModelIndex addAccount(const QString& jid, XMPP::Status::Type status, bool enabled);
};

#endif
