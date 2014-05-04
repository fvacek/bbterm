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
		QChar character;
		Color color;
		Attributes attributes;

		Data(QChar ch = QChar(), quint16 c = 0, quint8 a = 0) : character(ch), color(c), attributes(a) {}
	};
	QSharedDataPointer<Data> d;
	class NullHelper {};
	ScreenCell(NullHelper);
	static const ScreenCell& sharedNull();
public:
	ScreenCell();
	ScreenCell(QChar character, quint16 color, quint8 attributes);

	bool isNull() const {
		return d == sharedNull().d;
	}
	QChar character() const {return d->character;}
	void setCharacter(QChar c) {d->character = c;}
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
			ret += c.character();
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
	int escape_acsc(int matched_len, const QStringList &params);
	int escape_bel(int matched_len, const QStringList &params);
	int escape_blink(int matched_len, const QStringList &params);
	int escape_bold(int matched_len, const QStringList &params);
	int escape_cbt(int matched_len, const QStringList &params);
	int escape_clear(int matched_len, const QStringList &params);
	int escape_cr(int matched_len, const QStringList &params);
	int escape_csr(int matched_len, const QStringList &params);
	int escape_cub(int matched_len, const QStringList &params);
	int escape_cub1(int matched_len, const QStringList &params);
	int escape_cud(int matched_len, const QStringList &params);
	int escape_cud1(int matched_len, const QStringList &params);
	int escape_cuf(int matched_len, const QStringList &params);
	int escape_cuf1(int matched_len, const QStringList &params);
	int escape_cup(int matched_len, const QStringList &params);
	int escape_cuu(int matched_len, const QStringList &params);
	int escape_cuu1(int matched_len, const QStringList &params);
	int escape_dch(int matched_len, const QStringList &params);
	int escape_dch1(int matched_len, const QStringList &params);
	int escape_dl(int matched_len, const QStringList &params);
	int escape_dl1(int matched_len, const QStringList &params);
	int escape_ech(int matched_len, const QStringList &params);
	int escape_ed(int matched_len, const QStringList &params);
	int escape_el(int matched_len, const QStringList &params);
	int escape_el1(int matched_len, const QStringList &params);
	int escape_enacs(int matched_len, const QStringList &params);
	int escape_home(int matched_len, const QStringList &params);
	int escape_hpa(int matched_len, const QStringList &params);
	int escape_ht(int matched_len, const QStringList &params);
	int escape_hts(int matched_len, const QStringList &params);
	int escape_ich(int matched_len, const QStringList &params);
	int escape_il(int matched_len, const QStringList &params);
	int escape_il1(int matched_len, const QStringList &params);
	int escape_ind(int matched_len, const QStringList &params);
	int escape_indn(int matched_len, const QStringList &params);
	int escape_invis(int matched_len, const QStringList &params);
	int escape_ka1(int matched_len, const QStringList &params);
	int escape_ka3(int matched_len, const QStringList &params);
	int escape_kb2(int matched_len, const QStringList &params);
	int escape_kbct(int matched_len, const QStringList &params);
	int escape_kbs(int matched_len, const QStringList &params);
	int escape_kc1(int matched_len, const QStringList &params);
	int escape_kc3(int matched_len, const QStringList &params);
	int escape_kcbt(int matched_len, const QStringList &params);
	int escape_kcub1(int matched_len, const QStringList &params);
	int escape_kcud1(int matched_len, const QStringList &params);
	int escape_kcuf1(int matched_len, const QStringList &params);
	int escape_kcuu1(int matched_len, const QStringList &params);
	int escape_kent(int matched_len, const QStringList &params);
	int escape_kf0(int matched_len, const QStringList &params);
	int escape_kf1(int matched_len, const QStringList &params);
	int escape_kf10(int matched_len, const QStringList &params);
	int escape_kf2(int matched_len, const QStringList &params);
	int escape_kf3(int matched_len, const QStringList &params);
	int escape_kf4(int matched_len, const QStringList &params);
	int escape_kf5(int matched_len, const QStringList &params);
	int escape_kf6(int matched_len, const QStringList &params);
	int escape_kf7(int matched_len, const QStringList &params);
	int escape_kf8(int matched_len, const QStringList &params);
	int escape_kf9(int matched_len, const QStringList &params);
	int escape_khome(int matched_len, const QStringList &params);
	int escape_kich1(int matched_len, const QStringList &params);
	int escape_lf1(int matched_len, const QStringList &params);
	int escape_lf2(int matched_len, const QStringList &params);
	int escape_lf3(int matched_len, const QStringList &params);
	int escape_lf4(int matched_len, const QStringList &params);
	int escape_mc0(int matched_len, const QStringList &params);
	int escape_mc4(int matched_len, const QStringList &params);
	int escape_mc5(int matched_len, const QStringList &params);
	int escape_nel(int matched_len, const QStringList &params);
	int escape_op(int matched_len, const QStringList &params);
	int escape_rc(int matched_len, const QStringList &params);
	int escape_rep(int matched_len, const QStringList &params);
	int escape_rev(int matched_len, const QStringList &params);
	int escape_ri(int matched_len, const QStringList &params);
	int escape_rin(int matched_len, const QStringList &params);
	int escape_rmacs(int matched_len, const QStringList &params);
	int escape_rmam(int matched_len, const QStringList &params);
	int escape_rmkx(int matched_len, const QStringList &params);
	int escape_rmpch(int matched_len, const QStringList &params);
	int escape_rmso(int matched_len, const QStringList &params);
	int escape_rmul(int matched_len, const QStringList &params);
	int escape_rs2(int matched_len, const QStringList &params);
	int escape_s0ds(int matched_len, const QStringList &params);
	int escape_s1ds(int matched_len, const QStringList &params);
	int escape_s2ds(int matched_len, const QStringList &params);
	int escape_s3ds(int matched_len, const QStringList &params);
	int escape_sc(int matched_len, const QStringList &params);
	int escape_setab(int matched_len, const QStringList &params);
	int escape_setaf(int matched_len, const QStringList &params);
	int escape_sgr(int matched_len, const QStringList &params);
	int escape_sgr0(int matched_len, const QStringList &params);
	int escape_smacs(int matched_len, const QStringList &params);
	int escape_smam(int matched_len, const QStringList &params);
	int escape_smkx(int matched_len, const QStringList &params);
	int escape_smpch(int matched_len, const QStringList &params);
	int escape_smso(int matched_len, const QStringList &params);
	int escape_smul(int matched_len, const QStringList &params);
	int escape_tbc(int matched_len, const QStringList &params);
	int escape_u6(int matched_len, const QStringList &params);
	int escape_u7(int matched_len, const QStringList &params);
	int escape_u8(int matched_len, const QStringList &params);
	int escape_u9(int matched_len, const QStringList &params);
	int escape_vpa(int matched_len, const QStringList &params);
	int escape_sgm(int matched_len, const QStringList &params);
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
