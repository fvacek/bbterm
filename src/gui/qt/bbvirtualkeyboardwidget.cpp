#include "bbvirtualkeyboardwidget.h"
#include "bbvirtualkeyboardhandler.h"

#include <bps/virtualkeyboard.h>

#include <QDebug>

using namespace gui::qt;

BBVirtualKeyboardWidget::BBVirtualKeyboardWidget(QWidget *parent)
: QWidget(parent)
{
	connect(BBVirtualKeyboardHandler::instance(), SIGNAL(keyboardVisibleChanged(bool)), this, SLOT(onVKBVisibleChanged(bool)));
}

void BBVirtualKeyboardWidget::onVKBVisibleChanged(bool visible)
{
	if(visible) {
		int h = BBVirtualKeyboardHandler::instance()->keyboardHeight();
		setMinimumHeight(h);
		show();
	}
	else {
		hide();
	}
}

