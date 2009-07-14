#ifndef PSITIPLABEL_H
#define PSITIPLABEL_H

#include <QFrame>
#include <QBasicTimer>

class QTextDocument;

class PsiTipLabel : public QFrame
{
	Q_OBJECT
public:
	PsiTipLabel(QWidget* parent, bool isHelperTipLabel = false);
	~PsiTipLabel();

	virtual void init(const QString& text);
	static PsiTipLabel* instance();

	virtual void setText(const QString& text);

	// int heightForWidth(int w) const;
	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	bool eventFilter(QObject *, QEvent *);

	QString theText() const;

	QBasicTimer hideTimer, deleteTimer;

public slots:
	virtual void hideTip();

protected:
	// reimplemented
	void enterEvent(QEvent*);
	void timerEvent(QTimerEvent* e);
	void paintEvent(QPaintEvent* e);
	// QSize sizeForWidth(int w) const;

	bool isHelperTipLabel() const;

private:
	QTextDocument* doc;
	QString theText_;
	bool isRichText;
	int margin;
	// int indent;
	bool isHelperTipLabel_;

	virtual void initUi();
	virtual void startHideTimer();

	static PsiTipLabel* instance_;
};

#endif
