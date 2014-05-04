#ifndef TERMINAL_H
#define TERMINAL_H

#include <QObject>

namespace core {
namespace term {

class ScreenBuffer;
class SlavePtyProcess;

class Terminal : public QObject
{
	Q_OBJECT
public:
	explicit Terminal(core::term::SlavePtyProcess *pty_process, QObject *parent = 0);
public:
	SlavePtyProcess* slavePtyProcess();
	ScreenBuffer* screenBuffer();
private slots:
	void onPtyProcessReadyRead();
private:
	SlavePtyProcess *m_slavePtyProcess;
	ScreenBuffer *m_screenBuffer;
};

}
}

#endif // TERMINAL_H
