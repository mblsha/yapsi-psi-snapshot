#ifndef PSICHATDLG_H
#define PSICHATDLG_H

#include "minicmd.h"
#include "mcmdsimplesite.h"
#include "mcmdmanager.h"
#include "chatdlg.h"
#include "mcmdcompletion.h"

#include "ui_chatdlg.h"

class IconAction;

class PsiChatDlg : public ChatDlg
{
	Q_OBJECT
public:
	PsiChatDlg(const Jid& jid, PsiAccount* account, TabManager* tabManager);

protected:
	// reimplemented
	void contextMenuEvent(QContextMenuEvent *);
	void doSend();
	bool eventFilter(QObject *obj, QEvent *event);


private:
	void setContactToolTip(QString text);

private slots:
	void toggleSmallChat();
	void doClearButton();
	void doMiniCmd();
	void buildMenu();
	void updateCounter();
	void updateIdentityVisibility();
	void updateCountVisibility();

	// reimplemented
	void chatEditCreated();

private:
	void initToolBar();
	void initToolButtons();

	// reimplemented
	void initUi();
	void capsChanged();
	bool isEncryptionEnabled() const;
	void contactUpdated(UserListItem* u, int status, const QString& statusString);
	void updateAvatar();
	void optionsUpdate();
	void updatePGP();
	void setPGPEnabled(bool enabled);
	void activated();
	void setLooks();
	void setShortcuts();
	QString colorString(bool local, SpooledType spooled) const;
	void appendSysMsg(const QString &);
	void appendEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt);
	void appendNormalMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt);
	void appendMessageFields(const Message& m);
	void updateLastMsgTime(QDateTime t);

	// reimplemented
	virtual ChatViewClass* chatView() const;
	virtual ChatEdit* chatEdit() const;

private:
	Ui::ChatDlg ui_;

	QMenu* pm_settings_;

	IconAction* act_clear_;
	IconAction* act_history_;
	IconAction* act_info_;
	IconAction* act_pgp_;
	IconAction* act_icon_;
	IconAction* act_file_;
	IconAction* act_compact_;
	IconAction* act_voice_;

	QAction *act_mini_cmd_;

	MCmdManager mCmdManager_;
	MCmdSimpleSite mCmdSite_;

	MCmdTabCompletion tabCompletion;

	bool smallChat_;
	QDateTime lastMsgTime_;
	class ChatDlgMCmdProvider;
};

#endif
