#include "testmodel.h"

TestModel::TestModel()
	: YaChatViewModel()
{
	addMessage(false, "Hey dude, what are you doing tonight?");
	addMessage(false, "PING");
	addMessage(false, "WAKE UP1111!!!");

	addMessage(true, "wazzup?");
	addMessage(true, "you want smth?");

	addMessage(false, "You bet!\nI've been waiting for you since yesterday!!!");

	addMessage(true, "and what exactly, if i may ask?\nyou know i just woke up and fill a little bit dizzy\nneed to take some FreeCola™ first");

	addMessage(false, "How about a deathmatch in SuperTux?");
}

void TestModel::addMessage(bool incoming, const char* msg)
{
	YaChatViewModel::addMessage(incoming, QString::fromUtf8(msg));
}
