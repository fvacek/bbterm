#include "terminalwidget.h"

#include <core/term/screenbuffer.h>
#include <core/term/slaveptyprocess.h>
#include <core/term/terminal.h>
#ifdef Q_OS_QNX
#include "bbvirtualkeyboardhandler.h"
#endif

#include <QPainter>
#include <QColor>
#include <QSwipeGesture>

//#define NO_BBTERM_LOG_DEBUG
#include <core/util/log.h>

using namespace gui::qt;

TerminalWidget::TerminalWidget(QWidget *parent)
: QWidget(parent), m_historyLinesOffset(0), m_horizontalScrollPx(0)
{
	setupFont(8);
#ifdef Q_OS_QNX
	// do not work, should be???
	//grabGesture(Qt::SwipeGesture);
#endif
}

void TerminalWidget::setupFont(int point_size)
{
#ifdef Q_OS_QNX
		m_font = QFont("Andale Mono");
		connect(BBVirtualKeyboardHandler::instance(), SIGNAL(keyboardVisibleChanged(bool)), this, SLOT(updateFocus(bool)));
#else
		m_font = QFont();
		m_font.setStyleHint(QFont::Monospace);
		m_font.setStyleStrategy(QFont::NoAntialias);
		m_font.setFamily("Monospace");
		m_font.setFixedPitch(true);
		m_font.setKerning(false);
#endif
		/*
		if( QApplication::desktop()->screenGeometry().width() < 1000 ) {
			font->setPointSize(6);
		} else {
			font->setPointSize(12);
		}
		*/
		m_font.setPointSize(point_size);

		QFontMetrics metrics(m_font);
		m_charWidthPx = metrics.width(QChar(' '));
		m_charHeightPx = metrics.lineSpacing();
		m_charShiftPx = m_charHeightPx / 5;
		LOGDEB() << "char width:" << m_charWidthPx << "height:" << m_charHeightPx << "leading:" << m_charShiftPx;
}

void TerminalWidget::invalidateRegion(const QRect &dirty_rect)
{
	//LOGDEB() << Q_FUNC_INFO;
	Q_UNUSED(dirty_rect);
	update();
}

void TerminalWidget::invalidateAll()
{
	core::term::ScreenBuffer *screen_buffer = m_terminal->screenBuffer();
	invalidateRegion(QRect(QPoint(0, 0), screen_buffer->terminalSize()));
}

void TerminalWidget::updateFocus(bool activate)
{
	LOGDEB() << Q_FUNC_INFO << activate;
	if(activate) {
		setFocus();
	}
}

void TerminalWidget::setTerminal(core::term::Terminal *t)
{
	m_terminal = t;
	if(m_terminal) {
		connect(m_terminal->screenBuffer(), SIGNAL(dirtyRegion(QRect)), this, SLOT(invalidateRegion(QRect)));
	}
	setupGeometry();
}

void TerminalWidget::paintEvent(QPaintEvent *ev)
{
	//LOGDEB() << Q_FUNC_INFO;
	Q_UNUSED(ev);
	QPainter painter(this);
	QColor fg_color(255,255,255);
	QColor bg_color(8,0,0);
	//painter.setBackgroundMode(Qt::TransparentMode);
	painter.setPen(QPen(fg_color));
	painter.setFont(m_font);
	QRect r(QPoint(0, 0), geometry().size());
	painter.fillRect(r, QBrush(bg_color));
	core::term::ScreenBuffer *screen_buffer = m_terminal->screenBuffer();
	int row_count = screen_buffer->rowCount();
	int start_line_ix = screen_buffer->firstVisibleLineIndex() - m_historyLinesOffset;
	if(start_line_ix < 0)  start_line_ix = 0;
	//LOGDEB() << start_ix << row_count;
	for(int i=start_line_ix; i<row_count; i++) {
		const core::term::ScreenLine screen_line = screen_buffer->lineAt(i);
		QString line_str = screen_line.toString();
		core::term::ScreenCell first_cell;
		first_cell.setAttributes(0xff);
		int term_y = i - start_line_ix;
		int chunk_pos = 0;
		int chunk_len = 0;
		while(chunk_pos + chunk_len < line_str.length()) {
			core::term::ScreenCell cell = screen_line.value(chunk_pos + chunk_len);
			//int term_x = chunk_pos + chunk_len;
			if(cell.isAllAttributesEqual(first_cell)) {
				chunk_len++;
			}
			else {
				if(chunk_len > 0) {
					QString s = line_str.mid(chunk_pos, chunk_len);
					paintText(&painter, QPoint(chunk_pos, term_y), s, first_cell);
				}
				first_cell = cell;
				chunk_pos += chunk_len;
				chunk_len = 0;
			}
		}
		if(chunk_len > 0) {
			QString s = line_str.mid(chunk_pos, chunk_len);
			paintText(&painter, QPoint(chunk_pos, term_y), s, first_cell);
		}
	}
	if(m_historyLinesOffset == 0) {
		// print cursor
		QPoint cursor_pos = screen_buffer->cursorPosition();
		const core::term::ScreenLine screen_line = screen_buffer->lineAt(start_line_ix + cursor_pos.y());
		core::term::ScreenCell cell = screen_line.value(cursor_pos.x());
		if(cell.isNull()) cell.setLetter(' ');
		// flip reverse attribute
		int atts = cell.attributes();
		atts = atts ^ core::term::ScreenCell::AttrReverse;
		cell.setAttributes(atts);
		paintText(&painter, cursor_pos, QString(cell.letter()), cell);
	}
}

void TerminalWidget::paintText(QPainter *painter, const QPoint &term_pos, const QString &text, const core::term::ScreenCell &text_attrs)
{
	int px_x = term_pos.x() * m_charWidthPx - m_horizontalScrollPx;
	int px_y = term_pos.y() * m_charHeightPx;
	//LOGDEB() << term_pos.x() << term_pos.y() << text;
	QRect r(px_x, px_y, text.length() * m_charWidthPx, m_charHeightPx);
	painter->fillRect(r, brushForCell(text_attrs));
	painter->setPen(penForCell(text_attrs));
	painter->drawText(px_x, px_y + m_charHeightPx - m_charShiftPx, text);
}

QPen TerminalWidget::penForCell(const core::term::ScreenCell &cell)
{
	bool reverse = cell.attributes() & core::term::ScreenCell::AttrReverse;
	QColor color = m_palete.getColor(reverse? cell.bgColor(): cell.fgColor(), cell.attributes() & core::term::ScreenCell::AttrBright);
	//LOGDEB() << Q_FUNC_INFO << color.name();
	//color = m_palete.getColor(7, false);
	return QPen(color);
}

QBrush TerminalWidget::brushForCell(const core::term::ScreenCell &cell)
{
	bool reverse = cell.attributes() & core::term::ScreenCell::AttrReverse;
	QColor color = m_palete.getColor(reverse? cell.fgColor(): cell.bgColor(), false);
	//LOGDEB() << Q_FUNC_INFO << color.name();
	//color = m_palete.getColor(0, false);
	return QBrush(color);
}

void TerminalWidget::resizeEvent(QResizeEvent *ev)
{
	Q_UNUSED(ev);
	setupGeometry();
}

void TerminalWidget::setupGeometry()
{
	core::term::ScreenBuffer *screen_buffer = m_terminal->screenBuffer();
	QSize old_size = screen_buffer->terminalSize();
	QSize sz = geometry().size();
	QSize new_size(sz.width() / m_charWidthPx, sz.height() / m_charHeightPx);
	if(old_size != new_size) {
		screen_buffer->setTerminalSize(new_size);
	}
}

void TerminalWidget::keyPressEvent(QKeyEvent *ev)
{
	//LOGDEB() << __FUNCTION__ << ev->text() << ev->text().toLatin1().toHex();
	bool is_accepted = true;
	core::term::SlavePtyProcess *pty = m_terminal->slavePtyProcess();
	switch(ev->key()) {
		//case Qt::Key_CapsLock:
		//case Qt::Key_Shift:
		//    break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			pty->write("\n", 1);
			break;
		case Qt::Key_Backspace:
			sendKeyBackspace();
			break;
		case Qt::Key_Tab:
			sendKeyTab();
			break;
		case Qt::Key_Escape:
			pty->write("\x1b", 1);
			break;
		case Qt::Key_Up:
			sendKeyUp();
			break;
		case Qt::Key_Down:
			sendKeyDown();
			break;
		case Qt::Key_Right:
			sendKeyRight();
			break;
		case Qt::Key_Left:
			sendKeyLeft();
			break;
		default:
			is_accepted = false;
			QString text = ev->text();
			if(!text.isEmpty()) {
				//QChar c = text[0];
				pty->write(text.toUtf8());
				is_accepted = true;
			}
			break;
	}
	if(is_accepted) {
		ev->accept();
		resetHistoryLinesOffset();
	}
}

qint64 TerminalWidget::sendKey(const char *sequence, int length)
{
	core::term::SlavePtyProcess *pty = m_terminal->slavePtyProcess();
	return pty->write(sequence, length);
}

void TerminalWidget::scrollBy(int x_pixels, int y_lines)
{
	int old_x = m_horizontalScrollPx;
	int old_ofset = m_historyLinesOffset;
	m_horizontalScrollPx -= x_pixels;
	if(m_horizontalScrollPx < 0) m_horizontalScrollPx = 0;
	addHistoryLinesOffset(y_lines);
	if(m_historyLinesOffset != old_ofset || old_x != m_horizontalScrollPx) {
		invalidateAll();
	}
}

void TerminalWidget::addHistoryLinesOffset(int offset)
{
	m_historyLinesOffset += offset;
	// anti wind-up
	core::term::ScreenBuffer *screen_buffer = m_terminal->screenBuffer();
	int start_ix = screen_buffer->firstVisibleLineIndex() - m_historyLinesOffset;
	if(start_ix < 0) {
		m_historyLinesOffset += start_ix;
	}
	if(m_historyLinesOffset < 0) {
		m_historyLinesOffset = 0;
	}
}

void TerminalWidget::resetHistoryLinesOffset()
{
	m_horizontalScrollPx = 0;
	m_historyLinesOffset = 0;
	invalidateAll();
}

void TerminalWidget::wheelEvent(QWheelEvent *ev)
{
	LOGDEB() << __FUNCTION__ << ev->delta();
	bool is_accepted = true;
	// delta 120 == 15 degrees
	// let delta 120 scrolls terminal by 3 lines
	int num_lines = ev->delta() / 40;
	scrollBy(0, num_lines);
	if(is_accepted)
		ev->accept();
}

bool TerminalWidget::event(QEvent *ev)
{
	//LOGDEB() << Q_FUNC_INFO << ev->type();
	bool ret = true;
	if (ev->type() == QEvent::Gesture) {
		ret = gestureEvent(static_cast<QGestureEvent*>(ev));
	}
	else if(ev->type() == QEvent::MouseButtonPress) {
		//LOGDEB() << "MouseButtonPress";
		QMouseEvent *event = static_cast<QMouseEvent*>(ev);
		m_swipeStartPosition = event->pos();
		//m_swipeSpeedTimer.start();
	}
	else if(ev->type() == QEvent::MouseMove) {
		QMouseEvent *event = static_cast<QMouseEvent*>(ev);
		if(!m_swipeStartPosition.isNull()) {
			QPoint end_pos = event->pos();
			int delta_x_px = end_pos.x() - m_swipeStartPosition.x();
			int delta_y_px = end_pos.y() - m_swipeStartPosition.y();
			/*
			int tm_ms = m_swipeSpeedTimer.elapsed();
			double velocity = delta_px / (double)tm_ms;
			LOGDEB() << "delta;" << delta_px << "time:" << tm_ms << "vel:" << velocity;
			*/
			core::term::ScreenBuffer *screen_buffer = m_terminal->screenBuffer();
			int lines = delta_y_px * screen_buffer->terminalSize().height() / size().height();
			static const int HORIZONTAL_SCROLL_TRESHOLD = 50;
			if(lines != 0 || delta_x_px > HORIZONTAL_SCROLL_TRESHOLD || delta_x_px < -HORIZONTAL_SCROLL_TRESHOLD) {
				//LOGDEB() << "MouseMove" << "delta:" << delta_px << "lines:" << lines;
				scrollBy(delta_x_px, lines);
				m_swipeStartPosition = end_pos;
			}
		}
	}
	else if(ev->type() == QEvent::MouseButtonRelease) {
	}
	else
		ret = QWidget::event(ev);
	return ret;
}

bool TerminalWidget::gestureEvent(QGestureEvent *ev)
{
	LOGDEB() << Q_FUNC_INFO;
	bool ret = false;
	if (QGesture *swipe = ev->gesture(Qt::SwipeGesture)) {
		ret = true;
		QSwipeGesture *swipe_gesture = static_cast<QSwipeGesture *>(swipe);
		if (swipe_gesture->state() == Qt::GestureFinished) {
			core::term::ScreenBuffer *screen_buffer = m_terminal->screenBuffer();
			int delta = screen_buffer->terminalSize().height();
			if (swipe_gesture->verticalDirection() == QSwipeGesture::Up) delta = -delta;
			scrollBy(0, delta);
		}
	}
	return ret;
}
