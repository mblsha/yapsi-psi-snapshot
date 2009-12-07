/*
 * yacommon.cpp - common YaPsi routines
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "yacommon.h"

#include <QPixmap>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QTemporaryFile>
#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include "xmpp_jid.h"
#include "avatars.h"
#include "eventdb.h"
#include "desktoputil.h"
#include "jidutil.h"
#include "textutil.h"
#include "userlist.h"
#include "psiaccount.h"
#include "psioptions.h"
#include "psievent.h"
#include "visibletext.h"
#include "yacontactlistviewdelegate.h"
#include "yavisualutil.h"
#include "yapsi_revision.h"
#include "common.h"
#include "iconset.h"
#include "psicontact.h"
#include "psicon.h"
#include "psicontactlist.h"

const static QString S_ELLIPSIS = QString::fromUtf8("…");
const static QString S_HISTORY_IS_EMPTY = QString::fromUtf8("<h2>История общения пуста ☹</h2>");
const static QString STR_YAPSI_HISTORY = QString::fromUtf8("chats_with_%1_at_%2.html"); // No I18N !!

const QString Ya::INFORMERS_GROUP_NAME = QString::fromUtf8("Яндекс.Информеры");
const QStringList Ya::BOTS_GROUP_NAMES = QStringList() << Ya::INFORMERS_GROUP_NAME;

const QString Ya::TIMESTAMP = "#c7c7c7";
const QString Ya::MESAYS = "#DDA46D";
const QString Ya::HESAYS = "#13A9E8";
const QString Ya::SPOOLED = "#660000";

static QString processFromDirName(const QString& path, const QString& fromPath)
{
	QString result = path;
	result.replace(fromPath, QString());
	if (result.startsWith("/")) {
		result.remove(0, 1);
	}
	return result;
}

bool Ya::copyDir(const QString& _fromPath, const QString& toPath)
{
	QFileInfo fromDirInfo(_fromPath);
	QString fromPath = fromDirInfo.absoluteFilePath();
	if (fromDirInfo.isSymLink()) {
		fromPath = fromDirInfo.symLinkTarget();
	}

	QDir toDir(toPath);

	QDirIterator it(fromPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
	while (it.hasNext()) {
		it.next();

		QString path = it.fileInfo().absolutePath();
		if (!path.startsWith(fromPath)) {
			continue;
		}

		path = processFromDirName(path, fromPath);
		QString fileName = processFromDirName(it.filePath(), fromPath);

		if (path.isEmpty())
			toDir.mkpath(".");
		else
			toDir.mkpath(path);

		QString newFileName = toDir.filePath(fileName);
		if (it.fileInfo().isFile() && !QFile::exists(newFileName)) {
			QFile::copy(it.filePath(), newFileName);
		}
	}

	return true;
}

const QString& Ya::ellipsis()
{
	return S_ELLIPSIS;
}

QPixmap Ya::groupPixmap(QSize size, bool open, DecorationState state)
{
	QString group = open ? "open" : "closed";
	QString s;
	if (state == Hover)
		s = "hover";
	else
		s = "normal";
	return QPixmap(QString(":/images/group/%1_%2.png").arg(group, s));
}

QPixmap Ya::genderPixmap(XMPP::VCard::Gender gender)
{
	QString fn = "gn";
	if (gender == XMPP::VCard::Male)
		fn = "gm";
	else if (gender == XMPP::VCard::Female)
		fn = "gw";
	return QPixmap(QString(":/images/gender/%1.gif").arg(fn));
}

// adapted from XBMC/xbmc/utils/CharsetConverter.cpp (GPL)
static bool isValidUtf8(const QByteArray& ba)
{
	const char* buf = ba.constData();
	unsigned int len = ba.length();
	const unsigned char *endbuf = (unsigned char*)buf + len;
	unsigned char byte2mask = 0x00, c;
	int trailing = 0; // trailing (continuation) bytes to follow

	while ((unsigned char*)buf != endbuf) {
		c = *buf++;
		if (trailing) {
			if ((c & 0xc0) == 0x80) { // does trailing byte follow UTF-8 format ?
				if (byte2mask) {  // need to check 2nd byte for proper range
					if (c & byte2mask) // are appropriate bits set ?
						byte2mask = 0x00;
					else
						return false;
				}
				trailing--;
			}
			else {
				return 0;
			}
		}
		else {
			if ((c & 0x80) == 0x00) continue; // valid 1-byte UTF-8
			else if ((c & 0xe0) == 0xc0)      // valid 2-byte UTF-8
				if (c & 0x1e)             //is UTF-8 byte in proper range ?
					trailing = 1;
				else
					return false;
			else if ((c & 0xf0) == 0xe0) {    // valid 3-byte UTF-8
				if (!(c & 0x0f))          // is UTF-8 byte in proper range ?
					byte2mask = 0x20; // if not set mask
				trailing = 2;             // to check next byte
			}
			else if ((c & 0xf8) == 0xf0) {    // valid 4-byte UTF-8
				if (!(c & 0x07))          // is UTF-8 byte in proper range ?
					byte2mask = 0x30; // if not set mask
				trailing = 3;             // to check next byte
			}
			else if ((c & 0xfc) == 0xf8) {    // valid 5-byte UTF-8
				if (!(c & 0x03))          // is UTF-8 byte in proper range ?
					byte2mask = 0x38; // if not set mask
				trailing = 4;             // to check next byte
			}
			else if ((c & 0xfe) == 0xfc) {    // valid 6-byte UTF-8
				if (!(c & 0x01))          // is UTF-8 byte in proper range ?
					byte2mask = 0x3c; // if not set mask
				trailing = 5;             // to check next byte
			}
			else {
				return false;
			}
		}
	}
	return trailing == 0;
}

QString Ya::ljUtf8Hack(const XMPP::Jid& jid, const QString& orig)
{
	if (jid.domain() != "livejournal.com")
		return orig;

	QTextCodec* latin1 = QTextCodec::codecForName("latin1");
	if (latin1->canEncode(orig)) {
		QByteArray ba = latin1->fromUnicode(orig);
		if (isValidUtf8(ba)) {
			return QString::fromUtf8(ba);
		}
	}

	return orig;
}

QString Ya::nickFromVCard(const XMPP::Jid& jid, const XMPP::VCard* vcard)
{
	QString nick = jid.user();
	if (vcard) {
		if (!vcard->nickName().isEmpty()) {
			nick = vcard->nickName();
		}
		else if (!vcard->fullName().isEmpty()) {
			nick = vcard->fullName();
		}
	}
	return nick;
}

bool Ya::isYaInformer(PsiEvent* event)
{
	if (event->type() != PsiEvent::Message)
		return false;
	QStringList informers;
	informers
	  << "informer.ya.ru"
	  << "mail.ya.ru"
	  << "probki.ya.ru"
	  << "weather.ya.ru";
	return informers.contains(event->from().host());
}

bool Ya::isYaJid(const XMPP::Jid& jid)
{
	return jid.host() == "ya.ru";
}

bool Ya::isYandexTeamJid(const XMPP::Jid& jid)
{
	return jid.host() == "yandex-team.ru";
}

bool Ya::historyAvailable(PsiAccount* account, const XMPP::Jid& jid)
{
	return true;
#if 0
	return EDBFlatFile::File::historyExists(account, jid);
#endif
}

bool Ya::useMaleMessage(XMPP::VCard::Gender gender)
{
	return gender != XMPP::VCard::Female;
}

QString Ya::statusFullName(XMPP::Status::Type status, XMPP::VCard::Gender gender)
{
	switch (status) {
	case XMPP::Status::Offline:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "Offline", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "Offline", "contact is female");
	case XMPP::Status::XA:
	case XMPP::Status::Away:
		return QCoreApplication::instance()->translate("Ya", "Away");
	case XMPP::Status::DND:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "DnD", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "DnD", "contact is female");
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "Online", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "Online", "contact is female");
	case XMPP::Status::Invisible:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "Invisible", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "Invisible", "contact is female");
	}

	Q_ASSERT(false);
	return QString();
}

QString Ya::statusName(XMPP::Status::Type status, XMPP::VCard::Gender gender)
{
	QString result = Ya::statusFullName(status, gender).toLower();
	switch (status) {
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
	case XMPP::Status::Offline:
	case XMPP::Status::Invisible:
		result = QString();
		break;
	default:
		break;
	}

	return result;
}

QString Ya::statusDescription(XMPP::Status::Type type, XMPP::VCard::Gender gender)
{
	switch (type) {
	case XMPP::Status::Offline:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "offline", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "offline", "contact is female");
	case XMPP::Status::Blocked:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "blocked", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "blocked", "contact is female");
	case XMPP::Status::Reconnecting:
		return QCoreApplication::instance()->translate("Ya", "reconnecting");
	case XMPP::Status::NotAuthorizedToSeeStatus:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "not authorized", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "not authorized", "contact is female");
	case XMPP::Status::XA:
	case XMPP::Status::Away:
		return QCoreApplication::instance()->translate("Ya", "away");
	case XMPP::Status::DND:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "do not disturb", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "do not disturb", "contact is female");
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
	case XMPP::Status::Invisible:
		return Ya::useMaleMessage(gender) ?
		       QCoreApplication::instance()->translate("Ya", "online", "contact is male") :
		       QCoreApplication::instance()->translate("Ya", "online", "contact is female");
	}
	Q_ASSERT(false);
	return QString();
}

QString Ya::statusToFlash(XMPP::Status::Type type)
{
	switch (type) {
	case XMPP::Status::Offline:
		return "offline";
	case XMPP::Status::Blocked:
		return "blocked";
	case XMPP::Status::Reconnecting:
		return "reconnecting";
	case XMPP::Status::NotAuthorizedToSeeStatus:
		return "notauthorizedtoseestatus";
	case XMPP::Status::XA:
	case XMPP::Status::Away:
		return "away";
	case XMPP::Status::DND:
		return "dnd";
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
	case XMPP::Status::Invisible:
		return "online";
	}
	Q_ASSERT(false);
	return "offline";
}

QColor Ya::statusColor(XMPP::Status::Type status)
{
	return Ya::VisualUtil::statusColor(status, false);
}

QString Ya::normalizeStanza(QString txt)
{
	/*
	// bloody kopete inserts superfluous paras
	QRegExp rx("\\s*<span\\s+xmlns=\".*\">\\s*<p\\s+style=\".*\"\\s*>(.*)</p>\\s*</span>\\s*");
	if (rx.indexIn(txt) > -1) {
		txt = rx.cap(1);
	}
	// bloody pandeon inserts superfluous spans
	QRegExp rx("\\s*<span\\s+xmlns=\".*\">\\s*<span\\s+style=\".*\"\\s*>(.*)</span>\\s*</span>\\s*");
	if (rx.indexIn(txt) > -1) {
		txt = rx.cap(1);
	}
	*/
	return txt;
}

QString Ya::visibleText(const QString& fullText, const QFontMetrics& fontMetrics, int width, int height, int numLines)
{
	return ::visibleText(fullText, fontMetrics, width, height, numLines);
}

QRect Ya::circle_bounds(const QPointF &center, double radius, double compensation)
{
	return QRect(
	           qRound(center.x() - radius - compensation),
	           qRound(center.y() - radius - compensation),
	           qRound((radius + compensation) * 2),
	           qRound((radius + compensation) * 2)
	       );
}

const QString Ya::truncate(const QString &s, int len)
{
	return len <= 0 || len >= s.length() ? s : s.left(len - ellipsis().length()) + ellipsis();
}

bool Ya::isSubscriptionRequest(PsiEvent* event)
{
	return dynamic_cast<AuthEvent*>(event) && dynamic_cast<AuthEvent*>(event)->authType() == "subscribe";
}

QList<Ya::SpooledMessage> Ya::lastMessages(const PsiAccount* me, const XMPP::Jid& interlocutor, unsigned int count)
{
	QList<Ya::SpooledMessage> result;

	EDBHandle exp(me->edb());
	exp.getLatest(me, interlocutor, count);

	while (exp.busy()) {
		QCoreApplication::instance()->processEvents();
	}

	const EDBResult* r = exp.result();
	if (r && r->count() > 0) {
		Q3PtrListIterator<EDBItem> it(*r);
		it.toLast();
		for (; it.current(); --it) {
			PsiEvent *e = it.current()->event();

			if (e->type() == PsiEvent::Message) {
				MessageEvent* me = static_cast<MessageEvent*>(e);
				result << Ya::SpooledMessage(me->originLocal(), me->timeStamp(), me->message());
			}
			else if (e->type() == PsiEvent::Mood) {
				MoodEvent* me = static_cast<MoodEvent*>(e);
				result << Ya::SpooledMessage(me->timeStamp(), me->mood());
			}
		}
	}

	return result;
}

static QList<Ya::SpooledMessage> messagesFor(const PsiAccount* me, XMPP::Jid interlocutor)
{
	QList<Ya::SpooledMessage> result;

	EDBHandle exp(me->edb());
	QString id;

	while (true) {
		exp.get(me, interlocutor, id, EDB::Forward, 1000);

		while (exp.busy()) {
			QCoreApplication::instance()->processEvents();
		}

		const EDBResult* r = exp.result();
		if (r && r->count() > 0) {
			Q3PtrListIterator<EDBItem> it(*r);
			for (; it.current(); ++it) {
				id = it.current()->nextId();
				PsiEvent *e = it.current()->event();

				if (e->type() == PsiEvent::Message) {
					MessageEvent* me = static_cast<MessageEvent*>(e);
					result << Ya::SpooledMessage(me->originLocal(), me->timeStamp(), me->message());
				}
				else if (e->type() == PsiEvent::Mood) {
					MoodEvent* me = static_cast<MoodEvent*>(e);
					result << Ya::SpooledMessage(me->timeStamp(), me->mood());
				}
				else if (e->type() == PsiEvent::Auth) {
					AuthEvent* ae = static_cast<AuthEvent*>(e);
					XMPP:Message message;
					message.setBody(ae->description());
					result << Ya::SpooledMessage(ae->originLocal(), ae->timeStamp(), message);
				}
			}
		}
		else {
			break;
		}

		if (id.isEmpty()) {
			break;
		}
	}

	return result;
}

const void Ya::showHistory(const PsiAccount* me, const XMPP::Jid& interlocutor)
{
	const QString baseHistoryUrl = "http://mail.yandex.ru/history?yasoft=online";
	if (!me) {
		Q_ASSERT(interlocutor.isNull());
		DesktopUtil::openYaUrl(baseHistoryUrl);
		return;
	}
	// ONLINE-2097 ONLINE-2105
	// DesktopUtil::openYaUrl(QString("http://clck.yandex.ru/redir/dtype=stred/pid=135/cid=2435/*http://mail.yandex.ru/history#%1").arg(interlocutor.bare()));
	DesktopUtil::openYaUrl(QString("%1#%2")
	                       .arg(baseHistoryUrl)
	                       .arg(interlocutor.bare()));
}

QString Ya::limitText(const QString& text, int limit)
{
	if (text.length() > limit) {
		return text.left(limit) + "...";
	}
	return text;
}

QString Ya::messageNotifierText(const QString& messageText)
{
	if (!PsiOptions::instance()->getOption("options.ya.popups.message.show-text").toBool()) {
		return QObject::tr("New message.");
	}

	return messageText;
}

static QRegExp yandexHostnameRegExp()
{
	return QRegExp("@((narod|yandex|ya)\\.(ru|ua)|yandex\\.(com|kz|by))$");
}

QString Ya::yaRuAliasing(const QString& jid)
{
	QString tmp = jid;
	tmp.replace(yandexHostnameRegExp(), "@ya.ru");

	XMPP::Jid j(tmp);
	if (isYaJid(j)) {
		QString node = j.node();
		node.replace(".", "-");
		j.setNode(node);
		return j.full();
	}

	return tmp;
}

QString Ya::stripYaRuHostname(const QString& jid)
{
	QString tmp = jid;
	tmp.replace(yandexHostnameRegExp(), "");
	return tmp;
}

PsiContact* Ya::findContact(PsiCon* controller, const QString& jid)
{
	if (controller) {
		XMPP::Jid j(jid);
		j = j.bare();
		foreach(PsiAccount* account, controller->contactList()->enabledAccounts()) {
			PsiContact* contact = account->findContact(j);
			if (contact) {
				return contact;
			}
		}
	}
	return 0;
}

const QString Ya::colorString(bool local, bool spooled)
{
	return spooled ? SPOOLED : local ? MESAYS : HESAYS;
}

QString Ya::createUniqueName(QString baseName, QStringList existingNames)
{
	int index = 1;
	bool found = true;
	QString result;
	while (found) {
		result = baseName;
		if (index > 1)
			result += QString(" %1").arg(index);

		found = existingNames.contains(result);

		if (found) {
			++index;
		}
	}

	return result;
}

QString Ya::contactName(const QString& name, const QString& jid)
{
	if (name == jid) {
		XMPP::Jid jid(name);
		return jid.node();
	}

	return name;
}

QString Ya::contactName(const PsiAccount* account, const XMPP::Jid& jid)
{
	QString result = jid.bare();
	Q_ASSERT(account);
	PsiContact* contact = account ? account->findContact(jid) : 0;
	if (contact) {
		result = contact->name();
	}
	return Ya::contactName(result, jid.full());
}

QString Ya::emoticonToolTipSimple(const PsiIcon* icon, const QString& _text)
{
	Q_ASSERT(icon);
	QString text = _text.isEmpty() ? icon->defaultText() : _text;
	QString result;
	// if (icon->description().isEmpty())
		result = text;
	// else
	// 	result = QString("%1 %2").arg(icon->description(), text);
	return result;
}

QString Ya::emoticonToolTip(const PsiIcon* icon, const QString& _text)
{
	return "<div style='white-space:pre'>" + TextUtil::plain2rich(emoticonToolTipSimple(icon, _text)) + "</div>";
}

/**
 * Filters out moods that come from the automatic statuses, and also blocks
 * away statuses since they're automatic 99% of the time
 */
QString Ya::processMood(const QString& oldMood, const QString& newMood, XMPP::Status::Type statusType)
{
	static QStringList autoStatus;
	if (autoStatus.isEmpty()) {
		autoStatus << QString::fromUtf8("Auto Status (idle)");
		autoStatus << QString::fromUtf8("Авто состояние (по бездействию)");
		autoStatus << QString::fromUtf8("Yep, I'm here.");
		autoStatus << QString::fromUtf8("Да, я здесь.");
		autoStatus << QString::fromUtf8("Available");
		autoStatus << QString::fromUtf8("Меня нет на месте");
		autoStatus << QString::fromUtf8("Not available as a result of being idle");
		autoStatus << QString::fromUtf8("Автоматический статус: меня нет на месте");
		autoStatus << QString::fromUtf8("Отсутствую более 5 минут");
		autoStatus << QString::fromUtf8("I'm not here right now");
	}

	if (autoStatus.contains(newMood)) {
		// qWarning("processMood: autoStatus: %s", qPrintable(newMood));
		return oldMood;
	}

	if (statusType == XMPP::Status::Away ||
	    statusType == XMPP::Status::XA   ||
	    statusType == XMPP::Status::Offline)
	{
		// qWarning("processMood: away: %s, %d", qPrintable(newMood), statusType);
		return oldMood;
	}

	return newMood;
}

void Ya::initializeDefaultGeometry(QWidget* w, const QString& geometryOptionPath, const QRect& _defaultGeometry, bool centerOnScreen)
{
	QRect savedGeometry = PsiOptions::instance()->getOption(geometryOptionPath).toRect();
	QByteArray byteArray = PsiOptions::instance()->getOption(geometryOptionPath).toByteArray();
	if (byteArray.isEmpty() && !savedGeometry.width() && !savedGeometry.height()) {
		QRect defaultGeometry = _defaultGeometry;
		if (centerOnScreen) {
			QRect r = QApplication::desktop()->availableGeometry(w);
			defaultGeometry.moveTo((r.width() - defaultGeometry.width()) / 2,
			                       (r.height() - defaultGeometry.height()) / 2);
		}

		PsiOptions::instance()->setOption(geometryOptionPath, defaultGeometry);
	}
}

Ya::DateFormatter* Ya::DateFormatter::instance_ = 0;

Ya::DateFormatter* Ya::DateFormatter::instance()
{
	if (!instance_) {
		instance_ = new DateFormatter();
	}
	return instance_;
}

Ya::DateFormatter::DateFormatter()
	: QObject(QCoreApplication::instance())
{
	months
		<< tr("January")
		<< tr("February")
		<< tr("March")
		<< tr("April")
		<< tr("May")
		<< tr("June")
		<< tr("July")
		<< tr("August")
		<< tr("September")
		<< tr("October")
		<< tr("November")
		<< tr("December");

	daysOfWeek[Qt::Monday]    = tr("Monday");
	daysOfWeek[Qt::Tuesday]   = tr("Tuesday");
	daysOfWeek[Qt::Wednesday] = tr("Wednesday");
	daysOfWeek[Qt::Thursday]  = tr("Thursday");
	daysOfWeek[Qt::Friday]    = tr("Friday");
	daysOfWeek[Qt::Saturday]  = tr("Saturday");
	daysOfWeek[Qt::Sunday]    = tr("Sunday");
}

QString Ya::DateFormatter::daySuffix(int day)
{
	static QString st = tr("st", "st as in 1st");
	static QString nd = tr("nd", "nd as in 2nd");
	static QString rd = tr("rd", "rd as in 3rd");
	static QString th = tr("th", "th as in 4th");
	if (day == 11 || day == 12 || day == 13)
		return th;
	if (day % 10 == 1)
		return st;
	if (day % 10 == 2)
		return nd;
	if (day % 10 == 3)
		return rd;
	return th;
}

QString Ya::DateFormatter::dayWithSuffix(int day)
{
	return tr("%1%2", "1-day with 2-suffix").arg(day)
	       .arg(daySuffix(day));
}

QString Ya::DateFormatter::dateAndWeekday(QDate date)
{
	return tr("%1, %2 %3", "1-day-of-week, 2-month 3-day")
	       .arg(daysOfWeek[(Qt::DayOfWeek)date.dayOfWeek()])
	       .arg(months[date.month()-1])
	       .arg(date.day());
}


QString Ya::DateFormatter::longDateTime(QDateTime dateTime)
{
	return tr("%1, %2 %3 %4, %5", "1-day-of-week, 2-month 3-day 4-year, 5-time")
	       .arg(daysOfWeek[(Qt::DayOfWeek)dateTime.date().dayOfWeek()])
	       .arg(months[dateTime.date().month()-1])
	       .arg(dateTime.date().day())
	       .arg(dateTime.date().year())
	       .arg(dateTime.toString(tr("hh:mm:ss")));
}

QString Ya::AgeFormatter::ageSuffix(int age)
{
	static QString god_singular = tr("year", "1 year / 1 год");
	static QString god_plural   = tr("years", "21 years / 21 год");
	static QString goda         = tr("years", "2 years / 2 года");
	static QString let          = tr("years", "10 years / 10 лет");
	if (age == 1)
		return god_singular;
	if (age % 10 == 0 || (age >= 11 && age <= 19))
		return let;
	if (age % 10 == 1)
		return god_plural;
	if (age % 10 == 2 || age % 10 == 3 || age % 10 == 4)
		return goda;
	return let;
}

QString Ya::AgeFormatter::ageInYears(int age)
{
	return tr("%1 %2", "1-age-number, 2-age-suffix")
	       .arg(age)
	       .arg(ageSuffix(age));
}
