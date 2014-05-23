#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "palette.h"

#include <QWidget>
#include <QFont>
#include <QResizeEvent>
#include <QElapsedTimer>

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

	void pushKeyTab() {sendKey("\t", 1); resetHistoryLinesOffset();}
	void pushKeyUp() {sendKey("\x1bOA", 3); resetHistoryLinesOffset();}
	void pushKeyDown() {sendKey("\x1bOB", 3); resetHistoryLinesOffset();}
	void pushKeyRight() {sendKey("\x1bOC", 3); resetHistoryLinesOffset();}
	void pushKeyLeft() {sendKey("\x1bOD", 3); resetHistoryLinesOffset();}
	void pushKeyBackspace() {sendKey("\b", 1); resetHistoryLinesOffset();}
protected:
	void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
	void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
	void wheelEvent(QWheelEvent *ev) Q_DECL_OVERRIDE;
	//void mouseMoveEvent ( QMouseEvent * event ) Q_DECL_OVERRIDE;
	//void mousePressEvent ( QMouseEvent * event ) Q_DECL_OVERRIDE;
	//void mouseReleaseEvent ( QMouseEvent * event ) Q_DECL_OVERRIDE;
	bool event(QEvent *ev) Q_DECL_OVERRIDE;
private:
	bool gestureEvent(QGestureEvent *ev);
private:
	void setupGeometry();
	void setupFont(int point_size);
	QPen penForCell(const core::term::ScreenCell &cell);
	QBrush brushForCell(const core::term::ScreenCell &cell);
	void paintText(QPainter *painter, const QPoint &term_pos, const QString &text, const core::term::ScreenCell &text_attrs);

	void scrollBy(int x_pixels, int y_lines);
	void addHistoryLinesOffset(int offset);
	void resetHistoryLinesOffset();

	Q_SLOT void invalidateRegion(const QRect &dirty_rect);
	void invalidateAll();

	Q_SLOT void updateFocus(bool activate);

	void sendKeyTab() {sendKey("\t", 1);}
	void sendKeyUp() {sendKey("\x1bOA", 3);}
	void sendKeyDown() {sendKey("\x1bOB", 3);}
	void sendKeyRight() {sendKey("\x1bOC", 3);}
	void sendKeyLeft() {sendKey("\x1bOD", 3);}
	void sendKeyBackspace() {sendKey("\b", 1);}
	qint64 sendKey(const char *sequence, int length);
private:
	core::term::Terminal *m_terminal;
	QFont m_font;
	int m_charWidthPx;
	int m_charHeightPx;
	int m_charShiftPx;
	Palette m_palete;
	int m_historyLinesOffset;
	QPoint m_swipeStartPosition;
	//QElapsedTimer m_swipeSpeedTimer;
	int m_horizontalScrollPx;
};

}
}

#endif // TERMINALWIDGET_H
