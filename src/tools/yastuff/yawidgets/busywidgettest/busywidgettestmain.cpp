#include <QApplication>

#include "ui_busywidgettest.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	QWidget w;
	Ui::BusyWidgetTest bwt;
	bwt.setupUi(&w);
	QObject::connect(bwt.checkBox, SIGNAL(toggled(bool)), bwt.widget, SLOT(setActive(bool)));
	QObject::connect(bwt.checkBox, SIGNAL(toggled(bool)), bwt.widget_2, SLOT(setActive(bool)));
	w.show();
	return app.exec();
}
