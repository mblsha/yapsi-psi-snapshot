#ifndef TEXTUTIL_H
#define TEXTUTIL_H

class QString;
#include "xmpp_message.h"

namespace TextUtil 
{
	QString escape(const QString& unescaped);
	QString unescape(const QString& escaped);

	QString quote(const QString &, int width=60, bool quoteEmpty=false);
	QString plain2richSimple(const QString &);
	QString plain2rich(const QString &);
	QString rich2plain(const QString &);
	QString resolveEntities(const QString &);
	QString linkify(const QString &);
	QString linkifyClever(const QString &);
	QString legacyFormat(const QString &);
	QString emoticonify(const QString &in);
#ifdef YAPSI
	QString emoticonifyForYaOnline(const QString &in);
	QString emoticonifyForHtml(const QString &in);
#endif
};

#endif
