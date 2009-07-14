#ifndef PSICONTACTMENU_H
#define PSICONTACTMENU_H

#include "contactlistitemmenu.h"

class PsiAccount;

class PsiAccountMenu : public ContactListItemMenu
{
	Q_OBJECT
public:
	PsiAccountMenu(PsiAccount* account, ContactListModel* model);
	~PsiAccountMenu();

private:
	class Private;
	Private* d;
};

#endif
