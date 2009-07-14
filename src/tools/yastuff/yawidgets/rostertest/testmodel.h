#ifndef TESTMODEL_H
#define TESTMODEL_H

#include <QStandardItemModel>

#include "xmpp_status.h"

class TestModel : public QStandardItemModel
{
	Q_OBJECT
public:
	TestModel();

private:
	void addContact(int index, const char* name, const char* statusText, XMPP::Status::Type statusType);
};

#endif
