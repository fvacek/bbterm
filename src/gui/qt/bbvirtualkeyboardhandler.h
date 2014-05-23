#ifndef BBVIRTUALKEYBOARDHANDLER_H
#define BBVIRTUALKEYBOARDHANDLER_H

#include <QObject>
#include <QAbstractEventDispatcher>

namespace gui {
namespace qt {

class BBVirtualKeyboardHandler : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool keyboardVisible READ isKeyboardVisible WRITE setKeyboardVisible NOTIFY keyboardVisibleChanged)
private:
	explicit BBVirtualKeyboardHandler(QObject *parent = 0);
public:
	static BBVirtualKeyboardHandler* instance();

	bool isKeyboardVisible() const {return m_keyboardVisible;}
	Q_SLOT void setKeyboardVisible(bool b);
	Q_SIGNAL void keyboardVisibleChanged(bool visible);
	Q_SIGNAL void keyboardLayoutChanged();

	Q_SIGNAL void windowActiveChanged(bool window_active);

	int keyboardHeight();
private:
	bool m_keyboardVisible;
	//int m_keyboardShownCount;
	static bool eventFilter(void *message);
	static QAbstractEventDispatcher::EventFilter m_prevEventFilter;
};

}
}

#endif // BBVIRTUALKEYBOARDHANDLER_H
