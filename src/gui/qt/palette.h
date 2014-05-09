#ifndef GUI_QT_PALETTE_H
#define GUI_QT_PALETTE_H

#include <QColor>

namespace gui {
namespace qt {

class Palette 
{
public:
	Palette();
	virtual ~Palette();

	QColor getColor(int index, bool highlight);
private:
	QColor m_colors[16];
};

}
}

#endif /* GUI_QT_PALETTE_H */
