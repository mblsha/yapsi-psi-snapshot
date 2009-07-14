#ifndef TESTMODEL_H
#define TESTMODEL_H

#include "yachatviewmodel.h"

class TestModel : public YaChatViewModel
{
	Q_OBJECT
public:
	TestModel();

private:
	void addMessage(bool incoming, const char* msg);
};

#endif
