/***************************************************************************
 *   Copyright (©) 2006 by Яndex
 *   <a href="1st@1stone.ru">Алексей Матюшкин</a>
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>

#include "typographyhighlighter.h"
#include "cpphighlighter.h"
#include "wikihighlighter.h"
#include "listhighlighter.h"
#include "quotationhighlighter.h"
#include "combinedsyntaxhighlighter.h"

void retranslateUi(QDialog *Dialog)
{
	Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", 0, QApplication::UnicodeUTF8));
	Q_UNUSED(Dialog);
} // retranslateUi

void setupUi(QDialog *Dialog)
{
	Dialog->setObjectName(QString::fromUtf8("Dialog"));
	QVBoxLayout * vboxLayout = new QVBoxLayout(Dialog);
	vboxLayout->setSpacing(6);
	vboxLayout->setMargin(9);
	vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));

	QTextEdit * edit = new QTextEdit(Dialog);
	
	CombinedSyntaxHighlighter* hl = new CombinedSyntaxHighlighter(edit);
	new TypographyHighlighter(hl, edit);
	new WikiHighlighter(hl, edit);
	new ListHighlighter(hl, edit);
	new QuotationHighlighter(hl, edit);
	new CppHighlighter(hl, edit);

	edit->setObjectName(QString::fromUtf8("richedit"));
	edit->setText(QString::fromUtf8("«Обратите внимание, мы на лету типографируем кавычки в лапки-ёлочки и минусы в тире, причем работает /Undo/ по /Ctrl+Z/»,— сказал *жирный* /пошатывающийся/ _идиот_.\n\nВот, написал намедни, *зацени*:\n#c void main () {\n#c   return QString( \"6--\" ). toInt();\n#c }\n\n> А чего всё так цветастенько?\n  • я люблю цветы;\n  • ты нихрена не рубишь в эстетике!"));
	vboxLayout->addWidget(edit);

	retranslateUi(Dialog);

	QSize size(400, 300);
	size = size.expandedTo(Dialog->minimumSizeHint());
	Dialog->resize(size);

	QMetaObject::connectSlotsByName(Dialog);
} // setupUi

int main(int argc, char ** argv)
{
	QApplication a(argc, argv);
	QDialog * dlg = new QDialog();
	setupUi(dlg);
	dlg->exec();
	return 0;
}
