#include "mrimhelper.h"

#include <QApplication>
#include <QTimer>
#include <QPointer>

#include <QWidget>
#include <QMessageBox>

#include "ui_mrimhelper.h"

#include <qca.h>
#include "xmpp.h"
#include "im.h"
#include "xmpp_tasks.h"
#include "xmpp_xmlcommon.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

static QString MRIM_TRANSPORT = "mrim.jabber.ru";
static XMPP::Jid MRIM_JID = XMPP::Jid(MRIM_TRANSPORT);
static QString APP_NAME = "Test";

using namespace XMPP;

static QString plain2rich(const QString &plain)
{
	QString rich;
	int col = 0;

	for(int i = 0; i < (int)plain.length(); ++i) {
		if(plain[i] == '\n') {
			rich += "<br>";
			col = 0;
		}
		else if(plain[i] == '\t') {
			rich += QChar::nbsp;
			while(col % 4) {
				rich += QChar::nbsp;
				++col;
			}
		}
		else if(plain[i].isSpace()) {
			if(i > 0 && plain[i-1] == ' ')
				rich += QChar::nbsp;
			else
				rich += ' ';
		}
		else if(plain[i] == '<')
			rich += "&lt;";
		else if(plain[i] == '>')
			rich += "&gt;";
		else if(plain[i] == '\"')
			rich += "&quot;";
		else if(plain[i] == '\'')
			rich += "&apos;";
		else if(plain[i] == '&')
			rich += "&amp;";
		else
			rich += plain[i];
		++col;
	}

	return rich;
}

static QString certResultToString(int result)
{
	QString s;
	switch(result) {
		case QCA::TLS::NoCertificate:
			s = QObject::tr("No certificate presented.");
			break;
		case QCA::TLS::Valid:
			break;
		case QCA::TLS::HostMismatch:
			s = QObject::tr("Hostname mismatch.");
			break;
		case QCA::TLS::InvalidCertificate:
			s = QObject::tr("Invalid Certificate.");
			break;
			// TODO: Inspect why
//			case QCA::TLS::Untrusted:
//				s = QObject::tr("Not trusted for the specified purpose.");
//				break;
//			case QCA::TLS::SignatureFailed:
//				s = QObject::tr("Invalid signature.");
//				break;
//			case QCA::TLS::InvalidCA:
//				s = QObject::tr("Invalid CA certificate.");
//				break;
//			case QCA::TLS::InvalidPurpose:
//				s = QObject::tr("Invalid certificate purpose.");
//				break;
//			case QCA::TLS::SelfSigned:
//				s = QObject::tr("Certificate is self-signed.");
//				break;
//			case QCA::TLS::Revoked:
//				s = QObject::tr("Certificate has been revoked.");
//				break;
//			case QCA::TLS::PathLengthExceeded:
//				s = QObject::tr("Maximum cert chain length exceeded.");
//				break;
//			case QCA::TLS::Expired:
//				s = QObject::tr("Certificate has expired.");
//				break;
//			case QCA::TLS::Unknown:
		default:
			s = QObject::tr("General validation error.");
			break;
	}
	return s;
}

//----------------------------------------------------------------------------
// MrimHelper
//----------------------------------------------------------------------------
class MrimHelper : public QWidget
{
	Q_OBJECT
public:
	enum State {
		State_None = 0,
		State_Connecting,
		State_Connected,
		State_Authenticated,
		State_RosterReceived,
		State_Registered,
		State_Authorized,

		State_Last = State_Authorized
	};

	MrimHelper()
		: QWidget()
	{
		ui_.setupUi(this);
		connect(ui_.pushButton, SIGNAL(clicked()), SLOT(buttonClicked()));
		connect(ui_.yaLogin, SIGNAL(textChanged(const QString&)), SLOT(lineEditChanged()));
		connect(ui_.yaPass, SIGNAL(textChanged(const QString&)), SLOT(lineEditChanged()));
		connect(ui_.mrimLogin, SIGNAL(textChanged(const QString&)), SLOT(lineEditChanged()));
		connect(ui_.mrimPass, SIGNAL(textChanged(const QString&)), SLOT(lineEditChanged()));
		lineEditChanged();

		conn = new XMPP::AdvancedConnector;
		connect(conn, SIGNAL(srvLookup(const QString &)), SLOT(conn_srvLookup(const QString &)));
		connect(conn, SIGNAL(srvResult(bool)), SLOT(conn_srvResult(bool)));
		connect(conn, SIGNAL(httpSyncStarted()), SLOT(conn_httpSyncStarted()));
		connect(conn, SIGNAL(httpSyncFinished()), SLOT(conn_httpSyncFinished()));

		Q_ASSERT(QCA::isSupported("tls"));
		if(QCA::isSupported("tls")) {
			tls = new QCA::TLS;
			tlsHandler = new XMPP::QCATLSHandler(tls);
			tlsHandler->setXMPPCertCheck(true);
			connect(tlsHandler, SIGNAL(tlsHandshaken()), SLOT(tls_handshaken()));
		}
		else {
			tls = 0;
			tlsHandler = 0;
		}

		client = new XMPP::Client();
		connect(client, SIGNAL(rosterRequestFinished(bool, int, const QString &)), SLOT(client_rosterRequestFinished(bool, int, const QString &)));
		connect(client, SIGNAL(xmlIncoming(const QString &)), SLOT(client_xmlIncoming(const QString &)));
		connect(client, SIGNAL(xmlOutgoing(const QString &)), SLOT(client_xmlOutgoing(const QString &)));
		connect(client, SIGNAL(debugText(const QString &)), SLOT(client_debugText(const QString &)));
		connect(client, SIGNAL(subscription(const Jid &, const QString &, const QString&)), SLOT(client_subscription(const Jid &, const QString &, const QString&)));

		client->setOSName(APP_NAME);
		// client->setTimeZone(SystemInfo::instance()->timezoneString(), SystemInfo::instance()->timezoneOffset());
		client->setClientName(APP_NAME);
		client->setClientVersion("1.0");
		client->setCapsNode(APP_NAME);
		client->setCapsVersion("1.0");

		stream = new XMPP::ClientStream(conn, tlsHandler);
		//stream->setOldOnly(true);
		connect(stream, SIGNAL(connected()), SLOT(cs_connected()));
		connect(stream, SIGNAL(securityLayerActivated(int)), SLOT(cs_securityLayerActivated(int)));
		connect(stream, SIGNAL(needAuthParams(bool, bool, bool)), SLOT(cs_needAuthParams(bool, bool, bool)));
		connect(stream, SIGNAL(authenticated()), SLOT(cs_authenticated()));
		connect(stream, SIGNAL(connectionClosed()), SLOT(cs_connectionClosed()));
		connect(stream, SIGNAL(delayedCloseFinished()), SLOT(cs_delayedCloseFinished()));
		// connect(stream, SIGNAL(readyRead()), SLOT(cs_readyRead()));
		// connect(stream, SIGNAL(stanzaWritten()), SLOT(cs_stanzaWritten()));
		connect(stream, SIGNAL(warning(int)), SLOT(cs_warning(int)));
		connect(stream, SIGNAL(error(int)), SLOT(cs_error(int)));

		active = false;
		connected = false;

		QTimer::singleShot(0, this, SLOT(adjustLayout()));
	}

	~MrimHelper()
	{}

	void appendLibMsg(const QString &s)
	{
		appendSysMsg(s, Qt::magenta);
	}

	void appendErrMsg(const QString &s)
	{
		appendSysMsg(s, Qt::red);
	}

	void appendXmlOut(const QString &s)
	{
		QStringList lines = QStringList::split('\n', s, true);
		QString str;
		bool first = true;
		for(QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it) {
			if(!first)
				str += "<br>";
			str += QString("<font color=\"%1\">%2</font>").arg(QColor(Qt::darkGreen).name()).arg(plain2rich(*it));
			first = false;
		}
		ui_.te_log->append(str);
	}

	void appendXmlIn(const QString &s)
	{
		QStringList lines = QStringList::split('\n', s, true);
		QString str;
		bool first = true;
		for(QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it) {
			if(!first)
				str += "<br>";
			str += QString("<font color=\"%1\">%2</font>").arg(QColor(Qt::darkBlue).name()).arg(plain2rich(*it));
			first = false;
		}
		ui_.te_log->append(str);
	}

	void appendSysMsg(const QString &s, const QColor &_c = QColor())
	{
		QString str;
		QColor c;
		if(_c.isValid())
			c = _c;
		else
			c = Qt::blue;

		if(c.isValid())
			str += QString("<font color=\"%1\">").arg(c.name());
		str += QString("*** %1").arg(s);
		if(c.isValid())
			str += QString("</font>");
		ui_.te_log->append(str);
	}

private slots:
	void lineEditChanged()
	{
		ui_.pushButton->setEnabled(
			!ui_.yaLogin->text().isEmpty() &&
			!ui_.yaPass->text().isEmpty() &&
			!ui_.mrimLogin->text().isEmpty() &&
			!ui_.mrimPass->text().isEmpty()
		);
	}

	void adjustLayout()
	{
		setWorking(false);
		ui_.yaLogin->setFocus();

		// FIXME
		// setFixedWidth(minimumSizeHint().width());
		// resize(minimumSizeHint());
		// show();

		showMaximized();
	}

	void start()
	{
		if(active)
			return;

		// FIXME
		// jid = XMPP::Jid(ui_.yaLogin->text().trimmed() + "@ya.ru/" + APP_NAME);
		jid = XMPP::Jid(ui_.yaLogin->text() + "/" + APP_NAME);
		if(jid.domain().isEmpty() || jid.node().isEmpty() || jid.resource().isEmpty()) {
			QMessageBox::information(this, tr("Error"), tr("Please enter the Full JID to connect with."));
			return;
		}

		XMPP::AdvancedConnector::Proxy proxy;
		conn->setProxy(proxy);
		// conn->setOptHostPort("xmpp.yandex.ru", 5222); // FIXME
		conn->setOptProbe(true);
		conn->setOptSSL(false);

		if(tls) {
			tls->setTrustedCertificates(QCA::systemStore());
		}

		stream->setNoopTime(55000); // every 55 seconds
		stream->setAllowPlain(XMPP::ClientStream::AllowPlain);
		stream->setRequireMutualAuth(false);
		stream->setSSFRange(QCA::SL_None, 256);
		//stream->setOldOnly(true);
		stream->setCompress(false);

		setWorking(true);
		active = true;

		appendSysMsg("Connecting...");
		// stream->connectToServer(jid);
		client->connectToServer(stream, jid);
	}

	void setWorking(bool working)
	{
		ui_.gbYa->setEnabled(!working);
		ui_.gbMail->setEnabled(!working);
		ui_.pushButton->setText(!working ? trUtf8("Сделать зашибись!") : trUtf8("Отмена"));

		if (!working) {
			setState(State_None);
		}
		else {
			setState(State_Connecting);
		}
	}

	void stop()
	{
		if(!active)
			return;

		if(connected) {
			// pb_go->setEnabled(false);
			appendSysMsg("Disconnecting...");
			stream->close();
		}
		else {
			stream->close();
			appendSysMsg("Disconnected");
			cleanup();
		}
	}

	void buttonClicked()
	{
		if(active)
			stop();
		else
			start();
	}

	void cleanup()
	{
		setWorking(false);
		active = false;
		connected = false;
	}

	void conn_srvLookup(const QString &server)
	{
		appendLibMsg(QString("SRV lookup on [%1]").arg(server));
	}

	void conn_srvResult(bool b)
	{
		if(b)
			appendLibMsg("SRV lookup success!");
		else
			appendLibMsg("SRV lookup failed");
	}

	void conn_httpSyncStarted()
	{
		appendLibMsg("HttpPoll: syncing");
	}

	void conn_httpSyncFinished()
	{
		appendLibMsg("HttpPoll: done");
	}

	void tls_handshaken()
	{
		int vr = tls->peerIdentityResult();
		if (vr == QCA::TLS::Valid && !tlsHandler->certMatchesHostname()) vr = QCA::TLS::HostMismatch;

		appendSysMsg("Successful TLS handshake.");
		if(vr == QCA::TLS::Valid)
			appendSysMsg("Valid certificate.");
		else {
			appendSysMsg(QString("Invalid certificate: %1").arg(certResultToString(vr)), Qt::red);
			appendSysMsg("Continuing anyway");
		}

		tlsHandler->continueAfterHandshake();
	}

	void cs_connected()
	{
		QString s = "Connected";
		if(conn->havePeerAddress())
			s += QString(" (%1:%2)").arg(conn->peerAddress().toString()).arg(conn->peerPort());
		if(conn->useSSL())
			s += " [ssl]";
		appendSysMsg(s);
		setState(State_Connected);
	}

	void cs_securityLayerActivated(int type)
	{
		appendSysMsg(QString("Security layer activated (%1)").arg((type == XMPP::ClientStream::LayerTLS) ? "TLS": "SASL"));
	}

	void cs_needAuthParams(bool user, bool pass, bool realm)
	{
		QString s = "Need auth parameters -";
		if(user)
			s += " (Username)";
		if(pass)
			s += " (Password)";
		if(realm)
			s += " (Realm)";
		appendSysMsg(s);

		if(user) {
			stream->setUsername(jid.node());
		}
		if(pass) {
			stream->setPassword(ui_.yaPass->text());
		}
		if(realm) {
			stream->setRealm(jid.domain());
		}

		stream->continueAfterParams();
	}

	void cs_authenticated()
	{
		connected = true;
		conn->changePollInterval(10); // slow down after login
		appendSysMsg("Authenticated");
		setState(State_Authenticated);

		client->start(jid.host(), jid.user(), ui_.yaPass->text(), jid.resource());
		if (!stream->old()) {
			XMPP::JT_Session *j = new XMPP::JT_Session(client->rootTask());
			connect(j, SIGNAL(finished()), SLOT(sessionStart_finished()));
			j->go(true);
		}
		else {
			sessionStarted();
		}

	}

	void sessionStart_finished()
	{
		XMPP::JT_Session *j = (XMPP::JT_Session*)sender();
		if ( j->success() ) {
			sessionStarted();
		}
		else {
			cs_error(-1);
		}
	}

	void sessionStarted()
	{
		client->rosterRequest();
	}

	void client_rosterRequestFinished(bool success, int, const QString &)
	{
		if (!success) {
			appendSysMsg("Roster Request Failed");
			stop();
			return;
		}

		appendSysMsg("Roster Request Finished");
		setState(State_RosterReceived);

		XMPP::JT_Register* unreg = new XMPP::JT_Register(client->rootTask());
		connect(unreg, SIGNAL(finished()), SLOT(unregFinished()));
		unreg->unreg(MRIM_JID);
		unreg->go();

		XMPP::LiveRoster::ConstIterator it = client->roster().find(MRIM_JID, false);
		if (it != client->roster().end()) {
			appendSysMsg("MRIM Already Registered");

			XMPP::JT_Roster *r = new XMPP::JT_Roster(client->rootTask());
			r->remove(MRIM_JID);
			r->go(true);
			connect(r, SIGNAL(finished()), SLOT(startRegistration()));
			return;
		}

		startRegistration();
	}

	void unregFinished()
	{
		appendSysMsg("unregFinished");
	}

	void startRegistration()
	{
		appendSysMsg("startRegistration");
		client->setPresence(XMPP::Status(XMPP::Status::Online));

		XMPP::JT_Register* reg = new XMPP::JT_Register(client->rootTask());
		connect(reg, SIGNAL(finished()), SLOT(getRegistrationFormFinished()));
		reg->getForm(MRIM_JID);
		reg->go();
	}

	void getRegistrationFormFinished()
	{
		XMPP::JT_Register* formRequestTask = (XMPP::JT_Register*)(sender());
		if (!formRequestTask->success()) {
			appendSysMsg("getRegistrationFormFinished: Failed");
			stop();
			return;
		}

		XMPP::Form form(MRIM_JID);
		form << XMPP::FormField("email", ui_.mrimLogin->text());
		form << XMPP::FormField("password", ui_.mrimPass->text());
		XMPP::JT_Register* reg = new XMPP::JT_Register(client->rootTask());
		connect(reg, SIGNAL(finished()), SLOT(setRegistrationFormFinished()));
		reg->setForm(form);
		reg->go();
	}

	void setRegistrationFormFinished()
	{
		XMPP::JT_Register* setFormTask = (XMPP::JT_Register*)(sender());
		if (!setFormTask->success()) {
			appendSysMsg("setRegistrationFormFinished: Failed");
			QMessageBox::warning(this, APP_NAME, trUtf8("Указаны неправильные Логин / Пароль для Mail.ru."));

			stop();
			return;
		}

		setState(State_Registered);
	}

	void client_subscription(const Jid &j, const QString &str, const QString& nick)
	{
		if (str == "subscribe") {
			appendSysMsg(QString("Subscription Request: %1").arg(j.full()));
			if (j.compare(MRIM_JID, false)) {
				client->sendSubscription(j, "subscribed");
				client->sendSubscription(j, "subscribe");

				setState(State_Authorized);
				QMessageBox::information(this, APP_NAME, trUtf8("Mail.ru-транспорт успешно добавлен."));
				stop();
			}
		}
	}

	void client_xmlIncoming(const QString &s)
	{
		appendXmlIn(s);
	}

	void client_xmlOutgoing(const QString &s)
	{
		appendXmlOut(s);
	}

	void client_debugText(const QString &s)
	{
		Q_UNUSED(s);
		// appendSysMsg(plain2rich(s), Qt::gray);
	}

	void cs_connectionClosed()
	{
		appendSysMsg("Disconnected by peer");
		cleanup();
	}

	void cs_delayedCloseFinished()
	{
		appendSysMsg("Disconnected");
		cleanup();
	}

	void cs_readyRead()
	{
		while(stream->stanzaAvailable()) {
			XMPP::Stanza s = stream->read();
			appendXmlIn(XMPP::Stream::xmlToString(s.element(), true));
		}
	}

	void cs_stanzaWritten()
	{
		appendSysMsg("Stanza sent");
	}

	void cs_warning(int warn)
	{
		if(warn == XMPP::ClientStream::WarnOldVersion) {
			appendSysMsg("Warning: pre-1.0 protocol server", Qt::red);
		}
		else if(warn == XMPP::ClientStream::WarnNoTLS) {
			appendSysMsg("Warning: TLS not available!", Qt::red);
		}
		stream->continueAfterWarning();
	}

	void cs_error(int err)
	{
		if(err == XMPP::ClientStream::ErrParse) {
			appendErrMsg("XML parsing error");
		}
		else if(err == XMPP::ClientStream::ErrProtocol) {
			appendErrMsg("XMPP protocol error");
		}
		else if(err == XMPP::ClientStream::ErrStream) {
			int x = stream->errorCondition();
			QString s;
			if(x == XMPP::Stream::GenericStreamError)
				s = "generic stream error";
			else if(x == XMPP::ClientStream::Conflict)
				s = "conflict (remote login replacing this one)";
			else if(x == XMPP::ClientStream::ConnectionTimeout)
				s = "timed out from inactivity";
			else if(x == XMPP::ClientStream::InternalServerError)
				s = "internal server error";
			else if(x == XMPP::ClientStream::InvalidFrom)
				s = "invalid from address";
			else if(x == XMPP::ClientStream::InvalidXml)
				s = "invalid XML";
			else if(x == XMPP::ClientStream::PolicyViolation)
				s = "policy violation.  go to jail!";
			else if(x == XMPP::ClientStream::ResourceConstraint)
				s = "server out of resources";
			else if(x == XMPP::ClientStream::SystemShutdown)
				s = "system is shutting down NOW";
			appendErrMsg(QString("XMPP stream error: %1").arg(s));
		}
		else if(err == XMPP::ClientStream::ErrConnection) {
			int x = conn->errorCode();
			QString s;
			if(x == XMPP::AdvancedConnector::ErrConnectionRefused)
				s = "unable to connect to server";
			else if(x == XMPP::AdvancedConnector::ErrHostNotFound)
				s = "host not found";
			else if(x == XMPP::AdvancedConnector::ErrProxyConnect)
				s = "proxy connect";
			else if(x == XMPP::AdvancedConnector::ErrProxyNeg)
				s = "proxy negotiating";
			else if(x == XMPP::AdvancedConnector::ErrProxyAuth)
				s = "proxy authorization";
			else if(x == XMPP::AdvancedConnector::ErrStream)
				s = "stream error";
			appendErrMsg(QString("Connection error: %1").arg(s));
		}
		else if(err == XMPP::ClientStream::ErrNeg) {
			int x = stream->errorCondition();
			QString s;
			if(x == XMPP::ClientStream::HostGone)
				s = "host no longer hosted";
			else if(x == XMPP::ClientStream::HostUnknown)
				s = "host unknown";
			else if(x == XMPP::ClientStream::RemoteConnectionFailed)
				s = "a required remote connection failed";
			else if(x == XMPP::ClientStream::SeeOtherHost)
				s = QString("see other host: [%1]").arg(stream->errorText());
			else if(x == XMPP::ClientStream::UnsupportedVersion)
				s = "server does not support proper xmpp version";
			appendErrMsg(QString("Stream negotiation error: %1").arg(s));
		}
		else if(err == XMPP::ClientStream::ErrTLS) {
			int x = stream->errorCondition();
			QString s;
			if(x == XMPP::ClientStream::TLSStart)
				s = "server rejected STARTTLS";
			else if(x == XMPP::ClientStream::TLSFail) {
				int t = tlsHandler->tlsError();
				if(t == QCA::TLS::ErrorHandshake)
					s = "TLS handshake error";
				else
					s = "broken security layer (TLS)";
			}
			appendErrMsg(s);
		}
		else if(err == XMPP::ClientStream::ErrAuth) {
			int x = stream->errorCondition();
			QString s;
			if(x == XMPP::ClientStream::GenericAuthError)
				s = "unable to login";
			else if(x == XMPP::ClientStream::NoMech)
				s = "no appropriate auth mechanism available for given security settings";
			else if(x == XMPP::ClientStream::BadProto)
				s = "bad server response";
			else if(x == XMPP::ClientStream::BadServ)
				s = "server failed mutual authentication";
			else if(x == XMPP::ClientStream::EncryptionRequired)
				s = "encryption required for chosen SASL mechanism";
			else if(x == XMPP::ClientStream::InvalidAuthzid)
				s = "invalid authzid";
			else if(x == XMPP::ClientStream::InvalidMech)
				s = "invalid SASL mechanism";
			else if(x == XMPP::ClientStream::InvalidRealm)
				s = "invalid realm";
			else if(x == XMPP::ClientStream::MechTooWeak)
				s = "SASL mechanism too weak for authzid";
			else if(x == XMPP::ClientStream::NotAuthorized)
				s = "not authorized";
			else if(x == XMPP::ClientStream::TemporaryAuthFailure)
				s = "temporary auth failure";
			appendErrMsg(QString("Auth error: %1").arg(s));
		}
		else if(err == XMPP::ClientStream::ErrSecurityLayer)
			appendErrMsg("Broken security layer (SASL)");
		cleanup();

		QMessageBox::warning(this, APP_NAME, trUtf8("Указаны неправильные Логин / Пароль для Яндекса."));
	}

private:
	Ui::MrimHelper ui_;

	void setState(State state)
	{
		state_ = state;
		float value = (float(state) / float(State_Last)) * 100;
		ui_.progressBar->setValue(int(value));
	}

	bool active, connected;
	XMPP::AdvancedConnector *conn;
	QCA::TLS *tls;
	XMPP::QCATLSHandler *tlsHandler;
	XMPP::ClientStream *stream;
	XMPP::Client *client;
	XMPP::Jid jid;
	State state_;
};

//----------------------------------------------------------------------------
// TestDebug
//----------------------------------------------------------------------------
class TestDebug : public XMPP::Debug
{
public:
	TestDebug(MrimHelper* helper)
		: helper_(helper)
	{
	}

	void msg(const QString &s)
	{
		if (helper_)
			helper_->appendLibMsg(s);
	}

	void outgoingTag(const QString &s)
	{
		if (helper_)
			helper_->appendXmlOut(s);
	}

	void incomingTag(const QString &s)
	{
		if (helper_)
			helper_->appendXmlIn(s);
	}

	void outgoingXml(const QDomElement &e)
	{
		QString out = XMPP::Stream::xmlToString(e, true);
		if (helper_)
			helper_->appendXmlOut(out);
	}

	void incomingXml(const QDomElement &e)
	{
		QString out = XMPP::Stream::xmlToString(e, true);
		if (helper_)
			helper_->appendXmlIn(out);
	}

private:
	QPointer<MrimHelper> helper_;
};

//----------------------------------------------------------------------------
// main
//----------------------------------------------------------------------------
int main (int argc, char *argv[])
{
	QCA::Initializer init;

	srand(time(NULL));

	QApplication app(argc, argv);
	MrimHelper* helper = new MrimHelper();

	Q_UNUSED(helper);
	// TestDebug *td = new TestDebug(helper);
	// XMPP::setDebug(td);
	QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
	int ret = app.exec();
	// XMPP::setDebug(0);

	return ret;
}

#ifdef QCA_STATIC
#include <QtPlugin>
#ifdef HAVE_OPENSSL
Q_IMPORT_PLUGIN(qca_ossl)
#endif
#endif

#include "mrimhelper.moc"
