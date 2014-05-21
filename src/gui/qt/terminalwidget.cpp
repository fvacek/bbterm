#include "terminalwidget.h"

#include <core/term/screenbuffer.h>
#include <core/term/slaveptyprocess.h>
#include <core/term/terminal.h>

#include <QPainter>
#include <QColor>

//#define NO_BBTERM_LOG_DEBUG
#include <core/util/log.h>

using namespace gui::qt;

TerminalWidget::TerminalWidget(QWidget *parent)
: QWidget(parent)
{
	setupFont(8);
}

void TerminalWidget::setupFont(int point_size)
{
#ifdef Q_OS_QNX
		m_font = QFont("Andale Mono");
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
	LOGDEB() << Q_FUNC_INFO;
	Q_UNUSED(dirty_rect);
	update();
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
	int start_line_ix = screen_buffer->firstVisibleLineIndex();
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
	{
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
	int px_x = term_pos.x() * m_charWidthPx;
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
	LOGDEB() << __FUNCTION__ << ev->text() << ev->text().toLatin1().toHex();
	bool is_accepted = true;
	core::term::SlavePtyProcess *pty = m_terminal->slavePtyProcess();
	switch(ev->key()) {
		//case Qt::Key_CapsLock:
		//case Qt::Key_Shift:
		//    break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			pty->write("\n", 1 );
			break;
		case Qt::Key_Backspace:
			pty->write("\b", 1 );
			break;
		case Qt::Key_Tab:
			pty->write("\t", 1 );
			break;
		case Qt::Key_Escape:
			pty->write("\x1b", 1 );
			break;
		case Qt::Key_Up:
			pty->write("\x1b[A", 3 );
			break;
		case Qt::Key_Down:
			pty->write("\x1b[B", 3 );
			break;
		case Qt::Key_Right:
			pty->write("\x1b[C", 3 );
			break;
		case Qt::Key_Left:
			pty->write("\x1b[D", 3 );
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
	if(is_accepted)
		ev->accept();
}


