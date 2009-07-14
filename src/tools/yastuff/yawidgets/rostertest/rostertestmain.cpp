#include <QApplication>

#include "testmodel.h"
#include "ui_rostertest.h"

#include "yacontactlistview.h"
#include "../yacontactlistviewdelegate.h"
#include "../yacontactlistviewslimdelegate.h"
#include "../yacontactlistviewlargedelegate.h"

class Main : public QWidget
{
	Q_OBJECT
public:
	Main()
	{
		rosterTest.setupUi(this);
		connect(rosterTest.comboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateDelegate()));

		updateDelegate();

		TestModel* model = new TestModel();
		rosterTest.listView->setModel(model);

		rosterTest.comboBox->setCurrentIndex(1);
	}

public slots:
	void updateDelegate()
	{
		QAbstractItemDelegate* delegate = 0;
		switch (rosterTest.comboBox->currentIndex()) {
		case 0:
			delegate = new YaContactListViewSlimDelegate(rosterTest.listView);
			break;
		case 1:
			delegate = new YaContactListViewDelegate(rosterTest.listView);
			break;
		case 2:
			delegate = new YaContactListViewLargeDelegate(rosterTest.listView);
			break;
		default:
			delegate = new QItemDelegate(rosterTest.listView);
		}

		rosterTest.listView->setItemDelegate(delegate);
	}

private:
	Ui::RosterTest rosterTest;
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	Main main;

	main.show();
	return app.exec();
}

#include "rostertestmain.moc"
