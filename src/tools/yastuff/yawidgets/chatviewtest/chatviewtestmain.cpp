#include <QApplication>

#include "yachatview.h"
#include "testmodel.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyleSheet(
	"QWidget {"
	"	font-family: \"Arial\";"
	"}"
	);

	YaChatView chatView(0);
	chatView.nicksChanged(QString::fromUtf8("foo"), QString::fromUtf8("bar"));
	chatView.setModel(new TestModel());
	chatView.show();
	chatView.resize(398, 900);
	return app.exec();
}
