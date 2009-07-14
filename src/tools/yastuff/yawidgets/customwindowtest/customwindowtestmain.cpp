#include <QApplication>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>

#include "yawindow.h"
#include "ui_mywindow.h"

class MyWindow : public YaWindow
{
	Q_OBJECT
public:
	MyWindow()
		: YaWindow()
	{
		ui_.setupUi(this);
		connect(ui_.ck_customWindowMode, SIGNAL(toggled(bool)), SLOT(setCustomMode(bool)));
		ui_.ck_customWindowMode->setChecked(true);

		QTimer* timer = new QTimer(this);
		timer->setInterval(1000);
		timer->setSingleShot(false);
		connect(timer, SIGNAL(timeout()), SLOT(updateDebugInfo()));
		// timer->start();
	}

public slots:
	void setCustomMode(bool c) {
		setMode(c ? CustomWindowBorder : SystemWindowBorder);
	}

	void updateDebugInfo() {
		appendLog(QString("isActiveWindow = %1; isMinimized = %2").arg(isActiveWindow()).arg(isMinimized()));
	}

private:
	Ui::MyWindow ui_;

	void appendLog(const QString& message)
	{
		QDateTime now = QDateTime::currentDateTime();
		QString text = QString("[%1] %2").arg(now.toString("hh:mm:ss")).arg(message);
		ui_.textEdit->insertHtml(text);
		ui_.textEdit->insertHtml("<br>");
		ui_.textEdit->verticalScrollBar()->setValue(ui_.textEdit->verticalScrollBar()->maximum());
	}
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);

	MyWindow ya;
	ya.move(100, 100);
	ya.show();

	return app.exec();
}

#include "customwindowtestmain.moc"
