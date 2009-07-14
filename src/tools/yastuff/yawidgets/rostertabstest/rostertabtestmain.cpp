#include <QApplication>

#include "yatoolbox.h"
#include "yatoolboxpage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class TabButton : public YaToolBoxPageButton
{
	Q_OBJECT
public:
	TabButton()
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setMargin(0);
		layout->setSpacing(0);

		QPushButton* button = new QPushButton("Click me!", this);
		layout->addWidget(button);
		connect(button, SIGNAL(clicked()), SIGNAL(setCurrentPage()));
	}
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	YaToolBox tabs(0);
	tabs.addPage(new TabButton(), new QLabel("Woohoo!"));
	tabs.addPage(new TabButton(), new QLabel("Woohoo!"));
	tabs.addPage(new TabButton(), new QLabel("Woohoo!"));
	tabs.show();
	return app.exec();
}

#include "rostertabtestmain.moc"
