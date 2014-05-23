#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

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

class MainWindow : public QWidget
{
    Q_OBJECT
public:
	explicit MainWindow(core::term::SlavePtyProcess *pty_process, QWidget *parent = 0);
    ~MainWindow();
private slots:
	void on_actQuit_triggered();
	void on_btTab_clicked();
	void on_btUp_clicked();
	void on_btDown_clicked();
	void on_btLeft_clicked();
	void on_btRight_clicked();
	void on_btVKB_clicked(bool checked);
private:
    Ui::MainWindow *ui;
	core::term::Terminal *m_terminal;
};

}
}

#endif // MAINWINDOW_H
