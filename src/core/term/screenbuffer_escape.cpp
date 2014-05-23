#include "screenbuffer.h"

#include <core/util/log.h>

#include <QStringList>
#include <QDebug>

//#define DEBUG_ESCAPES_PROCESSING
#ifdef DEBUG_ESCAPES_PROCESSING
#define ESC_DEBUG() qDebug() << "<CTL>" << __FUNCTION__ << "captions:" << params.join(" - ")
#define ESC_DEBUG_IGNORED() qDebug() << "<CTL IGNORED>" << __FUNCTION__ << "captions:" << params.join(" - ")
#else
#define ESC_DEBUG() while(false) qDebug()
#define ESC_DEBUG_IGNORED() while(false) qDebug()
#endif

#define ESC_DEBUG_NIY() qWarning() << "<CTL NIY>" << __FUNCTION__ << "captions:" << params.join(" - ")

using namespace core::term;

struct EscCommand
{
	QLatin1String pattern;
	void (ScreenBuffer::*handler)(const QStringList &);
	bool isRegexp;
	const char *description;
};

static EscCommand esc_commands[] = {
	{QLatin1String("\x7"), &ScreenBuffer::escape_ignored, false, "BELL"},
	{QLatin1String("\x8"), &ScreenBuffer::cmdBackSpace, false, "Back Space"},
	{QLatin1String("\x9"), &ScreenBuffer::cmdHorizontalTab, false, "Horizontal tabulator"},
	{QLatin1String("\xd""\xa"), &ScreenBuffer::cmdCursorMoveDown, false, "CRLF Move down 1 line"},
	{QLatin1String("\xa"), &ScreenBuffer::cmdCursorMoveDown, false, "Move down 1 line"},
	{QLatin1String("\xd"), &ScreenBuffer::escape_cr, false, "Carriage return"},

	{QLatin1String("\x1b<"), &ScreenBuffer::escape_ignored, false, "Enter/exit ANSI mode (VT52) - setansi"},
	{QLatin1String("\x1b="), &ScreenBuffer::escape_ignored, false, "Enter alternate keypad mode - altkeypad"},
	{QLatin1String("\x1b>"), &ScreenBuffer::escape_ignored, false, "Exit alternate keypad mode - numkeypad"},

	{QLatin1String("\x1b""7"), &ScreenBuffer::cmdCursorSave, false, "Save cursor position and attributes"},
	{QLatin1String("\x1b""8"), &ScreenBuffer::cmdCursorRestore, false, "Restore cursor position and attributes"},

	{QLatin1String("^\x001b([\\(\\)])([012AB])"), &ScreenBuffer::escape_ignored, true, "set character set"},

	{QLatin1String("\x1b[K"), &ScreenBuffer::cmdClearToEndOfLine, false, "Clear to end of line"},
	{QLatin1String("\x1b[0K"), &ScreenBuffer::cmdClearToEndOfLine, false, "Clear to end of line"},
	{QLatin1String("\x1b[1K"), &ScreenBuffer::cmdClearFromBeginningOfLine, false, "Clear to beginning of line"},
	{QLatin1String("\x1b[2K"), &ScreenBuffer::cmdClearLine, false, "Clear line"},

	{QLatin1String("\x1b[J"), &ScreenBuffer::cmdClearToEndOfScreen, false, ""},
	{QLatin1String("\x1b[0J"), &ScreenBuffer::cmdClearToEndOfScreen, false, ""},
	{QLatin1String("\x1b[1J"), &ScreenBuffer::cmdClearFromBeginningOfScreen, false, ""},
	{QLatin1String("\x1b[2J"), &ScreenBuffer::cmdClearScreen, false, ""},

	{QLatin1String("^\x001b\\[([?])?(([0-9]+)?(;[0-9]*)*)([a-zA-Z])"), &ScreenBuffer::escape_controlSequenceCommand, true, "ESC [ p1;p2;... C"},
	{QLatin1String("^\x001b\\]([0-9]+);([^\x0007]*)\x0007"), &ScreenBuffer::escape_operatingSystemCommand, true, "ESC ] p1;text ST|BEL"},
};
static const int esc_commands_count = sizeof(esc_commands)/sizeof(EscCommand);

// carriage return
void ScreenBuffer::escape_cr(const QStringList &params)
{

	Q_UNUSED(params);
	ESC_DEBUG();
	m_cursorPosition.setX(0);
}

// Change scrolling region
void ScreenBuffer::escape_changeScrollingRegion(const QStringList &params)
{

	int n1 = params.value(1).toInt();
	int n2 = params.value(2).toInt();
	ESC_DEBUG() << "Enable scrolling from row:" << n1 << "to row:" << n2 << "IGNORED";
}

// Move down # lines
void ScreenBuffer::cmdCursorMoveDown(const QStringList &params)
{
	bool ok;
	int n = params.value(1).toInt(&ok);
	if(!ok)
		n = 1;
	ESC_DEBUG() << n << "lines feed";
	int ix = firstVisibleLineIndex() + m_cursorPosition.y();
	for(int i=0; i<n; i++) {
		ix++;
		if(ix >= rowCount()) {
			appendLine(true);
		}
		else {
			m_cursorPosition.rx() = 0;
			m_cursorPosition.ry()++;
		}
	}
}

// Move cursor left #1 spaces
void ScreenBuffer::cmdCursorMoveLeft(const QStringList &params)
{
	int n = params.value(1).toInt();
	if(n == 0) n = 1;
	ESC_DEBUG() << "n:" << n;
	int x = m_cursorPosition.x() - n;
	if(x < 0) {
		x = 0;
	}
	m_cursorPosition.setX(x);
}

// Move right # spaces
// The CUF sequence moves the active position to the right. The distance moved is determined by the parameter. A parameter value of zero or one moves the active position one position to the right. A parameter value of n moves the active position n positions to the right. If an attempt is made to move the cursor to the right of the right margin, the cursor stops at the right margin.
void ScreenBuffer::cmdCursorMoveRight(const QStringList &params)
{
	int n = params.value(1).toInt();
	if(n == 0) n = 1;
	ESC_DEBUG() << "n:" << n;
	int x = m_cursorPosition.x() + n;
	if(x >= m_terminalSize.width()) {
		x = m_terminalSize.width() - 1;
	}
	m_cursorPosition.setX(x);
}

// Move to row #1 col #2
void ScreenBuffer::cmdCursorMove(const QStringList &params)
{
	bool ok;
	int row = params.value(1).toInt(&ok) - 1;
	if(!ok) row = 0;
	int col = params.value(2).toInt(&ok) - 1;
	if(!ok) col = 0;
	ESC_DEBUG() << "move cursor to row:" << row << "col:" << col;
	if(row >= 0 && row < m_terminalSize.height()) {
		if(col >= 0 && col < m_terminalSize.width()) {
			while(rowCount() <= row) {
				appendLine(false);
			}
			m_cursorPosition.setX(col);
			m_cursorPosition.setY(row);
		}
		else {
			LOGWARN() << __FUNCTION__ << "col:" << row << "out of range: 0 -" << (m_terminalSize.width() - 1);
		}
	}
	else {
		LOGWARN() << __FUNCTION__ << "row:" << row << "out of range: 0 -" << (m_terminalSize.height() - 1);
	}
}

// Move cursor up #1 lines
void ScreenBuffer::cmdCursorMoveUp(const QStringList &params)
{
	int n = params.value(1).toInt();
	if(n == 0) n = 1;
	ESC_DEBUG() << "n:" << n;
	int y = m_cursorPosition.y() - n;
	if(y < 0) { y = 0; }
	m_cursorPosition.setY(y);
}

// Clear to end of display
void ScreenBuffer::cmdClearToEndOfScreen(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG();
	QPoint pos = m_cursorPosition;
	cmdClearToEndOfLine(QStringList());
	for(int y = pos.y() + 1; y<m_terminalSize.height(); y++) {
		m_cursorPosition.setX(0);
		m_cursorPosition.setY(y);
		cmdClearToEndOfLine(QStringList());
	}
	m_cursorPosition = pos;
}

void ScreenBuffer::cmdClearFromBeginningOfScreen(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG();
	QPoint pos = m_cursorPosition;
	cmdClearFromBeginningOfLine(QStringList());
	for(int y = 0; y<pos.y(); y++) {
		m_cursorPosition.setX(0);
		m_cursorPosition.setY(y);
		cmdClearToEndOfLine(QStringList());
	}
	m_cursorPosition = pos;
}

void ScreenBuffer::cmdClearScreen(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG();
	m_cursorPosition = QPoint(0, 0);
	cmdClearToEndOfScreen(QStringList());
}

// Clear to end of line
void ScreenBuffer::cmdClearToEndOfLine(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG();
	int row = firstVisibleLineIndex() + m_cursorPosition.y();
	if(row < rowCount()) {
		ScreenLine &line = m_lineBuffer.at(row);
		for(int i=m_cursorPosition.x(); i<m_terminalSize.width() && i<line.length(); i++) {
			ScreenCell &cell = line.cellAt(i);
			cell.setLetter(QChar());
			cell.setColor(ScreenCell::ColorWhite, ScreenCell::ColorBlack);
			cell.setAttributes(ScreenCell::AttrReset);
		}
	}
}

// Clear from beginning of line to cursor
void ScreenBuffer::cmdClearFromBeginningOfLine(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG() << "Clear from beginning of line to cursor";
	int row = firstVisibleLineIndex() + m_cursorPosition.y();
	if(row < rowCount()) {
		ScreenLine &line = m_lineBuffer.at(row);
		for(int i=0; i<=m_cursorPosition.x(); i++) {
			ScreenCell &cell = line.cellAt(i);
			cell.setLetter(QChar());
			cell.setColor(ScreenCell::ColorWhite, ScreenCell::ColorBlack);
			cell.setAttributes(ScreenCell::AttrReset);
		}
	}
}

void ScreenBuffer::cmdClearLine(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG() << "Clear line";
	int row = firstVisibleLineIndex() + m_cursorPosition.y();
	if(row < rowCount()) {
		ScreenLine &line = m_lineBuffer.at(row);
		for(int i=0; i<=line.length(); i++) {
			ScreenCell &cell = line.cellAt(i);
			cell.setLetter(QChar());
			cell.setColor(ScreenCell::ColorWhite, ScreenCell::ColorBlack);
			cell.setAttributes(ScreenCell::AttrReset);
		}
	}
}

void ScreenBuffer::cmdCursorSave(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG_IGNORED();
}

void ScreenBuffer::cmdCursorRestore(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG_IGNORED();
}

// tab to next 8-space hardware tab stop
void ScreenBuffer::cmdHorizontalTab(const QStringList &params)
{
	Q_UNUSED(params);
	static const int tab_width = 8;
	int x = m_cursorPosition.x();
	x = (x / tab_width + 1) * tab_width;
	m_cursorPosition.setX(x);
	if(m_cursorPosition.x() >= m_terminalSize.width()) {
		//appendLine();
	}
}

// backspace key
void ScreenBuffer::cmdBackSpace(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG();
	m_cursorPosition.rx()--;
	if(m_cursorPosition.x() < 0) {
		m_cursorPosition.ry()--;
		m_cursorPosition.rx() = 0;
		if(m_cursorPosition.y() < 0)
			m_cursorPosition.ry() = 0;
	}
	#ifdef BACKSPACE_DELETES
	int row = firstVisibleLineIndex() + m_cursorPosition.y();
	if(row < rowCount()) {
		ScreenLine &line = m_lineBuffer.at(row);
		ScreenCell &cell = line.cellAt(m_cursorPosition.x());
		cell.setLetter(QChar());
	}
	#endif
}

void ScreenBuffer::cmdSetCharAttributes(const QStringList &params)
{
	ESC_DEBUG();
	foreach(QString param, params) {
		int n = param.toInt();
		switch(n) {
		case 0:
			m_currentAttributes = 0;
			break;
		case 1:
			m_currentAttributes |= ScreenCell::AttrBright;
			break;
		case 2:
			m_currentAttributes |= ScreenCell::AttrDim;
			break;
		case 4:
			m_currentAttributes |= ScreenCell::AttrUnderscore;
			break;
		case 5:
			m_currentAttributes |= ScreenCell::AttrBlink;
			break;
		case 7:
			m_currentAttributes |= ScreenCell::AttrReverse;
			break;
		case 8:
			m_currentAttributes |= ScreenCell::AttrHidden;
			break;
		default:
			if(n >= 30 && n <= 39) {
				n -= 30;
				if(n > 7) {
					// set default fg
					n = ScreenCell::ColorWhite;
				}
				m_currentFgColor = n;
			}
			else if(n >= 40 && n <= 49) {
				n -= 40;
				if(n > 7) {
					// set default bg
					n = ScreenCell::ColorBlack;
				}
				m_currentBgColor = n;
			}
			else {
				LOGWARN() << "invalid character attribute value:" << n;
			}
		}
	}
}

void ScreenBuffer::escape_ignored(const QStringList &params)
{
	Q_UNUSED(params);
	ESC_DEBUG_IGNORED();
}

void ScreenBuffer::escape_controlSequenceCommand(const QStringList &params)
{
	ESC_DEBUG();
	QString cmd_str = params.value(5);
	if(cmd_str.isEmpty()) {
		LOGWARN() << "internal error: empty command, this should never happen";
	}
	else {
		QString mod = params.value(1);
		QStringList captions;
		captions << params.value(0) << params.value(2).split(';');
		int cmd = cmd_str[0].unicode();
		if(mod.isEmpty()) {
			switch(cmd) {
			case 'A': cmdCursorMoveUp(captions); break;
			case 'B': cmdCursorMoveDown(captions); break;
			case 'C': cmdCursorMoveRight(captions); break;
			case 'D': cmdCursorMoveLeft(captions); break;
			case 'H': cmdCursorMove(captions); break;
			case 'h': ESC_DEBUG_IGNORED() << "Set Mode (SM)"; break;
			case 'l': ESC_DEBUG_IGNORED() << "Reset Mode (RM)"; break;
			case 'm': cmdSetCharAttributes(captions); break;
			case 'r': ESC_DEBUG_IGNORED() << "Set top and bottom lines of a window"; break;
			default: ESC_DEBUG_NIY(); break;
			}
		}
		else if(mod == "?") {
			switch(cmd) {
			case 'h': ESC_DEBUG_IGNORED() << "DEC Private Mode Set (DECSET)"; break;
			case 'l': ESC_DEBUG_IGNORED() << "DEC Private Mode Reset (DECRST)"; break;
			case 's': ESC_DEBUG_IGNORED() << "Save DEC Private Mode Values. Ps values are the same as for DECSET."; break;
			default: ESC_DEBUG_NIY(); break;
			}
		}
		else {
			LOGWARN() << "internal error: unknown mode command:" << mod << "- this should never happen";
		}
	}
}

void ScreenBuffer::escape_operatingSystemCommand(const QStringList &params)
{
	ESC_DEBUG();
	QString cmd_str = params.value(1);
	if(cmd_str.isEmpty()) {
		LOGWARN() << "internal error: empty command, this should never happen";
	}
	else {
		int cmd = cmd_str.toInt();
		QString text_param = params.value(2);
		ESC_DEBUG_IGNORED() << cmd << text_param;
	}
}

static QMap<QString, QRegExp> seqRegExps;

int ScreenBuffer::processControlSequence(int start_pos)
{
	EscCommand *cmds = esc_commands;
	int cmdn = esc_commands_count;
	QStringList params;
	int cmd_index = -1;
	int matched_length = -1;
	for(int i=0; i<cmdn; i++) {
		EscCommand cmd = cmds[i];
		QLatin1String pattern = cmd.pattern;
		if(cmd.isRegexp) {
			QRegExp rx = seqRegExps.value(pattern);
			if(rx.isEmpty()) {
				rx = QRegExp(pattern);
				if(!rx.isValid()) {
					LOGWARN() << "Invalid escapesequence" << pattern;
				}
				seqRegExps[pattern] = rx;
			}
			if(rx.indexIn(m_inputBuffer, start_pos, QRegExp::CaretAtOffset) == start_pos) {
				cmd_index = i;
				params = rx.capturedTexts();
				matched_length = rx.matchedLength();
				break;
			}
		}
		else {
			QStringRef str_ref(&m_inputBuffer, start_pos, m_inputBuffer.length() - start_pos);
			if(str_ref.startsWith(pattern)) {
				cmd_index = i;
				QString param = m_inputBuffer.mid(start_pos, ::strlen(pattern.latin1()));
				matched_length = param.length();
				params << param;
				break;
			}
		}
	}
	if(cmd_index >= 0) {
		/*
		if(matched_length > 50) {
			LOGWARN() << "################" << params.value(0);
		}
		*/
		(*this.*(cmds[cmd_index].handler))(params);
		return matched_length;
	}
	return 0;
}
