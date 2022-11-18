#include "ug_label.h"
#include "ug_obj.h"
#include "ug_draw_elements.h"
#include "ug_draw_text.h"
#if UG_USE_LABEL != 0
/*********************
*      DEFINES
*********************/
#define UG_OBJX_NAME "ug_label"
#include "ug_font.h"
#include "ug_draw_rect.h"
#include "ug_font_declare.h"

static void ug_label_refr_text(ug_obj_t *label);
static ug_design_res_t ug_label_design(ug_obj_t *obj, const ug_area_t *clip_area, ug_design_mode_t mode);
static ug_res_t ug_label_signal(ug_obj_t *obj, ug_signal_t sign, void *param);

/**
 * Handle the drawing related tasks of the base objects.
 * @param obj pointer to an object
 * @param clip_area the object will be drawn only in this area
 * @param mode UG_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             UG_DESIGN_DRAW: draw the object (always return 'true')
 * @param return an element of `ug_design_res_t`
 */
static ug_design_res_t ug_label_design(ug_obj_t * obj, const ug_area_t * clip_area, ug_design_mode_t mode)
{
    if(mode == UG_DESIGN_COVER_CHK) {
        ug_area_t coords;
        ug_area_copy(&coords, &obj->coords);
        if(_ug_area_is_in(clip_area, &coords, 0) == false) return UG_DESIGN_RES_NOT_COVER;
        return  UG_DESIGN_RES_COVER;

    }
    else if(mode == UG_DESIGN_DRAW_MAIN) {
        ug_label_ext_t * ext = ug_obj_get_ext_attr(obj);
        ug_area_t coords;
        ug_area_copy(&coords, &obj->coords);
        ug_draw_rect_dsc_t draw_dsc;
        ug_draw_rect_dsc_init(&draw_dsc);
        draw_dsc.bg_color = obj->bg_color;
        draw_dsc.drawbackground = obj->drawbackground;
        draw_dsc.border_color.full = obj->bd_color.full;
        draw_dsc.border_width = obj->bd_width;
        ug_draw_rect(&coords, clip_area, &draw_dsc);


        ug_draw_str(&coords, ext->font, ext->color, obj->bg_color, (uint8_t *) ext->text, clip_area);
    }

    return UG_DESIGN_RES_OK;
}




/**
 * Signal function of the basic object
 * @param obj pointer to an object
 * @param sign signal type
 * @param param parameter for the signal (depends on signal type)
 * @return UG_RES_OK: the object is not deleted in the function; UG_RES_INV: the object is deleted
 */
static ug_res_t ug_label_signal(ug_obj_t * obj, ug_signal_t sign, void * param)
{

    ug_res_t res = UG_RES_OK;
    ug_label_ext_t * ext = ug_obj_get_ext_attr(obj);

    if(sign == UG_SIGNAL_CHILD_CHG) {
        /*Return 'invalid' if the child change signal is not enabled*/
        if(ug_obj_is_protected(obj, UG_PROTECT_CHILD_CHG) != false) res = UG_RES_INV;
    }
    else if(sign == UG_SIGNAL_CLEANUP) {
        if(ext->text != NULL && ext->static_txt == 0) {
            ug_mem_free(ext->text);
            ext->text = NULL;
        }
    }
#if UG_USE_OBJ_REALIGN
    else if(sign == UG_SIGNAL_PARENT_SIZE_CHG) {
        if(obj->realign.auto_realign) {
            ug_obj_realign(obj);
        }
    }
#endif

    return res;
}


/**
* Create a label objects
* @param par pointer to an object, it will be the parent of the new label
* @param copy pointer to a label object, if not NULL then the new object will be copied from it
* @return pointer to the created button
*/
ug_obj_t * ug_label_create(ug_obj_t * par, const ug_obj_t * copy)
    {
    /*Create a basic object*/
    ug_obj_t * new_label = ug_obj_create(par, copy, UG_OBJX_NAME);

    if(new_label == NULL) return NULL;

    /*Extend the basic object to a label object*/
    ug_obj_allocate_ext_attr(new_label, sizeof(ug_label_ext_t));

    ug_label_ext_t * ext = ug_obj_get_ext_attr(new_label);

    if(ext == NULL) {
        ug_obj_del(new_label);
        return NULL;
    }

    new_label->drawbackground = 0;
    new_label->bg_color.full = par->bg_color.full;

    ext->text       = NULL;
    ext->static_txt = 0;

    ext->offset.x = 0;
    ext->offset.y = 0;

    ext->color.full = UG_OBJ_DEFAULT_FR_COLOR;
    ext->font = &ug_font_consolas_16px_15;

    ug_obj_set_design_cb(new_label, ug_label_design);
    ug_obj_set_signal_cb(new_label, ug_label_signal);

    /*Init the new label*/
    if(copy == NULL) {
        ug_label_set_text(new_label, "Text");
    }
    /*Copy 'copy' if not NULL*/
    else {
        ug_label_ext_t * copy_ext = ug_obj_get_ext_attr(copy);
        ug_label_set_align(new_label, ug_label_get_align(copy));
        if(copy_ext->static_txt == 0)
            ug_label_set_text(new_label, ug_label_get_text(copy));
        else
            ug_label_set_text_static(new_label, ug_label_get_text(copy));
    }
    return new_label;
}





/**
* Set a new text for a label. Memory will be allocated to store the text by the label.
* @param label pointer to a label object
* @param text '\0' terminated character string. NULL to refresh with the current text.
*/
void ug_label_set_text(ug_obj_t * label, const char * text)
{
    ug_obj_invalidate(label);

    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);

    /*If text is NULL then refresh */
    if(text == NULL) {
        ug_label_refr_text(label);
        return;
    }

    if(ext->text == text && ext->static_txt == 0) {
        /*If set its own text then reallocate it (maybe its size changed)*/
        ext->text = ug_mem_realloc(ext->text, strlen(ext->text) + 1);
        if(ext->text == NULL) return;
    }
    else {
        /*Free the old text*/
        if(ext->text != NULL && ext->static_txt == 0) {
            ug_mem_free(ext->text);
            ext->text = NULL;
        }

        /*Get the size of the text*/
        size_t len = strlen(text) + 1;

        /*Allocate space for the new text*/
        ext->text = ug_mem_alloc(len);
        if(ext->text == NULL) return;

        strcpy(ext->text, text);

        /*Now the text is dynamically allocated*/
        ext->static_txt = 0;
    }

    ug_label_refr_text(label);
}


void ug_label_set_font(ug_obj_t * label, ug_font_t * font)
{
    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);
    ext->font = font;

    ug_label_refr_text(label);
}


void ug_label_set_color(ug_obj_t * label, ug_color_t color)
{
    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);
    ext->color.full = color.full;

    ug_label_refr_text(label);
}



/**
* Set a static text. It will not be saved by the label so the 'text' variable
* has to be 'alive' while the label exist.
* @param label pointer to a label object
* @param text pointer to a text. NULL to refresh with the current text.
*/
void ug_label_set_text_static(ug_obj_t * label, const char * text)
{
    // ug_ASSERT_OBJ(label, UG_OBJX_NAME);

    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);
    if(ext->static_txt == 0 && ext->text != NULL) {
        ug_mem_free(ext->text);
        ext->text = NULL;
    }

    if(text != NULL) {
        ext->static_txt = 1;
        ext->text       = (char *)text;
    }

    ug_label_refr_text(label);
}

/**
* Get the text of a label
* @param label pointer to a label object
* @return the text of the label
*/
char * ug_label_get_text(const ug_obj_t * label)
{
//    ug_ASSERT_OBJ(label, ug_OBJX_NAME);

    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);

    return ext->text;
}


/**
* Set the align of the label (left or center)
* @param label pointer to a label object
* @param align 'ug_LABEL_ALIGN_LEFT' or 'ug_LABEL_ALIGN_LEFT'
*/
void ug_label_set_align(ug_obj_t * label, ug_label_align_t align)
{
//    ug_ASSERT_OBJ(label, UG_OBJX_NAME);

   ug_label_ext_t * ext = ug_obj_get_ext_attr(label);
   if(ext->align == align) return;

   ext->align = align;

   ug_obj_invalidate(label); /*Enough to invalidate because alignment is only drawing related
                                (ug_refr_label_text() not required)*/
}

/**
* Get the align attribute
* @param label pointer to a label object
* @return ug_LABEL_ALIGN_LEFT or ug_LABEL_ALIGN_CENTER
*/
ug_label_align_t ug_label_get_align(const ug_obj_t * label)
{
    // ug_ASSERT_OBJ(label, ug_OBJX_NAME);

    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);

    ug_label_align_t align = ext->align;

    if(align == UG_LABEL_ALIGN_AUTO) {
    #if UG_USE_BIDI
        ug_bidi_dir_t base_dir = ug_obj_get_base_dir(label);
        if(base_dir == ug_BIDI_DIR_AUTO) base_dir = _ug_bidi_detect_base_dir(ext->text);

        if(base_dir == ug_BIDI_DIR_LTR) align = ug_LABEL_ALIGN_LEFT;
        else if(base_dir == ug_BIDI_DIR_RTL) align = ug_LABEL_ALIGN_RIGHT;
    #else
        align = UG_LABEL_ALIGN_LEFT;
    #endif
    }

    return align;
}


/**
* Refresh the label with its text stored in its extended data
* @param label pointer to a label object
*/
static void ug_label_refr_text(ug_obj_t * label)
{
    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);

    if(ext->text == NULL) return;

    ug_obj_invalidate(label);
    // if(label->drawbackground == 0){
    //     ug_obj_t *par = ug_obj_get_parent(label);
    //     // ug_signal_send(par, UG_SIGNAL_CHILD_CHG, label);
    //     ug_obj_invalidate_area(par, &label->coords);
    // }
}




#if 0



/*=====================
* Setter functions
*====================*/

/**
* Set a new formatted text for a label. Memory will be allocated to store the text by the label.
* @param label pointer to a label object
* @param fmt `printf`-like format
*/
void ug_label_set_text_fmt(ug_obj_t * label, const char * fmt, ...)
{
   ug_obj_invalidate(label);

   ug_label_ext_t * ext = ug_obj_get_ext_attr(label);

   /*If text is NULL then refresh */
   if(fmt == NULL) {
       ug_label_refr_text(label);
       return;
   }

   if(ext->text != NULL && ext->static_txt == 0) {
       ug_mem_free(ext->text);
       ext->text = NULL;
   }

   va_list ap, ap2;
   va_start(ap, fmt);
   va_copy(ap2, ap);

   /*Allocate space for the new text by using trick from C99 standard section 7.19.6.12 */
   uint32_t len = ug_vsnprintf(NULL, 0, fmt, ap);
   va_end(ap);


   ext->text = ug_mem_alloc(len + 1);

   if(ext->text == NULL) {
       va_end(ap2);
       return;
   }
   ext->text[len - 1] = 0; /* Ensure NULL termination */

   ug_vsnprintf(ext->text, len + 1, fmt, ap2);


   va_end(ap2);
   ext->static_txt = 0; /*Now the text is dynamically allocated*/

   ug_label_refr_text(label);
}

/**
* Set the behavior of the label with longer text then the object size
* @param label pointer to a label object
* @param long_mode the new mode from 'ug_label_long_mode' enum.
*                  In ug_LONG_BREAK/LONG/ROLL the size of the label should be set AFTER this
* function
*/
void ug_label_set_long_mode(ug_obj_t * label, ug_label_long_mode_t long_mode)
{
   ug_ASSERT_OBJ(label, ug_OBJX_NAME);

   ug_label_ext_t * ext = ug_obj_get_ext_attr(label);

#if ug_USE_ANIMATION
   /*Delete the old animation (if exists)*/
   ug_anim_del(label, (ug_anim_exec_xcb_t)ug_obj_set_x);
   ug_anim_del(label, (ug_anim_exec_xcb_t)ug_obj_set_y);
   ug_anim_del(label, (ug_anim_exec_xcb_t)ug_label_set_offset_x);
   ug_anim_del(label, (ug_anim_exec_xcb_t)ug_label_set_offset_y);
#endif
   ext->offset.x = 0;
   ext->offset.y = 0;

   if(long_mode == ug_LABEL_LONG_SROLL || long_mode == ug_LABEL_LONG_SROLL_CIRC || long_mode == ug_LABEL_LONG_CROP)
       ext->expand = 1;
   else
       ext->expand = 0;

   /*Restore the character under the dots*/
   if(ext->long_mode == ug_LABEL_LONG_DOT && ext->dot_end != ug_LABEL_DOT_END_INV) {
       ug_label_revert_dots(label);
   }

   ext->long_mode = long_mode;
   ug_label_refr_text(label);
}



/*=====================
* Getter functions
*====================*/




/**
* Enable the recoloring by in-line commands
* @param label pointer to a label object
* @param en true: enable recoloring, false: disable
*/
void ug_label_set_recolor(ug_obj_t * label, bool en)
{
    // ug_ASSERT_OBJ(label, UG_OBJX_NAME);

    ug_label_ext_t * ext = ug_obj_get_ext_attr(label);
    if(ext->recolor == en) return;

    ext->recolor = en == false ? 0 : 1;

    ug_label_refr_text(label); /*Refresh the text because the potential color codes in text needs to
                                    be hided or revealed*/
}


/**
* Get the long mode of a label
* @param label pointer to a label object
* @return the long mode
*/
ug_label_long_mode_t ug_label_get_long_mode(const ug_obj_t * label)
{
   ug_ASSERT_OBJ(label, ug_OBJX_NAME);

   ug_label_ext_t * ext = ug_obj_get_ext_attr(label);
   return ext->long_mode;
}



/**
* Get the recoloring attribute
* @param label pointer to a label object
* @return true: recoloring is enabled, false: disable
*/
bool ug_label_get_recolor(const ug_obj_t * label)
{
   ug_ASSERT_OBJ(label, ug_OBJX_NAME);

   ug_label_ext_t * ext = ug_obj_get_ext_attr(label);
   return ext->recolor == 0 ? false : true;
}






#endif















#endif