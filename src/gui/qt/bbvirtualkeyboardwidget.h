#ifndef BBVIRTUALKEYBOARDWIDGET_H
#define BBVIRTUALKEYBOARDWIDGET_H

#include <QWidget>

namespace gui {
namespace qt {

class BBVirtualKeyboardWidget : public QWidget
{
	Q_OBJECT
private:
	typedef QWidget Super;
public:
	explicit BBVirtualKeyboardWidget(QWidget *parent = 0);
public:
	//Q_SLOT void onVKBVisibleChanged(bool visible);
protected:
	//void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
private:
	Q_SLOT void lazyInit();
	Q_SLOT void updateHeightFromVKB();
};

}
}

#endif // BBVIRTUALKEYBOARDWIDGET_H
