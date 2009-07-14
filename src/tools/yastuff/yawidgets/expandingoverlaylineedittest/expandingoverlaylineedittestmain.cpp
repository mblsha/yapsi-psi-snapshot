#include <QApplication>

#include "ui_test.h"

#include "yaexpandingoverlaylineedit.h"

class Main : public QWidget
{
	Q_OBJECT
public:
	Main()
	{
		ui_.setupUi(this);
		lineEdit_ = new YaExpandingOverlayLineEdit(this);
		lineEdit_->setController(ui_.pushButton);
		lineEdit_->addToGrounding(ui_.pushButton_2);
		lineEdit_->addToGrounding(ui_.pushButton_3);
		lineEdit_->setEmptyText("email@ya.ru");

		connect(ui_.pushButton, SIGNAL(toggled(bool)), lineEdit_, SLOT(setVisible(bool)));
		connect(lineEdit_, SIGNAL(cancelled()), ui_.pushButton, SLOT(toggle()));
		connect(lineEdit_, SIGNAL(enteredText(const QString &)), ui_.pushButton, SLOT(toggle()));

		lineEdit2_ = new YaExpandingOverlayLineEdit(this);
		lineEdit2_->setController(ui_.pushButton_6);
		lineEdit2_->addToGrounding(ui_.pushButton_4);
		lineEdit2_->addToGrounding(ui_.pushButton_5);

		connect(ui_.pushButton_6, SIGNAL(toggled(bool)), lineEdit2_, SLOT(setVisible(bool)));
		connect(lineEdit2_, SIGNAL(cancelled()), ui_.pushButton_6, SLOT(toggle()));
		connect(lineEdit2_, SIGNAL(enteredText(const QString &)), ui_.pushButton_6, SLOT(toggle()));
	}


private:
	Ui::Test ui_;
	YaExpandingOverlayLineEdit* lineEdit_;
	YaExpandingOverlayLineEdit* lineEdit2_;
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	Main main;
	main.show(); 
	return app.exec();
}

#include "expandingoverlaylineedittestmain.moc"
