#include "slaveptyprocess.h"

#include <QSocketNotifier>
#include <QDebug>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>

using namespace core::term;

SlavePtyProcess::SlavePtyProcess(int master_fd, pid_t pid, QObject *parent)
: QIODevice(parent), m_masterFd(master_fd), m_pid(pid)
{
	{
		struct ::termios ttmode;
		::tcgetattr(m_masterFd, &ttmode);
		/*
		if (!_xonXoff)
			ttmode.c_iflag &= ~(IXOFF | IXON);
		else
			ttmode.c_iflag |= (IXOFF | IXON);
			*/
		//ttmode.c_iflag |= IUTF8;
		ttmode.c_cc[VERASE] = '\x08';

		if (!tcsetattr(m_masterFd, TCSANOW, &ttmode))
			qWarning() << "Unable to set terminal attributes.";
	}
	m_readNotifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
	connect(m_readNotifier, SIGNAL(activated(int)), this, SIGNAL(readyRead()));
	//m_writeNotifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Write, this);
	//connect(m_writeNotifier, SIGNAL(activated(int)), this, SIGNAL(readyRead());
}

void SlavePtyProcess::setSize(int cols, int rows)
{
	qDebug() << Q_FUNC_INFO << "Resize terimnal to" << cols << "x" << rows;

	struct winsize window_size;

	window_size.ws_row = rows;
	window_size.ws_col = cols;
	window_size.ws_xpixel = cols;
	window_size.ws_ypixel = rows;

	int ret = ::ioctl(m_masterFd, TIOCSWINSZ, &window_size);

	if( ret != -1 ) {
		// Now our internals are up-to-date, notify the application
		ret = ::kill(m_pid, SIGWINCH);
		if(ret != 0) {
			qWarning() << "Error send signal to proces PID:" << m_pid << ::strerror(errno);
		}
	}
	else {
		qWarning() << "Error resize terimnal to" << cols << "x" << rows << ::strerror(errno);
	}
}

qint64 SlavePtyProcess::readData(char *data, qint64 max_size)
{
	//qDebug() << Q_FUNC_INFO;
	m_readNotifier->setEnabled(false);
	qint64 ret = ::read(m_masterFd, data, max_size);
	if(ret > 0) {
		QByteArray ba(data, ret);
		qDebug() << ret << "bytes read" << ba << "HEX:" << ba.toHex();
	}
	m_readNotifier->setEnabled(true);
	return ret;
}

qint64 SlavePtyProcess::writeData(const char *data, qint64 max_size)
{
	qDebug() << Q_FUNC_INFO;
	qint64 ret = ::write(m_masterFd, data, max_size);
	if(ret > 0) {
		QByteArray ba(data, ret);
		qDebug() << ret << "bytes written" << ba << "HEX:" << ba.toHex();
	}
	if(ret < 0) {
		QString s = QString::fromUtf8(strerror(errno));
		qWarning() << "error write data to" << m_masterFd << ":" << s;
	}
	return ret;
}
