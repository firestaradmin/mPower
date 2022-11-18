#ifndef __UG_DRAW_TEXT_H__
#define __UG_DRAW_TEXT_H__
#include "ug_user_config.h"
#include "ug_type.h"
#include "ug_area.h"
#include "ug_color.h"
#include "ug_font.h"

void ug_draw_str(const ug_area_t *coords, const ug_font_t *font_p, ug_color_t color, ug_color_t backColor, const unsigned char *str, const ug_area_t *clip);

#endif //!__UG_DRAW_TEXT_H__




