#include <QApplication>
#include <QWidget>
#include <QItemSelection>
#include <QMessageBox>

#include "testmodel.h"

#include "ui_accountinformertest.h"

class MyWidget : public QWidget
{
	Q_OBJECT
public:
	MyWidget()
	{
		ui_.setupUi(this);
		connect(ui_.addButton, SIGNAL(clicked()), SLOT(addAccount()));
		connect(ui_.deleteButton, SIGNAL(clicked()), SLOT(removeAccount()));

		connect(ui_.treeView, SIGNAL(selectionChanged(const QItemSelection&)), SLOT(selectionChanged(const QItemSelection&)));

		model_ = new AccountInformerModel(this);
		ui_.treeView->setModel(model_);
	}

private slots:
	void addAccount()
	{
		ui_.treeView->createNewAccount();
	}

	void removeAccount()
	{
		QModelIndex index = ui_.treeView->selectedIndex();
		if (index.isValid()) {
			if (model_->editMode(index) != AccountInformerModel::EditJID) {
				if (QMessageBox::question(this, tr("Deleting account"),
				                          tr("Do you really want to delete <b>%1</b>?").arg(index.data(Qt::DisplayRole).toString()),
				                          tr("Delete"), tr("Cancel"), QString::null, 1))
				{
					return;
				}
			}

			model_->removeRow(index.row(), index.parent());
		}
	}

	void selectionChanged(const QItemSelection& selected)
	{
		ui_.deleteButton->setEnabled(!selected.isEmpty());
	}

private:
	Ui::AccountInformerTest ui_;
	AccountInformerModel* model_;
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	MyWidget mw;
	mw.show();
	return app.exec();
}

#include "accountinformertestmain.moc"
