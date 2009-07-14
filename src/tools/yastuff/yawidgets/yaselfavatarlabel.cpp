/*
 * yaselfavatarlabel.cpp - self avatar widget
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

#include "yaselfavatarlabel.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QDir>
#include <QGridLayout>
#include <QFileDialog>
#include <QStyle>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QBuffer>

#include "applicationinfo.h"
#include "avatars.h"
#include "psiaccount.h"
#include "psicontactlist.h"
#include "vcardfactory.h"
#include "xmpp_vcard.h"
#include "yarostertooltip.h"
#include "yastyle.h"
#include "pixmaputil.h"
#include "advwidget.h"
#include "psicontact.h"

//----------------------------------------------------------------------------
// YaSelfAvatarLabel::AvatarData
//----------------------------------------------------------------------------

YaSelfAvatarLabel::AvatarData::AvatarData()
{
}

YaSelfAvatarLabel::AvatarData::AvatarData(QString fileName)
{
	Q_ASSERT(!fileName.isEmpty());
	Q_ASSERT(QFile::exists(fileName));

	QFile file(fileName);
	if (!fileName.isEmpty() && file.open(QFile::ReadOnly)) {
		initFromData(file.readAll());
	}
	else {
		initFromData(QByteArray());
	}
}

void YaSelfAvatarLabel::AvatarData::initFromData(const QByteArray& data)
{
	data_ = data;

	// Q_ASSERT(!image_.isNull());
	// QByteArray ba;
	// QBuffer buffer(&ba);
	// buffer.open(QIODevice::WriteOnly);
	// image_.save(&buffer, "PNG");
	// return ba;

	image_ = QImage::fromData(data_);
	Q_ASSERT(!image_.isNull());

	hash_ = QCA::Hash("sha1").hashToString(data_);
	Q_ASSERT(!hash_.isEmpty());
}

QByteArray YaSelfAvatarLabel::AvatarData::data() const
{
	return data_;
}

bool YaSelfAvatarLabel::AvatarData::isNull() const
{
	return image_.isNull() || hash_.isEmpty();
}

const QImage& YaSelfAvatarLabel::AvatarData::image() const
{
	return image_;
}

const QString& YaSelfAvatarLabel::AvatarData::hash() const
{
	return hash_;
}

//----------------------------------------------------------------------------
// YaSelfAvatarLabelMenuButton
//----------------------------------------------------------------------------

static const int GRID_SIZE = 6;
static const QSize MAX_AVATAR_SIZE(50, 50);

static bool compare(const QImage& i1, const QImage& i2)
{
	if (i1 == i2)
		return true;

	if (i1.isNull() || i2.isNull())
		return false;

	if (i1.height() != i2.height() || i1.width() != i2.width() || i1.format() != i2.format())
		return false;

	if (i1.colorTable() != i2.colorTable())
		return false;

	int totalDelta = 0;
	for (int y = 0; y < i1.height(); ++y) {
		for (int x = 0; x < i1.width(); ++x) {
			QRgb rgb1 = i1.pixel(x, y);
			QRgb rgb2 = i2.pixel(x, y);
			if (rgb1 != rgb2) {
				int delta = qAbs(qAlpha(rgb1) - qAlpha(rgb2)) +
				            qAbs(qBlue(rgb1) - qBlue(rgb2)) +
				            qAbs(qRed(rgb1) - qRed(rgb2)) +
				            qAbs(qGreen(rgb1) - qGreen(rgb2));
				totalDelta += delta;
				// if (delta > 0) {
				// 	qWarning("a: %d %d: %d", qAlpha(rgb1), qAlpha(rgb2), qAbs(qAlpha(rgb1) - qAlpha(rgb2)));
				// 	qWarning("b: %d %d: %d", qBlue(rgb1), qBlue(rgb2), qAbs(qBlue(rgb1) - qBlue(rgb2)));
				// 	qWarning("r: %d %d: %d", qRed(rgb1), qRed(rgb2), qAbs(qRed(rgb1) - qRed(rgb2)));
				// 	qWarning("g: %d %d: %d", qGreen(rgb1), qGreen(rgb2), qAbs(qGreen(rgb1) - qGreen(rgb2)));
				// }
				// if (delta > 10) {
				// 	qWarning("delta = %d", delta);
				// 	qWarning() << rgb1;
				// 	qWarning() << rgb2;
				// 	qWarning("pixel off on %d %d", x, y);
				// 	// return false;
				// }
			}
		}
	}

	int size = i1.width() * i1.height();
	float diff = (float(totalDelta) / float(size)) * 100.0;
	// qWarning("totalDelta = %d, size = %d; %f", totalDelta, size, diff);

	return diff < 100.0;
}

class YaSelfAvatarLabelMenuButton : public QAbstractButton
{
	Q_OBJECT

private:
	YaSelfAvatarLabel::AvatarData avatar_;
	QPixmap pixmap_;

public:
	YaSelfAvatarLabelMenuButton(QWidget *parent, YaSelfAvatarLabel::AvatarData avatarData)
		: QAbstractButton(parent)
		, avatar_(avatarData)
	{
		pixmap_ = QPixmap::fromImage(avatar_.image());
		setPalette(YaStyle::menuPalette());
		setProperty("hide-menu-popupgutter", true);
	}

	const YaSelfAvatarLabel::AvatarData& avatar() const { return avatar_; }

protected:
	// reimplemented
	void enterEvent(QEvent *) { setFocus();   update(); } // focus follows mouse mode
	void leaveEvent(QEvent *) { clearFocus(); update(); }
	QSize sizeHint() const { return /*avatar_.size()*/ MAX_AVATAR_SIZE; }

	virtual void drawForeground(QPainter* p)
	{
		int margin = 4;
		QIcon icon(pixmap_);
		icon.paint(p, rect().adjusted(margin, margin, -margin, -margin),
		           Qt::AlignCenter,
		           hasFocus() ? QIcon::Selected : QIcon::Normal);
	}

	void paintEvent(QPaintEvent *)
	{
		QPainter p(this);
		QFlags<QStyle::StateFlag> flags = QStyle::State_Active | QStyle::State_Enabled;

		if ( hasFocus() )
			flags |= QStyle::State_Selected;

		QStyleOptionMenuItem opt;
		opt.palette = palette();
		opt.state = flags;
		opt.font = font();
		opt.rect = rect();
		style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);

		drawForeground(&p);
	}
};

class YaSelfAvatarLabelChooseButton : public YaSelfAvatarLabelMenuButton
{
	Q_OBJECT

public:
	YaSelfAvatarLabelChooseButton(QWidget *parent)
		: YaSelfAvatarLabelMenuButton(parent, YaSelfAvatarLabel::AvatarData())
	{
	}

protected:
	// reimplemented
	QSize sizeHint() const
	{
		return QSize(fontMetrics().width(chooseText()), 2 * fontMetrics().height());
	}

	void drawForeground(QPainter* p)
	{
		p->drawText(rect(), Qt::AlignCenter, chooseText());
	}

	QString chooseText() const
	{
		return tr("Choose...");
	}
};

//----------------------------------------------------------------------------
// YaSelfAvatarLabelMenu
//----------------------------------------------------------------------------

class YaSelfAvatarLabelMenu : public AdvancedWidget<QMenu>
{
	Q_OBJECT

public:
	YaSelfAvatarLabelMenu(QWidget *parent = 0)
		: AdvancedWidget<QMenu>(parent)
	{
		setStyle(YaStyle::defaultStyle());
		YaStyle::makeMeNativeLooking(this);

		setProperty("hide-menu-popupgutter", true);

		QGridLayout *grid = new QGridLayout(this);
		grid->setMargin(style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this));
		grid->setSpacing(0);
	}

public:
	/** shifts avatars when the user has the new picture chosen, to store it properly */
	void buildAvatarsMenu(const YaSelfAvatarLabel::AvatarData& currentAvatar)
	{
		QList<QAbstractButton*> buttons = findChildren<QAbstractButton*>();
		qDeleteAll(buttons);
		clear();

		QList<YaSelfAvatarLabel::AvatarData> avatars;
		buildAvatars(currentAvatar, avatars);

		QGridLayout *gridLayout = static_cast<QGridLayout *>(layout());
		int col = 0, row = 0;
		foreach(YaSelfAvatarLabel::AvatarData avatar, avatars) {
			YaSelfAvatarLabelMenuButton* button = new YaSelfAvatarLabelMenuButton(this, avatar);
			connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));
			gridLayout->addWidget(button, row, col);
			row += (++col %= GRID_SIZE) ? 0 : 1;
		}

		if (col > 0) {
			row++;
			col = 0;
		}

		YaSelfAvatarLabelChooseButton* chooseButton = new YaSelfAvatarLabelChooseButton(this);
		connect(chooseButton, SIGNAL(clicked()), SLOT(choosePixmapClicked()));
		gridLayout->addWidget(chooseButton, row, col, GRID_SIZE, 0);
	}

	// reimplemented
	void postShowWidgetOffscreen()
	{
		QGridLayout *gridLayout = static_cast<QGridLayout *>(layout());
		gridLayout->invalidate();
		gridLayout->activate();

		AdvancedWidget<QMenu>::postShowWidgetOffscreen();
	}

	// reimplemented
	void setVisible(bool visible)
	{
		if (visible) {
			showWidgetOffscreen();
			return;
		}

		AdvancedWidget<QMenu>::setVisible(visible);
	}

protected:
	/** @return avatars to be used as avatar choose menu images 
	  * @todo   make all the previously chosen avatars available thru this menu
	  */
	void buildAvatars(const YaSelfAvatarLabel::AvatarData& currentAvatar, QList<YaSelfAvatarLabel::AvatarData> &avatars, int maxAvatarCount = GRID_SIZE * GRID_SIZE) const
	{
		QList<QDir> dirs;
		dirs << QDir(ApplicationInfo::yavatarsDir());
		dirs << QDir(":images/avatars");

		QList<YaSelfAvatarLabel::AvatarData> excludeList;
		excludeList << currentAvatar;

		foreach(QDir dir, dirs) {
			foreach(QString avatar, dir.entryList()) {
				if (avatars.count() >= maxAvatarCount) {
					break;
				}

				YaSelfAvatarLabel::AvatarData img;
				{
					QString fileName = dir.absoluteFilePath(avatar);
					QImage image(fileName);
					if (image.isNull())
						continue;
					img = YaSelfAvatarLabel::AvatarData(fileName);
				}
				if (img.isNull())
					continue;

				bool found = false;
				foreach(const YaSelfAvatarLabel::AvatarData& i, excludeList) {
					if (i.hash() == img.hash()) {
						found = true;
						break;
					}
				}

				if (!found) {
					avatars << img;
					excludeList << img;
				}
			}
		}

		QList<YaSelfAvatarLabel::AvatarData> avatarsToSave = avatars;
		if (!currentAvatar.isNull())
			avatarsToSave.prepend(currentAvatar);

		saveAvatars(avatarsToSave);
	}

	void saveAvatars(const QList<YaSelfAvatarLabel::AvatarData> &avatars) const
	{
		QDir yavatarsDir = ApplicationInfo::yavatarsDir();
		foreach(QString file, yavatarsDir.entryList()) {
			QFile::remove(file);
		}

		int i = 0;
		foreach(YaSelfAvatarLabel::AvatarData avatar, avatars) {
			QFile file(QString("%1/%2.png")
			           .arg(yavatarsDir.absolutePath())
			           .arg(++i, 4, 10, QChar('0')));
			if (file.open(QFile::WriteOnly)) {
				file.write(avatar.data());
			}
		}
	}

signals:
	void avatarSelected(YaSelfAvatarLabel::AvatarData);
	void choosePixmap();

private slots:
	void buttonClicked()
	{
		close();
		YaSelfAvatarLabelMenuButton* button = static_cast<YaSelfAvatarLabelMenuButton*>(sender());
		emit avatarSelected(button->avatar());
	}

	void choosePixmapClicked()
	{
		close();
		emit choosePixmap();
	}

	/**
	 * Override QMenu's nasty sizeHint() and use standard QWidget one
	 * which honors layouts and child widgets.
	 */
	QSize sizeHint() const
	{
		return QWidget::sizeHint();
	}
};

//----------------------------------------------------------------------------
// YaSelfAvatarLabel
//----------------------------------------------------------------------------

YaSelfAvatarLabel::YaSelfAvatarLabel(QWidget* parent)
	: YaAvatarLabel(parent)
	, mode_(SelectAvatar)
	, menu_(new YaSelfAvatarLabelMenu(this))
	, contactList_(0)
	, uploadingVCardsCount_(0)
	, busyWidget_(0)
	, selfContact_(0)
{
	setAttribute(Qt::WA_Hover, true);

	busyWidget_ = new BusyWidget(this);
	busyWidget_->hide();

	qRegisterMetaType<YaSelfAvatarLabel::AvatarData>("YaSelfAvatarLabel::AvatarData");

	installEventFilter(this);

	connect(menu_, SIGNAL(avatarSelected(YaSelfAvatarLabel::AvatarData)), SLOT(avatarSelected(YaSelfAvatarLabel::AvatarData)));
	connect(menu_, SIGNAL(choosePixmap()), SLOT(choosePixmap()));
	connect(VCardFactory::instance(), SIGNAL(vcardChanged(const Jid&)), SLOT(update()));
}

Q_DECLARE_METATYPE(YaSelfAvatarLabel::AvatarData);

YaSelfAvatarLabel::~YaSelfAvatarLabel()
{
}

YaSelfAvatarLabel::Mode YaSelfAvatarLabel::mode() const
{
	return mode_;
}

void YaSelfAvatarLabel::setMode(YaSelfAvatarLabel::Mode mode)
{
	mode_ = mode;
}

void YaSelfAvatarLabel::mouseReleaseEvent(QMouseEvent*)
{
#if 0 // ONLINE-1890
	// menu shouldn't overlap a label
	QPoint menuPos = mapToGlobal(rect().bottomLeft());

	switch (mode_) {
	case SelectAvatar:
		setCursor(Qt::WaitCursor);
		menu_->buildAvatarsMenu(avatarData());
		unsetCursor();
		menu_->exec(menuPos);
		update();
		break;
	case OpenProfile:
		Q_ASSERT(false);
		break;
	default:
		Q_ASSERT(false);
	}
#else
	menu_->buildAvatarsMenu(avatarData());
	choosePixmap();
#endif
}

bool YaSelfAvatarLabel::isConnected() const
{
	foreach(const PsiAccount* a, accounts()) {
		if (a->isAvailable())
			return true;
	}

	return false;
}

void YaSelfAvatarLabel::avatarSelected(YaSelfAvatarLabel::AvatarData _av)
{
	YaSelfAvatarLabel::AvatarData av = _av;
	QByteArray data = av.data();
	if (qMax(av.image().width(), av.image().height()) > AvatarFactory::maxAvatarSize()) {
		// TODO: insert a fancy dialog for downscaling purposes
		QImage img = av.image();
		img = img.scaled(QSize(AvatarFactory::maxAvatarSize(), AvatarFactory::maxAvatarSize()),
		                 Qt::KeepAspectRatio, Qt::SmoothTransformation);

		// QPixmap tmp = PixmapUtil::createTransparentPixmap(AvatarFactory::maxAvatarSize(), AvatarFactory::maxAvatarSize());
		// QPainter p(&tmp);
		// p.drawPixmap((tmp.width() - pix.width()) / 2, (tmp.height() - pix.height()) / 2, av);

		data.clear();
		QBuffer buffer(&data);
		buffer.open(QIODevice::WriteOnly);
		Q_ASSERT(buffer.isOpen());
		img.save(&buffer, "PNG");
	}

	av.initFromData(data);

	if (contactList() && !av.isNull()) {
		foreach(const PsiAccount* a, accounts()) {
			if (!a->isAvailable())
				continue;
			VCard vcard;
			const VCard* currentVCard = VCardFactory::instance()->vcard(a->jid());
			if (currentVCard)
				vcard = *currentVCard;
			vcard.setPhoto(data);
			uploadingVCardsCount_++;
			VCardFactory::instance()->setVCard(a, vcard, this, SLOT(setVCardFinished()));
			updateBusyWidget();
		}

		menu_->buildAvatarsMenu(av);
	}
}

void YaSelfAvatarLabel::setVCardFinished()
{
	uploadingVCardsCount_--;
	Q_ASSERT(uploadingVCardsCount_ >= 0);
	updateBusyWidget();
}

void YaSelfAvatarLabel::updateBusyWidget()
{
	QSize sh = busyWidget_->sizeHint();
	busyWidget_->setGeometry(0, height() - sh.height() + 1, width(), sh.height());
	bool active = uploadingVCardsCount_ > 0;
	busyWidget_->setVisible(active);
	busyWidget_->setActive(active);
}

void YaSelfAvatarLabel::choosePixmap()
{
	// TODO: move to common Psi way of getting file names
	QString result = QFileDialog::getOpenFileName(0, trUtf8("Choose an image"),
	                   // ApplicationInfo::documentsDir(),
	                   ApplicationInfo::yavatarsDir(),
	                   trUtf8("All files (*.png *.jpg *.jpeg *.gif)"));
	// dialog.setViewMode(QFileDialog::Detail);
	if (!result.isEmpty()) {
		avatarSelected(AvatarData(result));
	}
}

XMPP::VCard::Gender YaSelfAvatarLabel::gender() const
{
	if (contactList()) {
		PsiAccount* account = contactList()->defaultAccount();
		if (account) {
			return account->selfContact()->gender();
		}
	}
	return YaAvatarLabel::gender();
}

void YaSelfAvatarLabel::paintAvatar(QPainter* painter)
{
	YaAvatarLabel::paintAvatar(painter);
	if (underMouse() && isEnabled()) {
		painter->setOpacity(0.3);
		painter->fillRect(rect(), QColor(0x00,0x00,0x00));
		// TODO: draw selection arrow at the bottom?
	}
}

YaSelfAvatarLabel::AvatarData YaSelfAvatarLabel::avatarData() const
{
	AvatarData result;
	if (contactList()) {
		PsiAccount* account = contactList()->defaultAccount();
		if (account) {
			QString fileName = account->avatarFactory()->getCachedAvatarFileName(account->jid());
			if (!fileName.isEmpty()) {
				result = AvatarData(fileName);
			}
		}
	}
	return result;
}

QPixmap YaSelfAvatarLabel::avatarPixmap() const
{
	QPixmap result;
	if (contactList()) {
		PsiAccount* account = contactList()->defaultAccount();
		if (account)
			result = account->avatarFactory()->getAvatar(account->jid());
	}
	return result;
}

QIcon YaSelfAvatarLabel::avatar() const
{
	return avatarPixmap();
}

void YaSelfAvatarLabel::accountCountChanged()
{
	if (!contactList()->accountsLoaded())
		return;

	if (contactList() && contactList()->defaultAccount()) {
		foreach(PsiAccount* account, accounts())
			disconnect(account->avatarFactory(), SIGNAL(avatarChanged(const Jid&)), this, SLOT(update()));
		connect(contactList()->defaultAccount()->avatarFactory(), SIGNAL(avatarChanged(const Jid&)), SLOT(update()));
	}

	accountActivityChanged();
}

void YaSelfAvatarLabel::accountActivityChanged()
{
	setStatus(isConnected() ? XMPP::Status::Online : XMPP::Status::Offline);
	setEnabled(isConnected());

	if (isEnabled())
		setCursor(Qt::PointingHandCursor);
	else
		unsetCursor();

	update();
}

void YaSelfAvatarLabel::setContactList(const PsiContactList* contactList)
{
	if (contactList_ != contactList) {
		contactList_ = (PsiContactList*)contactList;
		if (contactList_) {
			connect(contactList_, SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
			connect(contactList_, SIGNAL(accountActivityChanged()), SLOT(accountActivityChanged()));
		}

		accountCountChanged();
	}
}

const PsiContactList* YaSelfAvatarLabel::contactList() const
{
	return contactList_;
}

QList<PsiAccount*> YaSelfAvatarLabel::accounts() const
{
	if (!contactList())
		return QList<PsiAccount*>();

	return contactList()->enabledAccounts();
}

bool YaSelfAvatarLabel::eventFilter(QObject* obj, QEvent* event)
{
#if 0
	if (event->type() == QEvent::ToolTip &&
	    (isActiveWindow() || window()->testAttribute(Qt::WA_AlwaysShowToolTips)))
	{
		if (!selfContact_)
			return true;

		QRect windowRect = QRect(window()->mapToGlobal(window()->rect().topLeft()),
		                         window()->mapToGlobal(window()->rect().bottomRight()));

		QRect globalRect = QRect(mapToGlobal(rect().topLeft()),
		                         mapToGlobal(rect().bottomRight()));
		// globalRect.setLeft(windowRect.left());
		globalRect.setRight(windowRect.right());

		YaRosterToolTip::instance()->showText(globalRect,
		                                      selfContact_,
		                                      dynamic_cast<QWidget*>(obj),
		                                      0);
		return true;
	}
#endif

	return YaAvatarLabel::eventFilter(obj, event);
}

PsiContact* YaSelfAvatarLabel::selfContact() const
{
	return selfContact_;
}

void YaSelfAvatarLabel::setSelfContact(PsiContact* contact)
{
	selfContact_ = contact;
}

void YaSelfAvatarLabel::contextMenuEvent(QContextMenuEvent* e)
{
	e->accept();
}

#include "yaselfavatarlabel.moc"
