#include "screenbuffer.h"

#include <core/term/slaveptyprocess.h>

#include <QDebug>

#define DBG() qDebug() << __FILE__ << __LINE__
//#define DBG() while(false) qDebug()

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
	*this = sharedNull();
}

ScreenCell::ScreenCell(QChar character, quint16 color, quint8 attributes)
{
	d = new Data(character, color, attributes);
}

ScreenBuffer::ScreenBuffer(SlavePtyProcess *slave_pty_process, QObject *parent)
: QObject(parent), m_slavePtyProcess(slave_pty_process)
{
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

void ScreenBuffer::processInput(const QString &input)
{
	m_inputBuffer += input;
	DBG() << m_inputBuffer;
	int consumed = 0;
	QRect dirty_rect;
	while(consumed < m_inputBuffer.length()) {
		QChar c = m_inputBuffer[consumed];
		if(c >= ' ') {
			if(m_lineBuffer.count() == 0 || m_currentPosition.x() == (terminalSize().width() - 1)) {
				appendNewLine();
			}
			ScreenLine &line = m_lineBuffer.at(m_lineBuffer.count() - 1);
			ScreenCell &cell = line.cellAt(m_currentPosition.x());
			//DBG() << c;
			cell.setCharacter(c);
			cell.setColor(m_currentColor);
			cell.setAttributes(m_currentAttributes);
			//DBG() << line.toString();
			m_currentPosition.setX(m_currentPosition.x() + 1);
			consumed++;
		}
		else {
			int seq_len = processControlSequence(consumed);
			if(seq_len > 0) {
				consumed += seq_len;
			}
			else {
				qWarning() << __FILE__ << __LINE__ << "unrecognized escape sequence";
				consumed++;
			}
		}
	}
	m_inputBuffer = m_inputBuffer.mid(consumed);
	if(consumed > 0) {
		// TODO: implement dirty rect
		emit dirtyRegion(dirty_rect);
	}
}

void ScreenBuffer::appendNewLine()
{
	m_currentPosition.setX(0);
	m_lineBuffer.append(ScreenLine());
}

