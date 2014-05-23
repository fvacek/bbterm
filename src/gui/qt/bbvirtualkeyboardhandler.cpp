#include "bbvirtualkeyboardhandler.h"

#include <core/util/log.h>

#include <bps/virtualkeyboard.h>
#include <bps/navigator.h>

#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

using namespace gui::qt;

QAbstractEventDispatcher::EventFilter BBVirtualKeyboardHandler::m_prevEventFilter = 0;

BBVirtualKeyboardHandler::BBVirtualKeyboardHandler(QObject *parent) :
	QObject(parent)
{
	//m_keyboardShownCount = 0;
	virtualkeyboard_request_events(0);
	navigator_request_events(0);
	//setKeyboardVisible(true);
	m_prevEventFilter = QAbstractEventDispatcher::instance()->setEventFilter(eventFilter);
}

BBVirtualKeyboardHandler *BBVirtualKeyboardHandler::instance()
{
	// thread safe with C++11, see http://en.wikipedia.org/wiki/Double-checked_locking
	static BBVirtualKeyboardHandler i;
	return &i;
}

void BBVirtualKeyboardHandler::setKeyboardVisible(bool b)
{
	LOGDEB() << Q_FUNC_INFO << b;
	if(b)
		virtualkeyboard_show();
	else
		virtualkeyboard_hide();
}

int BBVirtualKeyboardHandler::keyboardHeight()
{
	int h = 0;
	virtualkeyboard_get_height(&h);
	return h;
}

bool BBVirtualKeyboardHandler::eventFilter(void *message)
{
	bps_event_t * const event = static_cast<bps_event_t *>(message);

	if (event && BBVirtualKeyboardHandler::instance()) {
		if (bps_event_get_domain(event) == virtualkeyboard_get_domain()) {
			const int id = bps_event_get_code(event);
			switch( id ) {
			case VIRTUALKEYBOARD_EVENT_VISIBLE: {
				LOGDEB() << "Keyboard visible";
				BBVirtualKeyboardHandler *hi = BBVirtualKeyboardHandler::instance();
				//hi->m_keyboardShownCount++;
				hi->m_keyboardVisible = true;
				emit hi->keyboardVisibleChanged(true);
				break;
			}
			case VIRTUALKEYBOARD_EVENT_HIDDEN: {
				LOGDEB() << "Keyboard hidden";
				BBVirtualKeyboardHandler *hi = BBVirtualKeyboardHandler::instance();
				hi->m_keyboardVisible = false;
				hi->keyboardVisibleChanged(false);
				break;
			}
			case VIRTUALKEYBOARD_EVENT_INFO:
				LOGDEB() << "Keyboard event";
				//instance()->resize();
				break;
			default:
				LOGDEB() << "Unexpected keyboard event:" << id;
				break;
			}
		} else if( bps_event_get_domain(event) == navigator_get_domain()) {
			const int id = bps_event_get_code(event);
			switch( id ) {
			case NAVIGATOR_SWIPE_DOWN:
			{
				LOGDEB() << "NAVIGATOR_SWIPE_DOWN not handled";
				/*
				virtualkeyboard_hide();
				QDialog dialog;
				QPushButton button;
				QTextEdit textEdit;
				button.setText("Ok");
				QVBoxLayout extensionLayout;
				extensionLayout.setMargin(0);
				extensionLayout.addWidget(&textEdit);
				extensionLayout.addWidget(&button);
				dialog.setLayout(&extensionLayout);
				QObject::connect(&button, SIGNAL(pressed()), &dialog, SLOT(accept()));
				textEdit.setText("<h2>Help</h2>\n\n"
								 "<h3>Special characters</h3>\n\n"
								 "To type special characters, touching anywhere on the display will cause a wheel to appear. The wheel enables typing the arrow keys, tab, escape, tilde and pipe characters. To select characters from the wheel, while continuing to touch, slide your finger into the region of the character you wish to type, then back to the center of the wheel. You may do this multiple times to type multiple characters without removing your finger from the screen.\n\n"
								 "<h3>Scrolling</h3>\n\n"
								 "To see history, hold two fingers on the display and slide both fingers up or down.");
				textEdit.setReadOnly(true);
				dialog.exec();
				virtualkeyboard_show();
				*/
			}
				break;
			}
		}
	}

	if (m_prevEventFilter)
		return m_prevEventFilter(message);
	else
		return false;
}
