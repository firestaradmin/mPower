/**
 * @file ug_label.h
 *
 */

#ifndef UG_LABEL_H
#define UG_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "ug_user_config.h"
#if UG_USE_LABEL != 0

#include <stdarg.h>
#include "ug_obj.h"
#include "ug_font.h"

/** Label align policy*/
enum {
    UG_LABEL_ALIGN_LEFT, /**< Align text to left */
    UG_LABEL_ALIGN_CENTER, /**< Align text to center */
    UG_LABEL_ALIGN_RIGHT, /**< Align text to right */
    UG_LABEL_ALIGN_AUTO, /**< Use LEFT or RIGHT depending on the direction of the text (LTR/RTL)*/
};
typedef uint8_t ug_label_align_t;


/** Data of label*/
typedef struct ug_label_ext{
    /*Inherited from 'base_obj' so no inherited ext.*/ /*Ext. of ancestor*/

    /*New data for this type */
    char * text;        /*Text of the label*/

    ug_point_t offset; /*Text draw position offset*/

    uint8_t                 static_txt : 1; /*Flag to indicate the text is static*/
    ug_label_align_t        align : 2;      /*Align type from 'ug_label_align_t'*/

    
    ug_color_t color;
    ug_font_t *font;

} ug_label_ext_t;



/**
* Create a label objects
* @param par pointer to an object, it will be the parent of the new label
* @param copy pointer to a label object, if not NULL then the new object will be copied from it
* @return pointer to the created button
*/
ug_obj_t *ug_label_create(ug_obj_t *par, const ug_obj_t *copy);

void ug_label_set_font(ug_obj_t *label, ug_font_t *font);
void ug_label_set_color(ug_obj_t *label, ug_color_t color);

/**
* Set a new text for a label. Memory will be allocated to store the text by the label.
* @param label pointer to a label object
* @param text '\0' terminated character string. NULL to refresh with the current text.
*/
void ug_label_set_text(ug_obj_t *label, const char *text);

/**
* Set a static text. It will not be saved by the label so the 'text' variable
* has to be 'alive' while the label exist.
* @param label pointer to a label object
* @param text pointer to a text. NULL to refresh with the current text.
*/
void ug_label_set_text_static(ug_obj_t *label, const char *text);

/**
* Get the text of a label
* @param label pointer to a label object
* @return the text of the label
*/
char *ug_label_get_text(const ug_obj_t *label);


/**
* Set the align of the label (left or center)
* @param label pointer to a label object
* @param align 'ug_LABEL_ALIGN_LEFT' or 'ug_LABEL_ALIGN_LEFT'
*/
void ug_label_set_align(ug_obj_t *label, ug_label_align_t align);


/**
* Get the align attribute
* @param label pointer to a label object
* @return ug_LABEL_ALIGN_LEFT or ug_LABEL_ALIGN_CENTER
*/
ug_label_align_t ug_label_get_align(const ug_obj_t *label);

/**
* Refresh the label with its text stored in its extended data
* @param label pointer to a label object
*/
static void ug_label_refr_text(ug_obj_t *label);




#if 0
/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/** Long mode behaviors. Used in 'ug_label_ext_t' 
 * Text showing behaviors.
*/
enum {
    UG_LABEL_LONG_EXPAND,    /**< Expand the object size to the text size*/
    UG_LABEL_LONG_BREAK,     /**< Keep the object width, break the too long lines and expand the object height*/
    UG_LABEL_LONG_DOT,       /**< Keep the size and write dots at the end if the text is too long*/
    UG_LABEL_LONG_SROLL,      /**< Keep the size and roll the text back and forth*/
    UG_LABEL_LONG_SROLL_CIRC, /**< Keep the size and roll the text circularly*/
    UG_LABEL_LONG_CROP,      /**< Keep the size and crop the text out of it*/
};
typedef uint8_t ug_label_long_mode_t;

/** Label align policy*/
enum {
    UG_LABEL_ALIGN_LEFT, /**< Align text to left */
    UG_LABEL_ALIGN_CENTER, /**< Align text to center */
    UG_LABEL_ALIGN_RIGHT, /**< Align text to right */
    UG_LABEL_ALIGN_AUTO, /**< Use LEFT or RIGHT depending on the direction of the text (LTR/RTL)*/
};
typedef uint8_t ug_label_align_t;

/** Data of label*/
typedef struct {
    /*Inherited from 'base_obj' so no inherited ext.*/ /*Ext. of ancestor*/

    /*New data for this type */
    char * text;        /*Text of the label*/

    uint32_t dot_end;  /*The text end position in dot mode (Handled by the library)*/

    ug_point_t offset; /*Text draw position offset*/

    ug_label_long_mode_t    long_mode : 3;  /*Determinate what to do with the long texts*/
    uint8_t                 static_txt : 1; /*Flag to indicate the text is static*/
    ug_label_align_t        align : 2;      /*Align type from 'ug_label_align_t'*/
    uint8_t                 recolor : 1;    /*Enable in-line letter re-coloring*/
    uint8_t                 expand : 1;     /*Ignore real width (used by the library with UG_LABEL_LONG_SROLL)*/

} ug_label_ext_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a label objects
 * @param par pointer to an object, it will be the parent of the new label
 * @param copy pointer to a button object, if not NULL then the new object will be copied from it
 * @return pointer to the created button
 */
ug_obj_t * ug_label_create(ug_obj_t * par, const ug_obj_t * copy);

/*=====================
 * Setter functions
 *====================*/
/**
 * Set a new text for a label. Memory will be allocated to store the text by the label.
 * @param label pointer to a label object
 * @param text '\0' terminated character string. NULL to refresh with the current text.
 */
void ug_label_set_text(ug_obj_t * label, const char * text);

/**
 * Set a static text. It will not be saved by the label so the 'text' variable
 * has to be 'alive' while the label exist.
 * @param label pointer to a label object
 * @param text pointer to a text. NULL to refresh with the current text.
 */
void ug_label_set_text_static(ug_obj_t * label, const char * text);

/**
 * Set the behavior of the label with longer text then the object size
 * @param label pointer to a label object
 * @param long_mode the new mode from 'ug_label_long_mode' enum.
 *                  In ug_LONG_BREAK/LONG/ROLL the size of the label should be set AFTER this
 * function
 */
void ug_label_set_long_mode(ug_obj_t * label, ug_label_long_mode_t long_mode);

/**
 * Set the align of the label (left or center)
 * @param label pointer to a label object
 * @param align 'ug_LABEL_ALIGN_LEFT' or 'ug_LABEL_ALIGN_LEFT'
 */
void ug_label_set_align(ug_obj_t * label, ug_label_align_t align);

/**
 * Enable the recoloring by in-line commands
 * @param label pointer to a label object
 * @param en true: enable recoloring, false: disable
 */
void ug_label_set_recolor(ug_obj_t * label, bool en);
/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of a label
 * @param label pointer to a label object
 * @return the text of the label
 */
char * ug_label_get_text(const ug_obj_t * label);

/**
 * Get the long mode of a label
 * @param label pointer to a label object
 * @return the long mode
 */
ug_label_long_mode_t ug_label_get_long_mode(const ug_obj_t * label);

/**
 * Get the align attribute
 * @param label pointer to a label object
 * @return ug_LABEL_ALIGN_LEFT or ug_LABEL_ALIGN_CENTER
 */
ug_label_align_t ug_label_get_align(const ug_obj_t * label);

/**
 * Get the recoloring attribute
 * @param label pointer to a label object
 * @return true: recoloring is enabled, false: disable
 */
bool ug_label_get_recolor(const ug_obj_t * label);

#endif
/*=====================
 * Other functions
 *====================*/



/**********************
 *      MACROS
 **********************/

#endif /*UG_USE_LABEL*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UG_LABEL_H*/
