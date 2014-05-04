#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
namespace core {
namespace term {
class SlavePtyProcess;
class Terminal;
}
}

namespace gui {
namespace qt {

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
	explicit MainWindow(core::term::SlavePtyProcess *pty_process, QWidget *parent = 0);
    ~MainWindow();
private slots:
	void on_actQuit_triggered();
	void on_edCommand_returnPressed();
private:
    Ui::MainWindow *ui;
	core::term::Terminal *m_terminal;
};

}
}

#endif // MAINWINDOW_H
