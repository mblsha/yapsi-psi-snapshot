#include <QApplication>
#include <QVBoxLayout>

#include "trimmablemultilinelabel.h"
#include "ui_main.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	QWidget widget;
	Ui::Main ui;
	ui.setupUi(&widget);

	ui.label->setMinNumLines(1);
	ui.label->setMaxNumLines(4);

	widget.resize(300, 300);
	widget.show();
	return app.exec();
}
