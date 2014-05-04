#include "screenbuffer.h"

#include <QStringList>
#include <QDebug>

#define ESC_DEBUG qDebug
//#define ESC_DEBUG while(false) qDebug

using namespace core::term;

struct EscCommand
{
	QLatin1String pattern;
	int (ScreenBuffer::*handler)(int, const QStringList &);
	bool isRegexp;
};

static EscCommand esc_commands_vt100[] = {
	{QLatin1String("\x7"), &ScreenBuffer::escape_bel, false},
	{QLatin1String("\x1b""5m"), &ScreenBuffer::escape_blink, false},
	{QLatin1String("\x1b""1m"), &ScreenBuffer::escape_bold, false},
	{QLatin1String("\x1b[2J"), &ScreenBuffer::escape_clear, false},
	{QLatin1String("\xd"), &ScreenBuffer::escape_cr, false},
	{QLatin1String("^\x001b\\[([0-9]+);([0-9]+)r"), &ScreenBuffer::escape_csr, true},
	{QLatin1String("^\x001b\\[([0-9]+)D"), &ScreenBuffer::escape_cub, true},
	{QLatin1String("\x8"), &ScreenBuffer::escape_cub1, false},
	{QLatin1String("^\x001b\\[([0-9]+)B"), &ScreenBuffer::escape_cud, true},
	{QLatin1String("\xa"), &ScreenBuffer::escape_cud1, false},
	{QLatin1String("^\x001b\\[([0-9]+)C"), &ScreenBuffer::escape_cuf, true},
	{QLatin1String("\x1b[C"), &ScreenBuffer::escape_cuf1, false},
	{QLatin1String("^\x001b\\[([0-9]+);([0-9]+)H"), &ScreenBuffer::escape_cup, true},
	{QLatin1String("^\x001b\\[([0-9]+)A"), &ScreenBuffer::escape_cuu, true},
	{QLatin1String("\x1b[A"), &ScreenBuffer::escape_cuu1, false},
	{QLatin1String("\x1b[J"), &ScreenBuffer::escape_ed, false},
	{QLatin1String("\x1b[K"), &ScreenBuffer::escape_el, false},
	{QLatin1String("\x1b[1K"), &ScreenBuffer::escape_el1, false},
	{QLatin1String("\x1b(B\x1b)0"), &ScreenBuffer::escape_enacs, false},
	{QLatin1String("\x1b[H"), &ScreenBuffer::escape_home, false},
	{QLatin1String("\x9"), &ScreenBuffer::escape_ht, false},
	{QLatin1String("\x1bH"), &ScreenBuffer::escape_hts, false},
	{QLatin1String("\xa"), &ScreenBuffer::escape_ind, false},
	{QLatin1String("\x1bOq"), &ScreenBuffer::escape_ka1, false},
	{QLatin1String("\x1bOs"), &ScreenBuffer::escape_ka3, false},
	{QLatin1String("\x1bOr"), &ScreenBuffer::escape_kb2, false},
	{QLatin1String("\x8"), &ScreenBuffer::escape_kbs, false},
	{QLatin1String("\x1bOp"), &ScreenBuffer::escape_kc1, false},
	{QLatin1String("\x1bOn"), &ScreenBuffer::escape_kc3, false},
	{QLatin1String("\x1bOD"), &ScreenBuffer::escape_kcub1, false},
	{QLatin1String("\x1bOB"), &ScreenBuffer::escape_kcud1, false},
	{QLatin1String("\x1bOC"), &ScreenBuffer::escape_kcuf1, false},
	{QLatin1String("\x1bOA"), &ScreenBuffer::escape_kcuu1, false},
	{QLatin1String("\x1bOM"), &ScreenBuffer::escape_kent, false},
	{QLatin1String("\x1bOy"), &ScreenBuffer::escape_kf0, false},
	{QLatin1String("\x1bOP"), &ScreenBuffer::escape_kf1, false},
	{QLatin1String("\x1bOx"), &ScreenBuffer::escape_kf10, false},
	{QLatin1String("\x1bOQ"), &ScreenBuffer::escape_kf2, false},
	{QLatin1String("\x1bOR"), &ScreenBuffer::escape_kf3, false},
	{QLatin1String("\x1bOS"), &ScreenBuffer::escape_kf4, false},
	{QLatin1String("\x1bOt"), &ScreenBuffer::escape_kf5, false},
	{QLatin1String("\x1bOu"), &ScreenBuffer::escape_kf6, false},
	{QLatin1String("\x1bOv"), &ScreenBuffer::escape_kf7, false},
	{QLatin1String("\x1bOl"), &ScreenBuffer::escape_kf8, false},
	{QLatin1String("\x1bOw"), &ScreenBuffer::escape_kf9, false},
	// lf1
	// lf2
	// lf3
	// lf4
	{QLatin1String("\x1b[0i"), &ScreenBuffer::escape_mc0, false},
	{QLatin1String("\x1b[4i"), &ScreenBuffer::escape_mc4, false},
	{QLatin1String("\x1b[5i"), &ScreenBuffer::escape_mc5, false},
	{QLatin1String("\x1b""8"), &ScreenBuffer::escape_rc, false},
	{QLatin1String("\x1b[7m"), &ScreenBuffer::escape_rev, false},
	{QLatin1String("\x1bM"), &ScreenBuffer::escape_ri, false},
	{QLatin1String("\xf"), &ScreenBuffer::escape_rmacs, false},
	{QLatin1String("\x1b[?7l"), &ScreenBuffer::escape_rmam, false},
	{QLatin1String("\x1b[?1l\x1b>"), &ScreenBuffer::escape_rmkx, false},
	{QLatin1String("\x1b[m"), &ScreenBuffer::escape_rmso, false},
	{QLatin1String("\x1b[m"), &ScreenBuffer::escape_rmul, false},
	// rs2
	{QLatin1String("\x1b""7"), &ScreenBuffer::escape_sc, false},
	// sgr
	{QLatin1String("\x1b[0m"), &ScreenBuffer::escape_sgr0, false},
	{QLatin1String("\xe"), &ScreenBuffer::escape_smacs, false},
	{QLatin1String("\x1b[?7h"), &ScreenBuffer::escape_smam, false},
	{QLatin1String("\x1b[?1h\x1b"), &ScreenBuffer::escape_smkx, false},
	{QLatin1String("\x1b""7m"), &ScreenBuffer::escape_smso, false},
	{QLatin1String("\x1b[4m"), &ScreenBuffer::escape_smul, false},
	{QLatin1String("\x1b[3g"), &ScreenBuffer::escape_tbc, false},
};
static const int esc_commands_count_vt100 = sizeof(esc_commands_vt100)/sizeof(EscCommand);

// graphics charset pairs, based on vt100
int ScreenBuffer::escape_acsc(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// audible signal (bell)
int ScreenBuffer::escape_bel(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
//  if( term->bell != NULL ) term->bell(TO_H(term));
	*/
}

// turn on blinking
int ScreenBuffer::escape_blink(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
//  term->cattr |= TERM_ATTRIB_BLINK;
	*/
}

// turn on bold (extra bright) mode
int ScreenBuffer::escape_bold(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
//  term->cattr |= TERM_ATTRIB_BOLD;
	*/
}

// Clear screen and home cursor
int ScreenBuffer::escape_clear(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int i, j;
	for( i = 0; i < term->grid.height; i ++ ) {
		for( j = 0; j < term->grid.width; j ++ ) {
			term->grid.grid[ i ][ j ] = ' ';
			term->grid.attribs[ i ][ j ] = 0;
		}
	}
	term->crow = 0;
	term->ccol = 0;
	term_add_dirty_rect( term, 0, 0, term->grid.width, term->grid.height );
	term->dirty_cursor.exists = true;
	*/
}

// carriage return
int ScreenBuffer::escape_cr(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
//  term->ccol = 0;
	*/
}

// Change scrolling region
int ScreenBuffer::escape_csr(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// Move cursor left #1 spaces
int ScreenBuffer::escape_cub(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int left = atoi( term->output_bytes + 2 );
	if( term->ccol < left ) {
		term->ccol = 0;
	}  else {
		term->ccol -= left;
	}
	term->dirty_cursor.exists = 1;
	*/
}

// move left one space
int ScreenBuffer::escape_cub1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	if( term->ccol > 0 ) {
		term->ccol --;
		term->dirty_cursor.exists = true;
	}
	*/
}

// Move down #1 lines
int ScreenBuffer::escape_cud(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int down = atoi( term->output_bytes + 2 );
	if( term->crow + down >= term->grid.height ) {
		term->crow = term->grid.height - 1;
	}  else {
		term->crow += down;
	}
	term->dirty_cursor.exists = true;
	*/
}

// down one line
int ScreenBuffer::escape_cud1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->crow++;
	term->dirty_cursor.exists = true;
	if( term->crow >= term->grid.height ) {
		term_shiftrows_up(term);
		term_add_dirty_rect( term, 0, 0, term->grid.width, term->grid.height );
	}
	*/
}

// Move right #1 spaces
int ScreenBuffer::escape_cuf(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int right = atoi( term->output_bytes + 2 );
	if( term->ccol + right >= term->grid.width ) {
		term->ccol = term->grid.width - 1;
	}  else {
		term->ccol += right;
	}
	term->dirty_cursor.exists = true;
	*/
}

// move right one space
int ScreenBuffer::escape_cuf1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	if( term->ccol + 1 >= term->grid.width ) {
		term->ccol = term->grid.width - 1;
	}  else {
		term->ccol ++;
	}
	term->dirty_cursor.exists = true;
	*/
}

// Move to row #1 col #2
int ScreenBuffer::escape_cup(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	char *n;
	term->crow = strtoul( term->output_bytes + 2, &n, 10 ) - 1;
	if( term->crow >= term->grid.height ) term->crow = term->grid.height - 1;
	term->ccol = strtoul( n + 1, NULL, 10 ) - 1;
	if( term->ccol >= term->grid.width ) term->crow = term->grid.width - 1;
	term->dirty_cursor.exists = true;
	slog("term: cursor moved to [%d, %d]", term->crow, term->ccol);
	*/
}

// Move cursor up #1 lines
int ScreenBuffer::escape_cuu(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int up = atoi( term->output_bytes + 2 );
	if( term->crow - up < 0 ) {
		term->crow = 0;
	}  else {
		term->crow -= up;
	}
	term->dirty_cursor.exists = true;
	*/
}

// up one line
int ScreenBuffer::escape_cuu1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	if( term->crow > 0 ) {
		term->crow --;
		term->dirty_cursor.exists = true;
	}
	*/
}

// Clear to end of display
int ScreenBuffer::escape_ed(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int i, j;
	for( i = term->crow; i < term->grid.height; i ++ ) {
		for( j = term->ccol; j < term->grid.width; j ++ ) {
			term->grid.grid[ i ][ j ] = ' ';
			term->grid.attribs[ i ][ j ] = 0;
		}
	}
	term_add_dirty_rect( term, term->ccol, term->crow, term->grid.width - term->ccol, term->grid.height - term->crow );
	*/
}

// Clear to end of line
int ScreenBuffer::escape_el(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int i;
	for( i = term->ccol; i < term->grid.width; i ++ ) {
		term->grid.grid[ term->crow ][ i ] = ' ';
	}
	term_add_dirty_rect( term, term->ccol, term->crow, term->grid.width - term->ccol, 1 );
	*/
}

// Clear to beginning of line
int ScreenBuffer::escape_el1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int i;
	for( i = term->ccol; i >= 0; i -- ) {
		term->grid.grid[ term->crow ][ i ] = ' ';
	}
	term_add_dirty_rect( term, 0, term->crow, term->ccol, 1 );
	*/
}

// enable alternate char set
int ScreenBuffer::escape_enacs(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// Home cursor
int ScreenBuffer::escape_home(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->crow = 0;
	term->ccol = 0;
	term->dirty_cursor.exists = true;
	*/
}

// turn on blank mode (characters invisible)
int ScreenBuffer::escape_invis(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// tab to next 8-space hardware tab stop
int ScreenBuffer::escape_ht(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// set a tab in every row, current columns
int ScreenBuffer::escape_hts(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// scroll text up
int ScreenBuffer::escape_ind(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// upper left of keypad
int ScreenBuffer::escape_ka1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// upper right of keypad
int ScreenBuffer::escape_ka3(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// center of keypad
int ScreenBuffer::escape_kb2(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// backspace key
int ScreenBuffer::escape_kbs(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// lower left of keypad
int ScreenBuffer::escape_kc1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// lower right of keypad
int ScreenBuffer::escape_kc3(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// left-arrow key
int ScreenBuffer::escape_kcub1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// down-arrow key
int ScreenBuffer::escape_kcud1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// right-arrow key
int ScreenBuffer::escape_kcuf1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// up-arrow key
int ScreenBuffer::escape_kcuu1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// enter/send key
int ScreenBuffer::escape_kent(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F0 function key
int ScreenBuffer::escape_kf0(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F1 function key
int ScreenBuffer::escape_kf1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F10 function key
int ScreenBuffer::escape_kf10(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F2 function key
int ScreenBuffer::escape_kf2(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F3 function key
int ScreenBuffer::escape_kf3(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F4 function key
int ScreenBuffer::escape_kf4(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F5 function key
int ScreenBuffer::escape_kf5(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F6 function key
int ScreenBuffer::escape_kf6(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F7 function key
int ScreenBuffer::escape_kf7(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F8 function key
int ScreenBuffer::escape_kf8(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// F9 function key
int ScreenBuffer::escape_kf9(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// label on function key f1 if not f1
int ScreenBuffer::escape_lf1(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// label on function key f2 if not f2
int ScreenBuffer::escape_lf2(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// label on function key f3 if not f3
int ScreenBuffer::escape_lf3(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// label on function key f4 if not f4
int ScreenBuffer::escape_lf4(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// print contents of screen
int ScreenBuffer::escape_mc0(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// turn off printer
int ScreenBuffer::escape_mc4(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// turn on printer
int ScreenBuffer::escape_mc5(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

int ScreenBuffer::escape_nel(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->ccol=0;
	term->crow++;
	term->dirty_cursor.exists = true;
	if( term->crow >= term->grid.height ) {
		term_shiftrows_up(term);
		term_add_dirty_rect( term, 0, 0, term->grid.width, term->grid.height );
	}
	*/
}

// Restore cursor to position of last sc
int ScreenBuffer::escape_rc(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->crow = term->csavedrow;
	term->ccol = term->csavedcol;
	term->dirty_cursor.exists = true;
	*/
}

// turn on reverse video mode
int ScreenBuffer::escape_rev(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->cattr |= TERM_ATTRIB_REVERSE;
	*/
}

// scroll text down
int ScreenBuffer::escape_ri(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term_shiftrows_down(term);
	term_add_dirty_rect( term, 0, 0, term->grid.width, term->grid.height );
	*/
}

// end alternate character set
int ScreenBuffer::escape_rmacs(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// turn off automatic margins
int ScreenBuffer::escape_rmam(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// Out of "keypad-transmit" mode
int ScreenBuffer::escape_rmkx(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	// Do nothing here, assume always transmit since we don't have a numeric keypad on VKB
	return matched_len;
}

// exit standout mode
int ScreenBuffer::escape_rmso(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->cattr &= ~TERM_ATTRIB_STANDOUT;
	*/
}

// End underscore mode
int ScreenBuffer::escape_rmul(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->cattr &= ~TERM_ATTRIB_UNDERSCORE;
	*/
}

// reset string
int ScreenBuffer::escape_rs2(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// Save cursor position
int ScreenBuffer::escape_sc(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->csavedrow = term->crow;
	term->csavedcol = term->ccol;
	*/
}

// define video attributes #1-#9
int ScreenBuffer::escape_sgr(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// Turn off all attributes
int ScreenBuffer::escape_sgr0(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->cattr = 0;
	term->ccolour = 0;
	*/
}

// start alternate character set
int ScreenBuffer::escape_smacs(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// turn on automatic margins
int ScreenBuffer::escape_smam(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
}

// Begin keypad transmit mode.
int ScreenBuffer::escape_smkx(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	// Do nothing here, assume always transmit since we don't have a numeric keypad on VKB
	return matched_len;
}

// begin standout mode
int ScreenBuffer::escape_smso(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->cattr |= TERM_ATTRIB_STANDOUT;
	*/
}

// begin underline mode
int ScreenBuffer::escape_smul(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	term->cattr |= TERM_ATTRIB_UNDERSCORE;
	*/
}

// clear all tab stops
int ScreenBuffer::escape_tbc(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	*/
}

// vertical position #1 absolute
int ScreenBuffer::escape_vpa(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int vpos = atoi( term->output_bytes + 2 );
	if( vpos >= term->grid.height ) {
		term->crow = term->grid.height - 1;
	}  else {
		term->crow = vpos;
	}
	term->dirty_cursor.exists = true;
	*/
}

int ScreenBuffer::escape_sgm(int matched_len, const QStringList &params)
{
	Q_UNUSED(matched_len);
	Q_UNUSED(params);
	ESC_DEBUG() << Q_FUNC_INFO << "NIY";
	return matched_len;
	/*
	int i = 2;
	int val;
	char *nptr;

	while( true ) {
		val = strtoul( term->output_bytes + i, &nptr, 10 );
		switch( val ) {
			case 0:
				term->cattr = 0;
				break;
			case 1:
				term->cattr |= TERM_ATTRIB_BOLD;
				break;
			case 4:
				term->cattr |= TERM_ATTRIB_UNDERSCORE;
				break;
			case 5:
				term->cattr |= TERM_ATTRIB_BLINK;
				break;
			case 7:
				term->cattr |= TERM_ATTRIB_REVERSE;
				break;
			case 8:
				term->cattr |= TERM_ATTRIB_CONCEALED;
				break;
			case 30:
				// Black
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_BLACK << TERM_FG_SHIFT;
				break;
			case 31:
				// Red
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_RED << TERM_FG_SHIFT;
				break;
			case 32:
				// Green
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_GREEN << TERM_FG_SHIFT;
				break;
			case 33:
				// Yellow
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_YELLOW << TERM_FG_SHIFT;
				break;
			case 34:
				// Blue
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_BLUE << TERM_FG_SHIFT;
				break;
			case 35:
				// Magenta
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_MAGENTA << TERM_FG_SHIFT;
				break;
			case 36:
				// Cyan
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_CYAN << TERM_FG_SHIFT;
				break;
			case 37:
				// White
				term->ccolour &= ~TERM_FG_MASK;
				term->ccolour |= TERM_COLOR_WHITE << TERM_FG_SHIFT;
				break;
			case 40:
				// Black
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_BLACK << TERM_BG_SHIFT;
				break;
			case 41:
				// Red
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_RED << TERM_BG_SHIFT;
				break;
			case 42:
				// Green
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_GREEN << TERM_BG_SHIFT;
				break;
			case 43:
				// Yellow
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_YELLOW << TERM_BG_SHIFT;
				break;
			case 44:
				// Blue
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_BLUE << TERM_BG_SHIFT;
				break;
			case 45:
				// Magenta
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_MAGENTA << TERM_BG_SHIFT;
				break;
			case 46:
				// Cyan
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_CYAN << TERM_BG_SHIFT;
				break;
			case 47:
				// White
				term->ccolour &= ~TERM_BG_MASK;
				term->ccolour |= TERM_COLOR_WHITE << TERM_BG_SHIFT;
				break;
		}
		if( (*nptr) == 'm' ) break;
		i = nptr - term->output_bytes + 1;
	}
	*/
}

static QMap<QString, QRegExp> seqRegExps;

int ScreenBuffer::processControlSequence(int start_pos)
{
	EscCommand *cmds = esc_commands_vt100;
	int cmdn = esc_commands_count_vt100;
	QStringList params;
	int cmd_index = -1;
	int matched_length = -1;
	for(int i=0; i<cmdn; i++) {
		EscCommand cmd = cmds[i];
		QString pattern = cmd.pattern;
		if(cmd.isRegexp) {
			QRegExp rx = seqRegExps.value(pattern);
			if(!rx.isEmpty()) {
				rx = QRegExp(pattern);
				seqRegExps[pattern] = rx;
			}
			if(rx.indexIn(m_inputBuffer, start_pos, QRegExp::CaretAtOffset) == start_pos) {
				cmd_index = i;
				params = rx.capturedTexts().mid(1);
				matched_length = rx.matchedLength();
				break;
			}
		}
		else {
			QStringRef str_ref(&m_inputBuffer, start_pos, m_inputBuffer.length() - start_pos);
			if(str_ref.startsWith(pattern)) {
				cmd_index = i;
				matched_length = pattern.length();
				break;
			}
		}
	}
	if(cmd_index >= 0) {
		(*this.*(cmds[cmd_index].handler))(matched_length, params);
		return matched_length;
	}
	return 0;
}

