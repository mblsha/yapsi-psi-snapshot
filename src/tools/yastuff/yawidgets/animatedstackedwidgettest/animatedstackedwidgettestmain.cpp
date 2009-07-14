#include <QApplication>

#include "ui_animatedstackedwidgettest.h"

class AnimatedStackedWidgetTest : public QWidget
{
	Q_OBJECT
public:
	AnimatedStackedWidgetTest()
	{
		ui_.setupUi(this);
		ui_.stackedWidget->init();

		ui_.stackedWidget->setAnimationStyle(AnimatedStackedWidget::Animation_Push_Horizontal);
		ui_.stackedWidget->setWidgetPriority(ui_.redPage, 0);
		ui_.stackedWidget->setWidgetPriority(ui_.bluePage, 1);

		connect(ui_.redButton, SIGNAL(clicked()), SLOT(red()));
		connect(ui_.blueButton, SIGNAL(clicked()), SLOT(blue()));
	}

public slots:
	void red() { ui_.stackedWidget->setCurrentWidget(ui_.redPage); }
	void blue() { ui_.stackedWidget->setCurrentWidget(ui_.bluePage); }

private:
	Ui::AnimatedStackedWidgetTest ui_;
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	AnimatedStackedWidgetTest test;
	test.show();
	return app.exec();
}

#include "animatedstackedwidgettestmain.moc"
