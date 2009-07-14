#include <QApplication>

#include "ui_settingstest.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);

	QWidget w;
	Ui::SettingsTest st;
	st.setupUi(&w);
	w.show();

	return app.exec();
}
