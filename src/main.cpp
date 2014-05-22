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
	int fd;
	char slave_pty_name[128];
	/*
	int pipefd[2];
	if( ::pipe( pipefd ) ) {
		perror("pipe");
		return 1;
	}
	*/
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
		if (execlp("/bin/sh", "sh", (void*)0) == -1) {
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
	/*
	const char* cmd = "ls -l /\n";
	if (write(fd, cmd, strlen(cmd)) == -1) {
		perror("write");
		return 1;
	}

	char buf[255];
	int nread;
	while ((nread = read(fd, buf, 254)) > 0) {
		int i;
		for (i = 0; i < nread; i++) {
			putchar(buf[i]);
		}
	}
	printf("Done\n");
	*/
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
