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

struct ScreenCell
{
public:
	typedef quint16 Color;
	typedef quint8 Attributes;
private:
	class Data : public QSharedData
	{
	public:
		QChar letter;
		Color color;
		Attributes attributes;

		Data(QChar ch = QChar(), quint16 c = 0, quint8 a = 0) : letter(ch), color(c), attributes(a) {}
	};
	QSharedDataPointer<Data> d;
	class NullHelper {};
	ScreenCell(NullHelper);
	static const ScreenCell& sharedNull();
public:
	ScreenCell();
	ScreenCell(QChar letter, quint16 color, quint8 attributes);

	bool isNull() const {
		return d == sharedNull().d;
	}
	QChar letter() const {return d->letter;}
	void setLetter(QChar c) {d->letter = c;}
	Color color() const {return d->color;}
	void setColor(Color c) {d->color = c;}
	Attributes attributes() const {return d->attributes;}
	void setAttributes(Attributes a) {d->attributes = a;}
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

class ScreenBuffer : public QObject
{
	Q_OBJECT
public:
	explicit ScreenBuffer(SlavePtyProcess *slave_pty_process, QObject *parent = 0);
public:
	void setTerminalSize(const QSize &cols_rows);
	QSize terminalSize();
	int rowCount() {return m_lineBuffer.count();}
	ScreenLine lineAt(int ix) {
		return m_lineBuffer.value(ix);
	}
	void processInput(const QString &input);
private:
	int processControlSequence(int start_pos);
	void appendNewLine();
signals:
	void dirtyRegion(const QRect &rect);
public:
	void escape_acsc(const QStringList &params);
	void escape_bel(const QStringList &params);
	void escape_blink(const QStringList &params);
	void escape_bold(const QStringList &params);
	void escape_cbt(const QStringList &params);
	void escape_clear(const QStringList &params);
	void escape_cr(const QStringList &params);
	void escape_csr(const QStringList &params);
	void escape_cub(const QStringList &params);
	void escape_cub1(const QStringList &params);
	void escape_cud(const QStringList &params);
	void escape_cud1(const QStringList &params);
	void escape_cuf(const QStringList &params);
	void escape_cuf1(const QStringList &params);
	void escape_cup(const QStringList &params);
	void escape_cuu(const QStringList &params);
	void escape_cuu1(const QStringList &params);
	void escape_dch(const QStringList &params);
	void escape_dch1(const QStringList &params);
	void escape_dl(const QStringList &params);
	void escape_dl1(const QStringList &params);
	void escape_ech(const QStringList &params);
	void escape_ed(const QStringList &params);
	void escape_el(const QStringList &params);
	void escape_el1(const QStringList &params);
	void escape_enacs(const QStringList &params);
	void escape_home(const QStringList &params);
	void escape_hpa(const QStringList &params);
	void escape_ht(const QStringList &params);
	void escape_hts(const QStringList &params);
	void escape_ich(const QStringList &params);
	void escape_il(const QStringList &params);
	void escape_il1(const QStringList &params);
	void escape_ind(const QStringList &params);
	void escape_indn(const QStringList &params);
	void escape_invis(const QStringList &params);
	void escape_ka1(const QStringList &params);
	void escape_ka3(const QStringList &params);
	void escape_kb2(const QStringList &params);
	void escape_kbct(const QStringList &params);
	void escape_kbs(const QStringList &params);
	void escape_kc1(const QStringList &params);
	void escape_kc3(const QStringList &params);
	void escape_kcbt(const QStringList &params);
	void escape_kcub1(const QStringList &params);
	void escape_kcud1(const QStringList &params);
	void escape_kcuf1(const QStringList &params);
	void escape_kcuu1(const QStringList &params);
	void escape_kent(const QStringList &params);
	void escape_kf0(const QStringList &params);
	void escape_kf1(const QStringList &params);
	void escape_kf10(const QStringList &params);
	void escape_kf2(const QStringList &params);
	void escape_kf3(const QStringList &params);
	void escape_kf4(const QStringList &params);
	void escape_kf5(const QStringList &params);
	void escape_kf6(const QStringList &params);
	void escape_kf7(const QStringList &params);
	void escape_kf8(const QStringList &params);
	void escape_kf9(const QStringList &params);
	void escape_khome(const QStringList &params);
	void escape_kich1(const QStringList &params);
	void escape_lf1(const QStringList &params);
	void escape_lf2(const QStringList &params);
	void escape_lf3(const QStringList &params);
	void escape_lf4(const QStringList &params);
	void escape_mc0(const QStringList &params);
	void escape_mc4(const QStringList &params);
	void escape_mc5(const QStringList &params);
	void escape_nel(const QStringList &params);
	void escape_op(const QStringList &params);
	void escape_rc(const QStringList &params);
	void escape_rep(const QStringList &params);
	void escape_rev(const QStringList &params);
	void escape_ri(const QStringList &params);
	void escape_rin(const QStringList &params);
	void escape_rmacs(const QStringList &params);
	void escape_rmam(const QStringList &params);
	void escape_rmkx(const QStringList &params);
	void escape_rmpch(const QStringList &params);
	void escape_rmso(const QStringList &params);
	void escape_rmul(const QStringList &params);
	void escape_rs2(const QStringList &params);
	void escape_s0ds(const QStringList &params);
	void escape_s1ds(const QStringList &params);
	void escape_s2ds(const QStringList &params);
	void escape_s3ds(const QStringList &params);
	void escape_sc(const QStringList &params);
	void escape_setab(const QStringList &params);
	void escape_setaf(const QStringList &params);
	void escape_sgr(const QStringList &params);
	void escape_sgr0(const QStringList &params);
	void escape_smacs(const QStringList &params);
	void escape_smam(const QStringList &params);
	void escape_smkx(const QStringList &params);
	void escape_smpch(const QStringList &params);
	void escape_smso(const QStringList &params);
	void escape_smul(const QStringList &params);
	void escape_tbc(const QStringList &params);
	void escape_u6(const QStringList &params);
	void escape_u7(const QStringList &params);
	void escape_u8(const QStringList &params);
	void escape_u9(const QStringList &params);
	void escape_vpa(const QStringList &params);
	void escape_sgm(const QStringList &params);
private:
	core::util::RingBuffer<ScreenLine> m_lineBuffer;
	QString m_inputBuffer;
	QSize m_terminalSize; // cols, rows
	SlavePtyProcess *m_slavePtyProcess;
	QPoint m_currentPosition;
	ScreenCell::Color m_currentColor;
	ScreenCell::Attributes m_currentAttributes;
};

}
}

#endif // SCREENBUFFER_H
