#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <core/term/slaveptyprocess.h>
#include <core/term/terminal.h>

#include <QDebug>

using namespace gui::qt;

MainWindow::MainWindow(core::term::SlavePtyProcess *pty_process, QWidget *parent) :
    QMainWindow(parent),
	ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	m_terminal = new core::term::Terminal(pty_process, this);
	ui->terminalWidget->setTerminal(m_terminal);
	resize(800, 700);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_actQuit_triggered()
{
	qApp->quit();
}

void MainWindow::on_edCommand_returnPressed()
{
	QString command = ui->edCommand->text();
	QByteArray ba = command.toUtf8() + "\n";
	qDebug() << "sending command:" << ba;
	qint64 len = m_terminal->slavePtyProcess()->write(ba);
	if(len != ba.length()) {
		qWarning() << "Only" << len << "of" << ba.length() << "bytes written.";
	}
}

