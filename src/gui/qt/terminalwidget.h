#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QFont>
#include <QResizeEvent>

namespace core {
namespace term {
class Terminal;
}
}

namespace gui {
namespace qt {


//#define Q_DECL_OVERRIDE

class TerminalWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TerminalWidget(QWidget *parent = 0);
public:
	void setTerminal(core::term::Terminal *t);
protected:
	void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
private:
	void setupGeometry();
	void setupFont(int point_size);
	Q_SLOT void invalidateRegion(const QRect &dirty_rect);
private:
	core::term::Terminal *m_terminal;
	QFont m_font;
	int m_charWidthPx;
	int m_charHeightPx;
};

}
}

#endif // TERMINALWIDGET_H
