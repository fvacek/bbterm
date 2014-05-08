#ifndef SCREENBUFFER_H
#define SCREENBUFFER_H

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
private:
	quint32 m_unicode:16;
	quint32 m_fgColor:4;
	quint32 m_bgColor:4;
	quint32 m_attributes:8;
};

class ScreenLine : public QList<ScreenCell>
{
public:
	ScreenCell& cellAt(int ix)
	{
		while(size() <= ix) {
			append(ScreenCell());
		}
		return operator[](ix);
	}

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

struct EscCommand;

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
	QPoint m_currentPosition;
	ScreenCell::Color m_currentFgColor;
	ScreenCell::Color m_currentBgColor;
	ScreenCell::Attributes m_currentAttributes;
public:
	void escape_acsc(EscCommand *esc_cmd, const QStringList &params);
	void escape_bel(EscCommand *esc_cmd, const QStringList &params);
	//void escape_blink(EscCommand *esc_cmd, const QStringList &params);
	void escape_bold(EscCommand *esc_cmd, const QStringList &params);
	void escape_cbt(EscCommand *esc_cmd, const QStringList &params);
	void escape_clear(EscCommand *esc_cmd, const QStringList &params);
	void escape_cr(EscCommand *esc_cmd, const QStringList &params);
	void escape_csr(EscCommand *esc_cmd, const QStringList &params);
	void escape_cub(EscCommand *esc_cmd, const QStringList &params);
	//void escape_cub1(EscCommand *esc_cmd, const QStringList &params);
	void escape_cud(EscCommand *esc_cmd, const QStringList &params);
	//void escape_cud1(EscCommand *esc_cmd, const QStringList &params);
	void escape_cuf(EscCommand *esc_cmd, const QStringList &params);
	//void escape_cuf1(EscCommand *esc_cmd, const QStringList &params);
	void escape_cup(EscCommand *esc_cmd, const QStringList &params);
	void escape_cuu(EscCommand *esc_cmd, const QStringList &params);
	void escape_cuu1(EscCommand *esc_cmd, const QStringList &params);
	void escape_dch(EscCommand *esc_cmd, const QStringList &params);
	void escape_dch1(EscCommand *esc_cmd, const QStringList &params);
	void escape_dl(EscCommand *esc_cmd, const QStringList &params);
	void escape_dl1(EscCommand *esc_cmd, const QStringList &params);
	void escape_ech(EscCommand *esc_cmd, const QStringList &params);
	void escape_ed(EscCommand *esc_cmd, const QStringList &params);
	void escape_el(EscCommand *esc_cmd, const QStringList &params);
	void escape_el1(EscCommand *esc_cmd, const QStringList &params);
	void escape_enacs(EscCommand *esc_cmd, const QStringList &params);
	void escape_home(EscCommand *esc_cmd, const QStringList &params);
	void escape_hpa(EscCommand *esc_cmd, const QStringList &params);
	void escape_ht(EscCommand *esc_cmd, const QStringList &params);
	void escape_hts(EscCommand *esc_cmd, const QStringList &params);
	void escape_ich(EscCommand *esc_cmd, const QStringList &params);
	void escape_il(EscCommand *esc_cmd, const QStringList &params);
	void escape_il1(EscCommand *esc_cmd, const QStringList &params);
	void escape_ind(EscCommand *esc_cmd, const QStringList &params);
	void escape_indn(EscCommand *esc_cmd, const QStringList &params);
	void escape_invis(EscCommand *esc_cmd, const QStringList &params);
	void escape_ka1(EscCommand *esc_cmd, const QStringList &params);
	void escape_ka3(EscCommand *esc_cmd, const QStringList &params);
	void escape_kb2(EscCommand *esc_cmd, const QStringList &params);
	void escape_kbct(EscCommand *esc_cmd, const QStringList &params);
	void escape_kbs(EscCommand *esc_cmd, const QStringList &params);
	void escape_kc1(EscCommand *esc_cmd, const QStringList &params);
	void escape_kc3(EscCommand *esc_cmd, const QStringList &params);
	void escape_kcbt(EscCommand *esc_cmd, const QStringList &params);
	void escape_kcub1(EscCommand *esc_cmd, const QStringList &params);
	void escape_kcud1(EscCommand *esc_cmd, const QStringList &params);
	void escape_kcuf1(EscCommand *esc_cmd, const QStringList &params);
	void escape_kcuu1(EscCommand *esc_cmd, const QStringList &params);
	void escape_kent(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf0(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf1(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf10(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf2(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf3(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf4(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf5(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf6(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf7(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf8(EscCommand *esc_cmd, const QStringList &params);
	void escape_kf9(EscCommand *esc_cmd, const QStringList &params);
	void escape_khome(EscCommand *esc_cmd, const QStringList &params);
	void escape_kich1(EscCommand *esc_cmd, const QStringList &params);
	void escape_lf1(EscCommand *esc_cmd, const QStringList &params);
	void escape_lf2(EscCommand *esc_cmd, const QStringList &params);
	void escape_lf3(EscCommand *esc_cmd, const QStringList &params);
	void escape_lf4(EscCommand *esc_cmd, const QStringList &params);
	void escape_mc0(EscCommand *esc_cmd, const QStringList &params);
	void escape_mc4(EscCommand *esc_cmd, const QStringList &params);
	void escape_mc5(EscCommand *esc_cmd, const QStringList &params);
	void escape_nel(EscCommand *esc_cmd, const QStringList &params);
	void escape_op(EscCommand *esc_cmd, const QStringList &params);
	void escape_rc(EscCommand *esc_cmd, const QStringList &params);
	void escape_rep(EscCommand *esc_cmd, const QStringList &params);
	void escape_rev(EscCommand *esc_cmd, const QStringList &params);
	void escape_ri(EscCommand *esc_cmd, const QStringList &params);
	void escape_rin(EscCommand *esc_cmd, const QStringList &params);
	void escape_rmacs(EscCommand *esc_cmd, const QStringList &params);
	void escape_rmam(EscCommand *esc_cmd, const QStringList &params);
	void escape_rmkx(EscCommand *esc_cmd, const QStringList &params);
	void escape_rmpch(EscCommand *esc_cmd, const QStringList &params);
	//void escape_rmso(EscCommand *esc_cmd, const QStringList &params);
	//void escape_rmul(EscCommand *esc_cmd, const QStringList &params);
	void escape_rs2(EscCommand *esc_cmd, const QStringList &params);
	void escape_s0ds(EscCommand *esc_cmd, const QStringList &params);
	void escape_s1ds(EscCommand *esc_cmd, const QStringList &params);
	void escape_s2ds(EscCommand *esc_cmd, const QStringList &params);
	void escape_s3ds(EscCommand *esc_cmd, const QStringList &params);
	void escape_sc(EscCommand *esc_cmd, const QStringList &params);
	void escape_setab(EscCommand *esc_cmd, const QStringList &params);
	void escape_setaf(EscCommand *esc_cmd, const QStringList &params);
	void escape_sgr(EscCommand *esc_cmd, const QStringList &params);
	//void escape_sgr0(EscCommand *esc_cmd, const QStringList &params);
	void escape_smacs(EscCommand *esc_cmd, const QStringList &params);
	void escape_smam(EscCommand *esc_cmd, const QStringList &params);
	void escape_smkx(EscCommand *esc_cmd, const QStringList &params);
	void escape_smpch(EscCommand *esc_cmd, const QStringList &params);
	//void escape_smso(EscCommand *esc_cmd, const QStringList &params);
	//void escape_smul(EscCommand *esc_cmd, const QStringList &params);
	void escape_tbc(EscCommand *esc_cmd, const QStringList &params);
	void escape_u6(EscCommand *esc_cmd, const QStringList &params);
	void escape_u7(EscCommand *esc_cmd, const QStringList &params);
	void escape_u8(EscCommand *esc_cmd, const QStringList &params);
	void escape_u9(EscCommand *esc_cmd, const QStringList &params);
	void escape_vpa(EscCommand *esc_cmd, const QStringList &params);
	void escape_sgm(EscCommand *esc_cmd, const QStringList &params);

	void escape_charAttr(EscCommand *esc_cmd, const QStringList &params);
	void escape_ignored(EscCommand *esc_cmd, const QStringList &params);
};

}
}

#endif // SCREENBUFFER_H
