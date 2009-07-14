#include <QApplication>

#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "yatabwidget.h"
#include "yatabbar.h"

class Page : public QWidget {
public:
	Page() {}

protected:
	void paintEvent(QPaintEvent*) {
		QPainter p(this);
		p.fillRect(rect(), Qt::white);
	}
};

class MyWidget : public QWidget {
	Q_OBJECT
public:
	MyWidget() {
		QVBoxLayout* vbox = new QVBoxLayout(this);
		vbox->setMargin(0);
		vbox->setSpacing(0);
		QHBoxLayout* hbox = new QHBoxLayout();
		vbox->addLayout(hbox);

		QPushButton* addTab = new QPushButton(this);
		addTab->setText("Add Tab");
		connect(addTab, SIGNAL(clicked()), SLOT(addTab()));
		hbox->addWidget(addTab);

		QPushButton* closeTab = new QPushButton(this);
		closeTab->setText("Close Tab");
		connect(closeTab, SIGNAL(clicked()), SLOT(closeTab()));
		hbox->addWidget(closeTab);

		tabs_ = new YaTabWidget(this);
		connect(tabs_, SIGNAL(closeTab(int)), SLOT(closeTab(int)));
		vbox->addWidget(tabs_);

		tabs_->setLayoutUpdatesEnabled(false);
		for (int i = 1; i <= 500; ++i) {
			this->addTab();
		}
		tabs_->setLayoutUpdatesEnabled(true);

		resize(500, 400);
	}

private slots:
	void addTab() {
		int i = tabs_->count() + 1;

		QWidget* widget = new Page();
		tabs_->addTab(widget, QPixmap(":/iconsets/roster/default/icon-online.png"), QString("Label %1").arg(i));

		// tabs_->setTabHighlighted(tabs_->indexOf(widget), i % 3 == 0);
	}

	void closeTab() {
		closeTab(tabs_->currentIndex());
	}

	void closeTab(int index) {
		Page* page = static_cast<Page*>(tabs_->widget(index));
		if (!page)
			return;

		Q_ASSERT(tabs_->indexOf(page) >= 0);
		tabs_->removeTab(tabs_->indexOf(page));
		delete page;
	}

private:
	YaTabWidget* tabs_;
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyleSheet(
	"QWidget {"
	"	font-family: \"Arial\";"
	"}"
	);

	MyWidget w;
	w.show();

	// QTimer::singleShot(0, &app, SLOT(quit()));

	return app.exec();
}

#include "tabbartestmain2.moc"
