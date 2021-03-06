#include "gui/qt/mainwindow.h"
#include "core/term/slaveptyprocess.h"

#include <QApplication>
#include <QDebug>

#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

#ifdef Q_OS_QNX
#include <unix.h>
#else
#include <pty.h>
#endif
/*
#ifdef QT_DEBUG
#warning "**********************debug"
#endif
#ifdef QT_NO_DEBUG
#warning "no **********************debug"
#endif
*/

int main(int argc, char *argv[])
{
	QString shell_path;
	for(int i=1; i<argc; i++) {
		QString arg = argv[i];
		if(arg == "--shell") {
			i++;
			if(i < argc) {
				shell_path = argv[i];
			}
		}
	}
	if(shell_path.isEmpty()) {
		// check SHELL env var
		shell_path = ::getenv("SHELL");
	}
	if(shell_path.isEmpty()) {
		shell_path = "/bin/sh";
	}

	int fd;
	char slave_pty_name[128];
	/*
	int pipefd[2];
	if( ::pipe( pipefd ) ) {
		perror("pipe");
		return 1;
	}
	*/
	qDebug() << "exec shell:" << shell_path;
	pid_t pid = forkpty(&fd, slave_pty_name, NULL, NULL);
	if (pid == -1) {
		perror("forkpty");
		return 1;
	}
	else if (pid == 0) {
		// child
		//::close(pipefd[0] );
		//::fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

		//setenv("TERM", "vt100", 1);
		setenv("TERM", "xterm", 1);
		QByteArray a1 = shell_path.toLatin1();
		QByteArray a2 = shell_path.section('/', -1).toLatin1();
		if (execlp(a1.constData(), a2.constData(), (void*)0) == -1) {
		//if (execlp("cat", "cat", (void*)0) == -1) {
			perror("execlp");
		}
		fprintf(stderr, "program exited.\n");
		return 1;
	}

	qDebug() << "Child process pid:" << pid;
	qDebug() << "master fd" << fd << "slave PTY name:" << slave_pty_name;

	int flags = ::fcntl(fd, F_GETFL, 0);
	::fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	QApplication a(argc, argv);
	core::term::SlavePtyProcess slave_pty_process(fd, pid);
	if(!slave_pty_process.open(QIODevice::ReadWrite)) {
		qWarning() << "cannot open master fd";
		return 1;
	}

	gui::qt::MainWindow w(&slave_pty_process);
	w.show();
	return a.exec();
}
