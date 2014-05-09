#include "palette.h"

using namespace gui::qt;

Palette::Palette() 
{
	m_colors[0]  = QColor(0,   0,   0); // Black
	m_colors[1]  = QColor(205, 0,   0); // Red
	m_colors[2]  = QColor(0, 205,   0); // Green
	m_colors[3]  = QColor(205, 205, 0); // Brown/Yellow
	m_colors[4]  = QColor(0,   0, 238); // Blue
	m_colors[5]  = QColor(205, 0, 205); // Magenta
	m_colors[6]  = QColor(0, 205, 205); // Cyan
	m_colors[7]  = QColor(229, 229, 229); // Gray
	m_colors[8]  = QColor(127, 127, 127); // Dark Gray
	m_colors[9]  = QColor(255, 0,   0);   // Bright red
	m_colors[10] = QColor(0, 255,   0);   // Bright green
	m_colors[11] = QColor(255, 255, 0);   // Bright yellow
	m_colors[12] = QColor(92, 92, 255);   // Bright blue
	m_colors[13] = QColor(255, 0, 255);   // Bright magenta
	m_colors[14] = QColor(0, 255, 255);   // Bright cyan
	m_colors[15] = QColor(255, 255, 255); // White
}

Palette::~Palette() 
{
	// TODO Auto-generated destructor stub
}

QColor Palette::getColor(int index, bool highlight) 
{
	if (index < 8) {
		return m_colors[highlight ? index+8 : index];
	}
	return QColor();
}
