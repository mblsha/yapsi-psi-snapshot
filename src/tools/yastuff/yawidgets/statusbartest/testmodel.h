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
	void addInformer(const char* name, const char* picture, const char* statusText, XMPP::Status::Type statusType, const char* value, const char* jid);
};

#endif
