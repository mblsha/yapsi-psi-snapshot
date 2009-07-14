#include <QApplication>

#include "testmodel.h"
#include "ui_statusbartest.h"

class Main : public QWidget
{
	Q_OBJECT
public:
	Main()
	{
		statusBarTest.setupUi(this);

		TestModel* model = new TestModel();
		statusBarTest.statusBar->setModel(model);
	}

private:
	Ui::StatusBarTest statusBarTest;
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	Main main;

	main.show();
	return app.exec();
}

#include "statusbartestmain.moc"
