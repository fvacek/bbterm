#include "terminal.h"

#include "slaveptyprocess.h"
#include "screenbuffer.h"

#include <QDebug>

using namespace core::term;

Terminal::Terminal(core::term::SlavePtyProcess *pty_process, QObject *parent) :
	QObject(parent), m_slavePtyProcess(pty_process)
{
	m_screenBuffer = new ScreenBuffer(m_slavePtyProcess, this);
	connect(m_slavePtyProcess, SIGNAL(readyRead()), this, SLOT(onPtyProcessReadyRead()));
}

SlavePtyProcess *Terminal::slavePtyProcess()
{
	if(!m_slavePtyProcess)
		qFatal("Slave Pty Process is NULL");
	return m_slavePtyProcess;
}

ScreenBuffer *Terminal::screenBuffer()
{
	if(!m_screenBuffer)
		qFatal("ScreenBuffer is NULL");
	return m_screenBuffer;
}

void Terminal::onPtyProcessReadyRead()
{
	QByteArray ba = m_slavePtyProcess->readAll();
	QString s = QString::fromUtf8(ba);
	m_screenBuffer->processInput(s);
}
