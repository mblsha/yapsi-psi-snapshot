#include <QtTest/QtTest>

#define protected public
#define private public

#include "userlist.h"
// #include "profiles.h"
// #include "psiaccount.h"
#include "psicontact.h"
#include "psicontactlist.h"
#include "contactlistnestedgroup.h"
#include "contactlistmodel.h"

#undef protected
#undef private

class TestContactListNestedGroup: public QObject
{
	Q_OBJECT
private:
	PsiContactList* contactList;
	ContactListNestedGroup *root;
	ContactListModel* model;
	QVector<PsiContact*> contacts;

	void clearRoot()
	{
		delete model;
		model = new ContactListModel(contactList);
		model->setGroupsEnabled(true);
		model->invalidateLayout();

		root = dynamic_cast<ContactListNestedGroup*>(model->rootGroup_);
		Q_ASSERT(root);
	}

private slots:
	void initTestCase()
	{
		ContactListGroup::setGroupDelimiter("::");

		contactList = new PsiContactList(0);
		model = 0;
		root = 0;
		for (int i = 0; i < 1000; ++i)
			contacts += new PsiContact(UserListItem(), 0);
	}

	void cleanupTestCase()
	{
		delete model;
		delete contactList;
	}

	void testSingleAddAndRemove()
	{
		clearRoot();
		PsiContact* contact = contacts[0];
		QCOMPARE(root->itemsCount(), 0);

		model->addContact(contact, QStringList());
		QCOMPARE(root->itemsCount(), 0);

		model->addContact(contact, QStringList() << "");
		QCOMPARE(root->itemsCount(), 1);
		Q_ASSERT(root->findContact(contact) != 0);

		model->contactGroupsChanged(contact, QStringList());
		QCOMPARE(root->itemsCount(), 0);
	}

	void testReparent()
	{
		clearRoot();
		PsiContact* contact = contacts[0];
		QCOMPARE(root->itemsCount(), 0);

		model->addContact(contact, QStringList() << "Foo");
		QCOMPARE(root->itemsCount(), 1);
		QCOMPARE(root->findGroup("Foo")->itemsCount(), 1);

		model->contactGroupsChanged(contact, QStringList() << "Foo::Bar");
		QCOMPARE(root->itemsCount(), 1);
		QCOMPARE(root->findGroup("Foo")->itemsCount(), 1);
		QCOMPARE(((ContactListNestedGroup*)root->groups_[0])->findGroup("Bar")->itemsCount(), 1);
	}

	void testTwoGroups()
	{
		clearRoot();
		QCOMPARE(root->itemsCount(), 0);

		int numFriends  = 3;
		int numContacts = 10;

		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < numFriends; ++j)
				model->contactGroupsChanged(contacts[j], QStringList() << "Friends");

			for (int j = 0; j < numContacts; ++j) {
				model->contactGroupsChanged(contacts[j+numFriends], QStringList() << "Contacts");
			}
		}
		QCOMPARE(root->itemsCount(), 2);
		Q_ASSERT(root->findGroup("Friends") != 0);
		QCOMPARE(root->findGroup("Friends")->itemsCount(), numFriends);
		Q_ASSERT(root->findGroup("Contacts") != 0);
		QCOMPARE(root->findGroup("Contacts")->itemsCount(), numContacts);

		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < numFriends; ++j)
				model->contactGroupsChanged(contacts[j], QStringList() << "Friends" << "Contacts");
		}
		QCOMPARE(root->findGroup("Contacts")->itemsCount(), numFriends+numContacts);

		for (int j = 0; j < numFriends+numContacts; ++j)
			model->contactGroupsChanged(contacts[j], QStringList() << "Enemies");
		QCOMPARE(root->itemsCount(), 1);
		Q_ASSERT(root->findGroup("Friends") == 0);
		Q_ASSERT(root->findGroup("Contacts") == 0);
		QCOMPARE(root->findGroup("Enemies")->itemsCount(), numFriends+numContacts);

		for (int j = 0; j < numFriends+numContacts; ++j)
			model->contactGroupsChanged(contacts[j], QStringList() << "");
		QCOMPARE(root->itemsCount(), numFriends+numContacts);
		Q_ASSERT(root->findGroup("Enemies") == 0);
	}

	void testNestedGroups()
	{
		clearRoot();
		QCOMPARE(root->itemsCount(), 0);

		int numContacts = 100;

		for (int j = 0; j < numContacts; ++j) {
			model->contactGroupsChanged(contacts[j], QStringList() << "1::2");
		}
		QCOMPARE(root->itemsCount(), 1);
		QCOMPARE(root->findGroup("1")->itemsCount(), 1);
		QCOMPARE(((ContactListNestedGroup*)root->groups_[0])->findGroup("2")->itemsCount(), numContacts);

		for (int j = 0; j < numContacts; ++j) {
			QStringList groupName;
			for (int i = 0; i < j; ++i)
				groupName << QString::number(i);
			model->contactGroupsChanged(contacts[j], QStringList() << groupName.join(root->groupDelimiter()));
		}

		{
			int i = 0;
			ContactListNestedGroup* group = root;
			while (group) {
				if (++i < numContacts) {
					QCOMPARE(group->itemsCount(), 2);
					QCOMPARE(group->groups_.count(), 1);
					group = (ContactListNestedGroup*)group->groups_[0];
				}
				else {
					QCOMPARE(group->groups_.count(), 0);
					QCOMPARE(group->itemsCount(), 1);
					break;
				}
			}
		}

		for (int j = 0; j < numContacts; ++j) {
			model->contactGroupsChanged(contacts[j], QStringList() << "1" << "2" << "3");
		}

		QCOMPARE(root->groups_.count(), 3);
		QCOMPARE(root->itemsCount(), 3);
		QCOMPARE(root->findGroup("1")->itemsCount(), numContacts);
		QCOMPARE(root->findGroup("2")->itemsCount(), numContacts);
		QCOMPARE(root->findGroup("3")->itemsCount(), numContacts);

		for (int j = 0; j < numContacts; ++j) {
			model->contactGroupsChanged(contacts[j], QStringList() << " 1 ::   2  3  ");
		}
		QCOMPARE(root->itemsCount(), 1);
		QCOMPARE(root->findGroup(" 1 ")->itemsCount(), 1);
		QCOMPARE(((ContactListNestedGroup*)root->groups_[0])->findGroup("   2  3  ")->itemsCount(), numContacts);

		for (int j = 0; j < numContacts; ++j) {
			model->contactGroupsChanged(contacts[j], QStringList() << "1::");
		}
		QCOMPARE(root->itemsCount(), 1);
		QCOMPARE(root->findGroup("1")->itemsCount(), numContacts);

		// forever loop in current version
		for (int j = 0; j < numContacts; ++j) {
			model->contactGroupsChanged(contacts[j], QStringList() << "::1::2");
		}
		QCOMPARE(root->itemsCount(), 1);

		for (int j = 0; j < numContacts; ++j) {
			model->contactGroupsChanged(contacts[j], QStringList() << "1::::2");
		}
		QCOMPARE(root->itemsCount(), 1);
	}
};

QTEST_MAIN(TestContactListNestedGroup)
#include "testcontactlistnestedgroup.moc"
