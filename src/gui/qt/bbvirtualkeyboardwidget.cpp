#include "bbvirtualkeyboardwidget.h"
#include "bbvirtualkeyboardhandler.h"

#include <core/util/log.h>

#include <bps/virtualkeyboard.h>

#include <QTimer>

using namespace gui::qt;

BBVirtualKeyboardWidget::BBVirtualKeyboardWidget(QWidget *parent)
: QWidget(parent)
{
	connect(BBVirtualKeyboardHandler::instance(), SIGNAL(keyboardVisibleChanged(bool)), this, SLOT(onVKBVisibleChanged(bool)));
	//QTimer::singleShot(200, BBVirtualKeyboardHandler::instance(), SLOT(setKeyboardVisible()));
	//connect(QCoreApplication::instance(), SIGNAL(awake()), this, SLOT(lazyInit()));
}

void BBVirtualKeyboardWidget::lazyInit()
{
	LOGDEB() << Q_FUNC_INFO;
	BBVirtualKeyboardHandler::instance()->setKeyboardVisible(true);
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

