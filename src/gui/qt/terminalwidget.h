#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "palette.h"

#include <QWidget>
#include <QFont>
#include <QResizeEvent>

namespace core {
namespace term {
class ScreenCell;
class Terminal;
}
}

namespace gui {
namespace qt {

class TerminalWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TerminalWidget(QWidget *parent = 0);
public:
	void setTerminal(core::term::Terminal *t);

	void sendKeyBackspace() {sendKey("\b", 1);}
	void sendKeyTab() {sendKey("\t", 1);}
	void sendKeyUp() {sendKey("\x1bOA", 3);}
	void sendKeyDown() {sendKey("\x1bOB", 3);}
	void sendKeyRight() {sendKey("\x1bOC", 3);}
	void sendKeyLeft() {sendKey("\x1bOD", 3);}
protected:
	void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
	void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
private:
	void setupGeometry();
	void setupFont(int point_size);
	QPen penForCell(const core::term::ScreenCell &cell);
	QBrush brushForCell(const core::term::ScreenCell &cell);
	void paintText(QPainter *painter, const QPoint &term_pos, const QString &text, const core::term::ScreenCell &text_attrs);

	Q_SLOT void invalidateRegion(const QRect &dirty_rect);

	qint64 sendKey(const char *sequence, int length);
private:
	core::term::Terminal *m_terminal;
	QFont m_font;
	int m_charWidthPx;
	int m_charHeightPx;
	int m_charShiftPx;
	Palette m_palete;
};

}
}

#endif // TERMINALWIDGET_H
