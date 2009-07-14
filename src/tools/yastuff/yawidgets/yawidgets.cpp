/*
 * yawidgets.cpp
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

#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <QtPlugin>
#include <QtGui/QIcon>

//----------------------------------------------------------------------------
// YaWidgetPlugin
//----------------------------------------------------------------------------

class YaWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
	YaWidgetPlugin(QObject *parent = 0);

	virtual QWidget *createWidget(QWidget *parent);
	virtual QString name() const;
	virtual QString group() const;
	virtual QString toolTip() const;
	virtual QString whatsThis() const;
	virtual QString includeFile() const;

	virtual QString codeTemplate() const;
	virtual QString domXml() const;
	virtual QIcon icon() const;
	virtual bool isContainer() const;

	void initialize(QDesignerFormEditorInterface *);
	bool isInitialized() const;

private:
	bool initialized;
};

YaWidgetPlugin::YaWidgetPlugin(QObject *parent)
		: QObject(parent)
{
	initialized = false;
}

QWidget *YaWidgetPlugin::createWidget(QWidget */* parent */)
{
	return 0;
}

QString YaWidgetPlugin::name() const
{
	return "Ya Plugin";
}

QString YaWidgetPlugin::group() const
{
	return "YaPsi";
}

QString YaWidgetPlugin::toolTip() const
{
	return name();
}

QString YaWidgetPlugin::whatsThis() const
{
	return "Ya Widget";
}

QString YaWidgetPlugin::includeFile() const
{
	return "yawidget.h";
}

QString YaWidgetPlugin::codeTemplate() const
{
	return QString();
}

QString YaWidgetPlugin::domXml() const
{
	return QString();
}

QIcon YaWidgetPlugin::icon() const
{
	return QIcon();
}

bool YaWidgetPlugin::isContainer() const
{
	return false;
}

void YaWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
	if (initialized)
		return;

	initialized = true;
}

bool YaWidgetPlugin::isInitialized() const
{
	return initialized;
}

//----------------------------------------------------------------------------
// ChatView
//----------------------------------------------------------------------------

#include <QTextEdit>

class ChatViewPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	ChatViewPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new QTextEdit(parent);
	}

	QString name() const
	{
		return "ChatView";
	}

	QString domXml() const
	{
		return "<widget class=\"ChatView\" name=\"chatView\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "ChatView.";
	}

	QString includeFile() const
	{
		return "msgmle.h";
	}
};

//----------------------------------------------------------------------------
// YaLabel
//----------------------------------------------------------------------------

#include "yalabel.h"
#include "yaprofile.h"

class YaLabelPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaLabelPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		YaLabel* label = new YaLabel(parent);
		// YaProfile* profile = new YaProfile(label);
		// label->setProfile(profile);
		return label;
	}

	QString name() const
	{
		return "YaLabel";
	}

	QString domXml() const
	{
		return "<widget class=\"YaLabel\" name=\"yaLabel\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaLabel.";
	}

	QString includeFile() const
	{
		return "yalabel.h";
	}
};

//----------------------------------------------------------------------------
// YaAvatarLabel
//----------------------------------------------------------------------------

// #include "yaavatarlabel.h"
// 
// class YaAvatarLabelPlugin : public YaWidgetPlugin
// {
// 	Q_OBJECT
// public:
// 	YaAvatarLabelPlugin(QObject *parent = 0)
// 			: YaWidgetPlugin(parent)
// 	{}
// 
// 	QWidget *createWidget(QWidget *parent)
// 	{
// 		YaAvatarLabel* label = new YaAvatarLabel(parent);
// 		YaProfile* profile = new YaProfile(label);
// 		label->setProfile(profile);
// 		return label;
// 	}
// 
// 	QString name() const
// 	{
// 		return "YaAvatarLabel";
// 	}
// 
// 	QString domXml() const
// 	{
// 		return "<widget class=\"YaAvatarLabel\" name=\"yaAvatarLabel\">\n"
// 		       "</widget>\n";
// 	}
// 
// 	QString whatsThis() const
// 	{
// 		return "YaAvatarLabel.";
// 	}
// 
// 	QString includeFile() const
// 	{
// 		return "yaavatarlabel.h";
// 	}
// };

//----------------------------------------------------------------------------
// YaSelfAvatarLabel
//----------------------------------------------------------------------------

#include "yaselfavatarlabel.h"

class YaSelfAvatarLabelPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaSelfAvatarLabelPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		YaSelfAvatarLabel* label = new YaSelfAvatarLabel(parent);
		// YaProfile* profile = new YaProfile(label);
		// label->setProfile(profile);
		return label;
	}

	QString name() const
	{
		return "YaSelfAvatarLabel";
	}

	QString domXml() const
	{
		return "<widget class=\"YaSelfAvatarLabel\" name=\"yaSelfAvatarLabel\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaSelfAvatarLabel.";
	}

	QString includeFile() const
	{
		return "yaselfavatarlabel.h";
	}
};

//----------------------------------------------------------------------------
// YaSelfStatus
//----------------------------------------------------------------------------

#include "yaselfstatus.h"

class YaSelfStatusPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaSelfStatusPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new YaSelfStatus(parent);
	}

	QString name() const
	{
		return "YaSelfStatus";
	}

	QString domXml() const
	{
		return "<widget class=\"YaSelfStatus\" name=\"yaSelfStatus\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaSelfStatus.";
	}

	QString includeFile() const
	{
		return "yaselfstatus.h";
	}
};

//----------------------------------------------------------------------------
// YaSelfStatusMessage
//----------------------------------------------------------------------------

#include "yaselfstatusmessage.h"

class YaSelfStatusMessagePlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaSelfStatusMessagePlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new YaSelfStatusMessage(parent);
	}

	QString name() const
	{
		return "YaSelfStatusMessage";
	}

	QString domXml() const
	{
		return "<widget class=\"YaSelfStatusMessage\" name=\"yaSelfStatusMessage\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaSelfStatusMessage.";
	}

	QString includeFile() const
	{
		return "yaselfstatusmessage.h";
	}
};

//----------------------------------------------------------------------------
// YaContactListView
//----------------------------------------------------------------------------

#include "yacontactlistview.h"

class YaContactListViewPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaContactListViewPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new YaContactListView(parent);
	}

	QString name() const
	{
		return "YaContactListView";
	}

	QString domXml() const
	{
		return "<widget class=\"YaContactListView\" name=\"yaContactListView\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaContactListView.";
	}

	QString includeFile() const
	{
		return "yacontactlistview.h";
	}
};

//----------------------------------------------------------------------------
// YaEventNotifierFrame
//----------------------------------------------------------------------------

#include "yaeventnotifierframe.h"

class YaEventNotifierFramePlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaEventNotifierFramePlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		YaEventNotifierFrame* frame = new YaEventNotifierFrame(parent);
		// frame->setBackground(YaEventNotifierFrame::Informer);
		return frame;
	}

	QString name() const
	{
		return "YaEventNotifierFrame";
	}

	QString domXml() const
	{
		return "<widget class=\"YaEventNotifierFrame\" name=\"yaEventNotifierFrame\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaEventNotifierFrame.";
	}

	QString includeFile() const
	{
		return "yaeventnotifierframe.h";
	}
};

//----------------------------------------------------------------------------
// YaEventMessage
//----------------------------------------------------------------------------

#include "yaeventmessage.h"

class YaEventMessagePlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaEventMessagePlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		YaEventMessage* label = new YaEventMessage(parent);
		// label->setMessage(QString::fromUtf8("Зачем Ви травите?\nХотя с другой стороны, если дустом…"));
		label->setMessage(QString::fromUtf8("Hello, World!\nYou're so wonderful I'm really gonna kill myself someday…"));
		return label;
	}

	QString name() const
	{
		return "YaEventMessage";
	}

	QString domXml() const
	{
		return "<widget class=\"YaEventMessage\" name=\"yaEventMessage\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaEventMessage.";
	}

	QString includeFile() const
	{
		return "yaeventmessage.h";
	}
};

//----------------------------------------------------------------------------
// YaChatContactStatus
//----------------------------------------------------------------------------

#include "yachatcontactstatus.h"

class YaChatContactStatusPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaChatContactStatusPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		YaChatContactStatus* label = new YaChatContactStatus(parent);
		label->setStatus(XMPP::Status::DND, QString::fromUtf8("Зачем Ви травите?\nХотя с другой стороны, если дустом…"));
		return label;
	}

	QString name() const
	{
		return "YaChatContactStatus";
	}

	QString domXml() const
	{
		return "<widget class=\"YaChatContactStatus\" name=\"yaChatContactStatus\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaChatContactStatus.";
	}

	QString includeFile() const
	{
		return "yachatcontactstatus.h";
	}
};

//----------------------------------------------------------------------------
// YaChatEdit
//----------------------------------------------------------------------------

#include "yachatedit.h"

class YaChatEditPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaChatEditPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new YaChatEdit(parent);
	}

	QString name() const
	{
		return "YaChatEdit";
	}

	QString domXml() const
	{
		return "<widget class=\"YaChatEdit\" name=\"yaChatEdit\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaChatEdit.";
	}

	QString includeFile() const
	{
		return "yachatedit.h";
	}
};

//----------------------------------------------------------------------------
// YaSplitter
//----------------------------------------------------------------------------

#include "yasplitter.h"

class YaSplitterPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaSplitterPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new YaSplitter(parent);
	}

	QString name() const
	{
		return "YaSplitter";
	}

	QString domXml() const
	{
		return "<widget class=\"YaSplitter\" name=\"yaSplitter\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaSplitter.";
	}

	QString includeFile() const
	{
		return "yasplitter.h";
	}
};

//----------------------------------------------------------------------------
// YaRosterComboBox
//----------------------------------------------------------------------------

#include "yarostercombobox.h"

class YaRosterComboBoxPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaRosterComboBoxPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new YaRosterComboBox(parent);
	}

	QString name() const
	{
		return "YaRosterComboBox";
	}

	QString domXml() const
	{
		return "<widget class=\"YaRosterComboBox\" name=\"yaRosterComboBox\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaRosterComboBox.";
	}

	QString includeFile() const
	{
		return "yarostercombobox.h";
	}
};

//----------------------------------------------------------------------------
// YaPushButton
//----------------------------------------------------------------------------

#include "yapushbutton.h"

class YaPushButtonPlugin : public YaWidgetPlugin
{
	Q_OBJECT
public:
	YaPushButtonPlugin(QObject *parent = 0)
			: YaWidgetPlugin(parent)
	{}

	QWidget *createWidget(QWidget *parent)
	{
		return new YaPushButton(parent);
	}

	QString name() const
	{
		return "YaPushButton";
	}

	QString domXml() const
	{
		return "<widget class=\"YaPushButton\" name=\"yaPushButton\">\n"
		       "</widget>\n";
	}

	QString whatsThis() const
	{
		return "YaPushButton.";
	}

	QString includeFile() const
	{
		return "yapushbutton.h";
	}
};

//----------------------------------------------------------------------------
// AllYaWidgetsPlugin
//----------------------------------------------------------------------------

class AllYaWidgetsPlugin : public QObject, public QDesignerCustomWidgetCollectionInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
public:
	AllYaWidgetsPlugin(QObject *parent = 0)
			: QObject(parent)
	{
		plugins.append(new ChatViewPlugin(this));
		plugins.append(new YaLabelPlugin(this));
		// plugins.append(new YaAvatarLabelPlugin(this));
		plugins.append(new YaSelfAvatarLabelPlugin(this));
		plugins.append(new YaSelfStatusPlugin(this));
		plugins.append(new YaSelfStatusMessagePlugin(this));
		plugins.append(new YaContactListViewPlugin(this));
		plugins.append(new YaEventNotifierFramePlugin(this));
		plugins.append(new YaEventMessagePlugin(this));
		plugins.append(new YaChatContactStatusPlugin(this));
		plugins.append(new YaChatEditPlugin(this));
		plugins.append(new YaSplitterPlugin(this));
		plugins.append(new YaRosterComboBoxPlugin(this));
		plugins.append(new YaPushButtonPlugin(this));
	}

	virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const
	{
		return plugins;
	}

private:
	QList<QDesignerCustomWidgetInterface*> plugins;
};

Q_EXPORT_PLUGIN2(ya_widgets, AllYaWidgetsPlugin);

#include "yawidgets.moc"
