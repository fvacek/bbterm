#include "screenbuffer.h"

#include <core/term/slaveptyprocess.h>

#include <QStringList>

//#define NO_BBTERM_LOG_DEBUG
#include <core/util/log.h>

using namespace core::term;

const ScreenCell & ScreenCell::sharedNull()
{
	static ScreenCell n = ScreenCell(NullHelper());
	return n;
}

ScreenCell::ScreenCell(ScreenCell::NullHelper)
{
	d = new Data();
}

ScreenCell::ScreenCell()
{
LOGDEB() << "*************" << sizeof(D);
qFatal("************");
	*this = sharedNull();
}

ScreenCell::ScreenCell(QChar letter, Color fg, Color bg, Attributes attributes)
{
	d = new Data(letter, fg, bg, attributes);
}

ScreenBuffer::ScreenBuffer(SlavePtyProcess *slave_pty_process, QObject *parent)
: QObject(parent), m_slavePtyProcess(slave_pty_process)
{
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

void ScreenBuffer::processInput(const QString &input)
{
	m_inputBuffer += input;
	//LOGDEB() << "processing input:" << input;
	int consumed = 0;
	QRect dirty_rect;
	while(consumed < m_inputBuffer.length()) {
		QChar c = m_inputBuffer[consumed];
		if(c >= ' ') {
			//LOGDEB() << "++++" << c;
			int ix = firstVisibleLineIndex() + m_currentPosition.y();
			if(ix >= rowCount()) {
				qCritical() << "attempt to write on not existing line index" << ix << "of" << rowCount();
			}
			else {
				ScreenLine &line = m_lineBuffer.at(ix);
				ScreenCell &cell = line.cellAt(m_currentPosition.x());
				//DBG() << c;
				cell.setLetter(c);
				cell.setColor(m_currentFgColor, m_currentBgColor);
				cell.setAttributes(m_currentAttributes);
			}
			// advance cursor to next position
			m_currentPosition.setX(m_currentPosition.x() + 1);
			if(m_currentPosition.x() >= terminalSize().width()) {
				//LOGDEB() << "@@@@@@@@@@@@" << m_lineBuffer.value(m_lineBuffer.count() - 1).toString();
				appendLine(true);
			}
			consumed++;
		}
		else {
			int seq_len = processControlSequence(consumed);
			if(seq_len > 0) {
				consumed += seq_len;
			}
			else {
				LOGWARN() << "unrecognized escape sequence:"
				<< m_inputBuffer.mid(consumed, 1).toUtf8().toHex()
				<< m_inputBuffer.mid(consumed);
				consumed++;
			}
		}
	}
	m_inputBuffer = m_inputBuffer.mid(consumed);
	if(consumed > 0) {
		// TODO: implement dirty rect
		emit dirtyRegion(dirty_rect);
	}
	LOGDEB() << "dump\n" << dump();
}

void ScreenBuffer::appendLine(bool move_cursor)
{
	m_lineBuffer.append(ScreenLine());
	if(move_cursor) {
		m_currentPosition.setX(0);
		int new_y = qMin(rowCount(), m_terminalSize.height()) - 1;
		if(new_y < 0)
			new_y = 0;
		m_currentPosition.setY(new_y);
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

