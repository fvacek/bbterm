#ifndef SCREENBUFFER_H
#define SCREENBUFFER_H

#include <core/util/ringbuffer.h>
#include <core/util/ringbuffer.h>

#include <QObject>
#include <QSharedData>
#include <QSize>
#include <QPoint>
#include <QRect>

namespace core {
namespace term {

class SlavePtyProcess;

class ScreenCell
{
public:
	enum Attribute {
		AttrReset = 0x00,
		AttrBright = 0x01,
		AttrDim = 0x02,
		AttrUnderscore = 0x04,
		AttrBlink = 0x08,
		AttrReverse = 0x10,
		AttrHidden = 0x20
	};

	enum Colors {
		ColorBlack = 0,
		ColorRed,
		ColorGreen,
		ColorYellow,
		ColorBlue,
		ColorMagenta,
		ColorCyan,
		ColorWhite
	};

	typedef quint8 Color;
	typedef quint8 Attributes;
public:
	ScreenCell(QChar letter = '\x0', Color fg = ColorWhite, Color bg = ColorBlack, Attributes a = AttrReset)
	: m_unicode(letter.unicode()), m_fgColor(fg), m_bgColor(bg), m_attributes(a) {}

	bool isNull() const {
		return m_unicode == 0;
	}
	QChar letter() const {return QChar(m_unicode);}
	void setLetter(QChar c) {m_unicode = c.unicode();}
	Color fgColor() const {return m_fgColor;}
	Color bgColor() const {return m_bgColor;}
	void setColor(Color fg, Color bg) {m_fgColor = fg; m_bgColor = bg;}
	Attributes attributes() const {return m_attributes;}
	void setAttributes(Attributes a) {m_attributes = a;}
	bool isAllAttributesEqual(const ScreenCell &o) {
		return m_fgColor == o.m_fgColor && m_bgColor == o.m_bgColor && m_attributes == o.m_attributes;
	}
	/*
	int allAttributes() const {
		return m_fgColor | (m_bgColor << 4) | (m_attributes << 8);
	}
	void setAllAttributes(int a) {
		m_fgColor =  a & ~(~0 << 4);
		m_bgColor =  (a & ~(~0 << 8)) >> 4;
		m_attributes =  a >> 8;
	}
	*/
private:
	quint32 m_unicode:16;
	quint32 m_fgColor:4;
	quint32 m_bgColor:4;
	quint32 m_attributes:8;
};

class ScreenLine : public QList<ScreenCell>
{
public:
	ScreenCell& cellAt(int ix);
	QString toString() const
	{
		QString ret;
		foreach(const ScreenCell &c, *this) {
			QChar ch = c.letter();
			if(ch.isNull())
				ch = ' ';
			ret += ch;
		}
		return ret;
	}
};

class ScreenBuffer : public QObject
{
	Q_OBJECT
public:
	explicit ScreenBuffer(SlavePtyProcess *slave_pty_process, QObject *parent = 0);
signals:
	void dirtyRegion(const QRect &rect);
public:
	void setTerminalSize(const QSize &cols_rows);
	QSize terminalSize();
	int rowCount() const {
		return m_lineBuffer.count();
	}
	ScreenLine lineAt(int ix) {
		return m_lineBuffer.value(ix);
	}
	int firstVisibleLineIndex() const;
	QPoint cursorPosition() const {return m_cursorPosition;}
	void processInput(const QString &input);
private:
	int processControlSequence(int start_pos);
	void appendLine(bool move_cursor);
	QString dump() const;
private:
	core::util::RingBuffer<ScreenLine> m_lineBuffer;
	QString m_inputBuffer;
	QSize m_terminalSize; // cols, rows
	SlavePtyProcess *m_slavePtyProcess;
	QPoint m_cursorPosition;
	ScreenCell::Color m_currentFgColor;
	ScreenCell::Color m_currentBgColor;
	ScreenCell::Attributes m_currentAttributes;
public:
	void cmdCursorMove(const QStringList &params);
	void cmdCursorMoveRight(const QStringList &params);
	void cmdCursorMoveDown(const QStringList &params);
	void cmdCursorMoveLeft(const QStringList &params);
	void cmdCursorMoveUp(const QStringList &params);

	void cmdCursorSave(const QStringList &params);
	void cmdCursorRestore(const QStringList &params);

	void cmdClearToEndOfLine(const QStringList &params);
	void cmdClearFromBeginningOfLine(const QStringList &params);
	void cmdClearLine(const QStringList &params);

	void cmdClearToEndOfScreen(const QStringList &params);
	void cmdClearFromBeginningOfScreen(const QStringList &params);
	void cmdClearScreen(const QStringList &params);

	void cmdHorizontalTab(const QStringList &params);

	void cmdSetCharAttributes(const QStringList &params);

	void cmdBackSpace(const QStringList &params);
public:
	void escape_cr(const QStringList &params);
	void escape_changeScrollingRegion(const QStringList &params);

	void escape_controlSequenceCommand(const QStringList &params);
	void escape_operatingSystemCommand(const QStringList &params);

	void escape_ignored(const QStringList &params);
};

}
}

#endif // SCREENBUFFER_H
