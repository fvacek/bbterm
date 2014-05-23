#ifndef BBVIRTUALKEYBOARDWIDGET_H
#define BBVIRTUALKEYBOARDWIDGET_H

#include <QWidget>

namespace gui {
namespace qt {

class BBVirtualKeyboardWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BBVirtualKeyboardWidget(QWidget *parent = 0);
public:
	Q_SLOT void onVKBVisibleChanged(bool visible);
private:
	Q_SLOT void lazyInit();
};

}
}

#endif // BBVIRTUALKEYBOARDWIDGET_H
