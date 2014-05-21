#include "screenbuffer.h"

#include <core/term/slaveptyprocess.h>

#include <QStringList>

//#define NO_BBTERM_LOG_DEBUG
#include <core/util/log.h>

using namespace core::term;

//====================================================
// ScreenLine
//====================================================
ScreenCell &ScreenLine::cellAt(int ix)
{
	if(ix < 0) {
		LOGWARN() << "Internal error, cell index < 0, ix:" << ix;
		ix = 0;
	}
	while(size() <= ix) {
		append(ScreenCell());
	}
	return operator[](ix);
}

//====================================================
// ScreenBuffer
//====================================================
ScreenBuffer::ScreenBuffer(SlavePtyProcess *slave_pty_process, QObject *parent)
: QObject(parent), m_slavePtyProcess(slave_pty_process)
{
	m_currentFgColor = ScreenCell::ColorWhite;
	m_currentBgColor = ScreenCell::ColorBlack;
	m_currentAttributes = ScreenCell::AttrReset;
	appendLine(true);
}

QSize ScreenBuffer::terminalSize()
{
	return m_terminalSize;
}

void ScreenBuffer::setTerminalSize(const QSize &cols_rows)
{
	m_terminalSize = cols_rows;
	m_slavePtyProcess->setSize(cols_rows.width(), cols_rows.height());
}

int ScreenBuffer::firstVisibleLineIndex() const
{
	int start_ix = rowCount() - m_terminalSize.height();
	if(start_ix < 0) start_ix = 0;
	return start_ix;
}

//#define DEBUG_LTPR() {if(!line_to_print_debug.isEmpty()) {LOGDEB() << line_to_print_debug; line_to_print_debug = QString();}}

void ScreenBuffer::processInput(const QString &input)
{
	m_inputBuffer += input;
	LOGDEB() << "processing input:" << input;
	int consumed = 0;
	QRect dirty_rect;
	//QString line_to_print_debug;
	while(consumed < m_inputBuffer.length()) {
		QChar c = m_inputBuffer[consumed];
		if(c >= ' ') {
			//LOGDEB() << "++++" << c;
			//line_to_print_debug += c;
			int ix = firstVisibleLineIndex() + m_cursorPosition.y();
			if(ix >= rowCount()) {
				qCritical() << "attempt to write on not existing line index" << ix << "of" << rowCount();
			}
			else {
				ScreenLine &line = m_lineBuffer.at(ix);
				ScreenCell &cell = line.cellAt(m_cursorPosition.x());
				cell.setLetter(c);
				cell.setColor(m_currentFgColor, m_currentBgColor);
				cell.setAttributes(m_currentAttributes);
			}
			// advance cursor to next position
			m_cursorPosition.rx()++;
			if(m_cursorPosition.x() >= terminalSize().width()) {
				m_cursorPosition.setX(0);
				m_cursorPosition.ry()++;
				while(m_cursorPosition.y() + firstVisibleLineIndex() >= rowCount()) {
						appendLine(false);
				}
			}
			consumed++;
		}
		else {
			//DEBUG_LTPR();
			int seq_len = processControlSequence(consumed);
			if(seq_len > 0) {
				//LOGDEB() << "SEQ LEN:" << seq_len;
				consumed += seq_len;
			}
			else {
				LOGWARN() << "unrecognized escape sequence:" << m_inputBuffer.mid(consumed, 200);
				LOGWARN() << "unrecognized escape sequence:" << m_inputBuffer.mid(consumed, 20).toLatin1().toHex();
				consumed++;
			}
		}
	}
	m_inputBuffer = m_inputBuffer.mid(consumed);
	if(consumed > 0) {
		// TODO: implement dirty rect
		emit dirtyRegion(dirty_rect);
	}
	//DEBUG_LTPR();
	//LOGDEB() << "dump\n" << dump();
}

void ScreenBuffer::appendLine(bool move_cursor)
{
	//LOGDEB() << Q_FUNC_INFO;
	m_lineBuffer.append(ScreenLine());
	if(move_cursor) {
		m_cursorPosition.setX(0);
		int new_y = qMin(rowCount(), m_terminalSize.height()) - 1;
		if(new_y < 0)
			new_y = 0;
		m_cursorPosition.setY(new_y);
	}
}

QString ScreenBuffer::dump() const
{
	QStringList lines;
	int i0 = firstVisibleLineIndex();
	for(int i=0; i<rowCount(); i++) {
		lines << QString("[%1]%2").arg(i - i0, 4, 10, QChar('0')).arg(m_lineBuffer.value(i).toString());
	}
	return lines.join("\n");
}
/*
ScreenLine::AttributedStringList ScreenLine::toAttributedStrings() const
{
	AttributedStringList ret;
	AttributedString curr_attr_str;
	foreach(const ScreenCell &c, *this) {
		int attrs = c.allAttributes();
		if(attrs != curr_attr_str.second) {
			if(!curr_attr_str.first.isEmpty()) {
				ret << curr_attr_str;
			}
		}
		QChar ch = c.letter();
		if(ch.isNull())
			ch = ' ';
		ret += ch;
	}
}
*/

