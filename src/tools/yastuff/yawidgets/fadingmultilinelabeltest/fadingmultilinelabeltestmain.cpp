#include <QApplication>

#include "fadingmultilinelabel.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	FadingMultilineLabel label(0);
	label.setMinimumSize(10, 10);
	label.setText("Moscowasdfasdadsfasdfasdf, ASDFasdflk ajsdfasdf, ASDfasdfsdfsdfsdfsdfsfdsdfsdf asdf sdfsdf sdfdfdf sdfdfdf, asdfasdfasfadfasdfasdfasdfsadfasdfsdfsdf.");
	label.setBackgroundColor(QColor("#a6ce39"));
	label.resize(100, 200);
	label.show();

	return app.exec();
}
