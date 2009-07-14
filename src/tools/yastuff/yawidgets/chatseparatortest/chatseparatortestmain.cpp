#include <QApplication>

#include <QVBoxLayout>
#include <QFrame>

#include "yachatseparator.h"
#include "iconset.h"
#include "yastyle.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyle(new YaStyle(app.style()));

	QWidget window;
	QVBoxLayout* vbox = new QVBoxLayout(&window);
	vbox->setMargin(0);
	vbox->setSpacing(0);

	QFrame* topFrame = new QFrame(&window);
	topFrame->setFrameShape(QFrame::NoFrame);
	topFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
	vbox->addWidget(topFrame);

	QFrame* bottomFrame = new QFrame(&window);
	bottomFrame->setFrameShape(QFrame::NoFrame);
	bottomFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	vbox->addWidget(bottomFrame);

	QVBoxLayout* vbox2 = new QVBoxLayout(bottomFrame);
	vbox2->setMargin(0);
	vbox2->setSpacing(0);

	YaChatSeparator* separator = new YaChatSeparator(bottomFrame);
	separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	vbox2->addWidget(separator);
	separator->show();

	QFrame* bottomFrame2 = new QFrame(bottomFrame);
	bottomFrame2->setMinimumSize(10, 30);
	bottomFrame2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	vbox2->addWidget(bottomFrame2);

	window.resize(200, 100);
	window.show();

	Iconset is;
	if (!is.load(":iconsets/emoticons/yaemo"))
		qWarning("unable to load iconset");;

	separator->setIconset(is);

	return app.exec();
}
