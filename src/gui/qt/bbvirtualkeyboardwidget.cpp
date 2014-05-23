#include "bbvirtualkeyboardwidget.h"
#include "bbvirtualkeyboardhandler.h"

#include <core/util/log.h>

#include <bps/virtualkeyboard.h>

#include <QTimer>

using namespace gui::qt;

BBVirtualKeyboardWidget::BBVirtualKeyboardWidget(QWidget *parent)
: Super(parent)
{
	connect(BBVirtualKeyboardHandler::instance(), SIGNAL(keyboardVisibleChanged(bool)), this, SLOT(updateHeightFromVKB()));
	connect(BBVirtualKeyboardHandler::instance(), SIGNAL(keyboardLayoutChanged()), this, SLOT(updateHeightFromVKB()));
	//QTimer::singleShot(200, BBVirtualKeyboardHandler::instance(), SLOT(setKeyboardVisible()));
	//connect(QCoreApplication::instance(), SIGNAL(awake()), this, SLOT(lazyInit()));
}

void BBVirtualKeyboardWidget::lazyInit()
{
	LOGDEB() << Q_FUNC_INFO;
	BBVirtualKeyboardHandler::instance()->setKeyboardVisible(true);
}
/*
void BBVirtualKeyboardWidget::onVKBVisibleChanged(bool visible)
{
	Q_UNUSED(visible);
	updateHeightFromVKB();
}

void BBVirtualKeyboardWidget::resizeEvent(QResizeEvent *ev)
{
	Super::resizeEvent(ev);
	QMetaObject::invokeMethod(this, "updateHeightFromVKB", Qt::QueuedConnection);
}
*/
void BBVirtualKeyboardWidget::updateHeightFromVKB()
{
	LOGDEB() << Q_FUNC_INFO;
	bool visible = BBVirtualKeyboardHandler::instance()->isKeyboardVisible();
	if(visible) {
		int h = BBVirtualKeyboardHandler::instance()->keyboardHeight();
		setMinimumHeight(h);
		setMaximumHeight(h);
		show();
	}
	else {
		hide();
	}
}

