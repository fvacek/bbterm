#include "terminalwidget.h"

#include <core/term/screenbuffer.h>
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

QPen TerminalWidget::penForCell(const core::term::ScreenCell &cell)
{
	QColor color = m_palete.getColor(cell.fgColor(), cell.attributes() | core::term::ScreenCell::AttrBright);
	return QPen(color);
}

QBrush TerminalWidget::brushForCell(const core::term::ScreenCell &cell)
{
	QColor color = m_palete.getColor(cell.bgColor(), false);
	return QBrush(color);
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
	int start_ix = screen_buffer->firstVisibleLineIndex();
	//LOGDEB() << start_ix << row_count;
	for(int i=start_ix; i<row_count; i++) {
		const core::term::ScreenLine screen_line = screen_buffer->lineAt(i);
		QString line_str = screen_line.toString();
		core::term::ScreenCell first_cell;
		first_cell.setAttributes(0xff);
		int curr_pos = 0;
		int curr_len = 0;
		while(curr_pos + curr_len <= line_str.length()) {
			core::term::ScreenCell cell = screen_line.value(curr_pos + curr_len);
			if(cell.isAllAttributesEqual(first_cell)) {
				curr_len++;
			}
			else {
				if(curr_len > 0) {
					QString s = line_str.mid(curr_pos, curr_len);
					int x = curr_pos * m_charWidthPx;
					int y = (i - start_ix) * m_charHeightPx;
					//LOGDEB() << y << line_str;
					QRect r(x, y, curr_len * m_charWidthPx, m_charHeightPx);
					painter.fillRect(r, brushForCell(first_cell));
					painter.setPen(penForCell(first_cell));
					//painter.setBrush(brushForCell(first_cell));

					painter.drawText(x, y + m_charHeightPx - m_charShiftPx, s);
				}
				first_cell = cell;
				curr_pos += curr_len;
				curr_len = 0;
			}
		}
	}
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


