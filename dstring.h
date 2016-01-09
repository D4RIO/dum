#ifndef _D_STRINGMANAGING_H_
#define _D_STRINGMANAGING_H_


/**
 * DSTRING - A powerful and fast dynamic-memory-based
 *  string handling library. This allows using of fast
 *  methodologies and simple interfaces, so you can
 *  just enjoy string handling easily.
 */


typedef struct {

	size_t              size;        /* lenght of the string, in characters */
	size_t              allocated;   /* the actual memory space used */
	unsigned short int  errorstatus; /* every string uses an error flag */
	char                *buffer;     /* buffer used by the string */

} dstring;


/**
 * To allocate space for a new string, you must pass an INITIALIZED pointer,
 * since the allocation is for the buffer pointer, no for the object itself.
 *
 * You will have to use dstring_error(dstring*) to obtain the error message
 * string, and dstring_error_ds(dstring*) to obtain a dstring object.
 *
 * Strings are safely treated, using '\0' byte as ending and taking care on
 * it for you, see the documentation 
 */
extern int         dstring_build      (dstring *refp);
extern int         dstring_build_size (dstring *refp, size_t inits);
extern int         dstring_buildfrom  (dstring *refp, const char *source);
extern int         dstring_rebuild    (dstring *refp);
extern int         dstring_free       (dstring *refp);
extern int         dstring_resize     (dstring *refp, size_t newsize);
extern int         dstring_copy       (dstring *dest, dstring *source);
extern int         dstring_xcopy      (dstring *dest, const char *source);
extern int         dstring_xadd       (dstring *refp, const char *addme);
extern int         dstring_add        (dstring *refp, dstring *addme);
extern const char *dstring_error      (dstring *refp);
extern void        dstring_dump       (dstring *refp);
extern int         dstring_grow       (dstring *refp, size_t increase);


#endif
