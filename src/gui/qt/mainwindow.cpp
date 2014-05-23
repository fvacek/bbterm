#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <core/term/slaveptyprocess.h>
#include <core/term/terminal.h>

#ifdef Q_OS_BLACKBERRY
#include "bbvirtualkeyboardhandler.h"
#include "bbvirtualkeyboardwidget.h"
#endif

#include <QDebug>

using namespace gui::qt;

MainWindow::MainWindow(core::term::SlavePtyProcess *pty_process, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	m_terminal = new core::term::Terminal(pty_process, this);
	ui->terminalWidget->setTerminal(m_terminal);
	#ifdef Q_OS_BLACKBERRY
	ui->mainLayout->addWidget(new BBVirtualKeyboardWidget(this));
	connect(BBVirtualKeyboardHandler::instance(), SIGNAL(keyboardVisibleChanged(bool)), this, SLOT(onVkbVisibleCchanged(bool)));
	#else
	ui->btVKB->hide();
	resize(800, 700);
	#endif
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_actQuit_triggered()
{
	qApp->quit();
}

void MainWindow::on_btTab_clicked()
{
	ui->terminalWidget->pushKeyTab();
}

void MainWindow::on_btUp_clicked()
{
	ui->terminalWidget->pushKeyUp();
}

void MainWindow::on_btDown_clicked()
{
	ui->terminalWidget->pushKeyDown();
}

void MainWindow::on_btLeft_clicked()
{
	ui->terminalWidget->pushKeyLeft();
}

void MainWindow::on_btRight_clicked()
{
	ui->terminalWidget->pushKeyRight();
}

void MainWindow::on_btVKB_clicked(bool checked)
{
#ifdef Q_OS_BLACKBERRY
	BBVirtualKeyboardHandler::instance()->setKeyboardVisible(checked);
#endif
}

void MainWindow::onVkbVisibleCchanged(bool visible)
{
	ui->btVKB->setChecked(visible);
}
/*
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
*/
