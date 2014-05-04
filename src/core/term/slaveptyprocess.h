#ifndef SLAVEPTYPROCESS_H
#define SLAVEPTYPROCESS_H

#include <QIODevice>

class QSocketNotifier;

namespace core {
namespace term {

class SlavePtyProcess : public QIODevice
{
	Q_OBJECT
public:
	explicit SlavePtyProcess(int master_fd, pid_t pid, QObject *parent = 0);
public:
	void setSize(int cols, int rows);
protected:
	virtual qint64 readData(char* data, qint64 maxSize);
	virtual qint64 writeData(const char* data, qint64 maxSize);
signals:
public slots:
	//void sendCommand(const QString &cmd);
private:
	int m_masterFd;
	pid_t m_pid;
	QSocketNotifier *m_readNotifier;
	//QSocketNotifier *m_writeNotifier;
};

}
}

#endif // SLAVEPTYPROCESS_H
