#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct {

	size_t              size;        /* the real lenght used by the string, in characters */
	size_t              allocated;   /* the real memory space used */
	unsigned short int  errorstatus; /* every string uses an error flag */
	char                *buffer;     /* buffer used by the string */

} dstring;



/*------------------------------------------------------------------------------------------------*/


#ifndef DEFAULT_DSTRING_CAPACITY
#define DEFAULT_DSTRING_CAPACITY 255
#endif

#undef NO_ERR
#undef BUILD_FAILURE
#undef FREE_FAILURE
#undef RESIZE_FAILURE

#define NO_ERR                    0
#define BUILD_FAILURE             1
#define FREE_FAILURE              2
#define RESIZE_FAILURE            3
#define NULL_POINTER              4
#define BUFFER_FREE               5



struct internal_err_hash  {

	unsigned short int err_num;
	const char *description;

};



static const struct internal_err_hash int_err_hash[] = {
	{ BUILD_FAILURE  , "dstring_build() refused to assign memory on not null pointer" },
	{ FREE_FAILURE   , "dstring_free() cannot free unassigned memory" },
	{ RESIZE_FAILURE , "dstring_resize() cannot realloc memory -NULL-" },
	{ NULL_POINTER   , "dstring - used a NULL pointer when not expected" },
	{ BUFFER_FREE    , "dstring buffer is not allocated" },
	{ NO_ERR         , "-dstring ok-" }
};


/* FUNCTION DECLARATION */
int         dstring_build      (dstring *refp);
int         dstring_build_size (dstring *refp, size_t inits);
int         dstring_buildfrom  (dstring *refp, const char *source);
int         dstring_free       (dstring *refp);
int         dstring_resize     (dstring *refp, size_t newsize);
int         dstring_xcopy      (dstring *dest, const char *source);
int         dstring_copy       (dstring *dest, dstring *source);
int         dstring_xadd       (dstring *refp, const char *addme);
int         dstring_add        (dstring *refp, dstring *addme);
const char *dstring_error      (dstring *refp);
void        dstring_dump       (dstring *refp);




/*------------------------------------------------------------------------------------------------*/



/* Build an 'object' of type dstring
 */
int dstring_build (dstring *refp) {

	/* returns it safe */
	char *on_heap   = (char*) malloc(DEFAULT_DSTRING_CAPACITY);
	memset(on_heap,0,DEFAULT_DSTRING_CAPACITY);
	refp->buffer      = on_heap;
	refp->allocated   = DEFAULT_DSTRING_CAPACITY;
	refp->errorstatus = NO_ERR;
	refp->size        = 0;

	return refp->errorstatus;

}



/* Build but using specified size
 */
int dstring_build_size (dstring *refp, size_t inits) {

	/* returns it safe */
	char *on_heap   = (char*) malloc(inits);
	memset(on_heap,0,inits);
	refp->buffer      = on_heap;
	refp->allocated   = inits;
	refp->errorstatus = NO_ERR;
	refp->size        = 0;

	return refp->errorstatus;

}



/* Almost everybody wants to do this...
 */
int  dstring_buildfrom (dstring *refp, const char *source) {

	if(!dstring_build_size(refp,strlen(source)+1))
		dstring_xcopy(refp,source);

	return refp->errorstatus;

};



/* I know you don't want to do this every time you have to reset a string
 * Build will also reset every member in the struct
 */
int  dstring_rebuild (dstring *refp) {

	dstring_free(refp);
	if(refp->buffer==NULL && refp->errorstatus==BUFFER_FREE)
		dstring_build(refp);

	return refp->errorstatus;

};



/* Destroy 'object'
 */
int dstring_free (dstring *refp) {

	if (refp->buffer && refp->errorstatus==NO_ERR) {
		free((void*)refp->buffer);
		refp->allocated   = 0;
		refp->errorstatus = BUFFER_FREE;
		refp->size        = 0;
		refp->buffer      = NULL;
	} else
		refp->errorstatus = FREE_FAILURE;

	return refp->errorstatus;

}



/* This is the resizer
 */
int dstring_resize (dstring *refp, size_t newsize) {

	refp->buffer = (char*) realloc(refp->buffer,newsize);
	if (refp->buffer!=NULL) {
		if (newsize>refp->allocated)
			memset(refp->buffer+refp->allocated,0,newsize-refp->allocated);
		refp->allocated = newsize;
	}
	else
		refp->errorstatus = RESIZE_FAILURE;

	return refp->errorstatus;

}



/* dstring copying - please notice that in this case the modified string
 * will be dest, and we don't touch source since it is not intended
 * to be modified.
 */
int dstring_xcopy (dstring *dest,const char *source) {

	if (dest && source) {
		size_t strsz = strlen(source);
		if (dest->allocated < strsz)
			dstring_resize(dest,strsz+1);

		memset(dest->buffer,0,dest->allocated);
		memcpy(dest->buffer,source,dest->allocated-1);
		dest->size = strsz;
	} else
		dest->errorstatus = NULL_POINTER;

	return dest->errorstatus;

}



/* dstring copying - please notice that in this case the modified string
 * will be dest, and we don't touch source since it is not intended
 * to be modified.
 */
int dstring_copy (dstring *dest,dstring *source) {

	if (dest && source) {
		if (dest->allocated < source->size+1)
			dstring_resize(dest,source->size+1);

		memset(dest->buffer,0,dest->allocated);
		memcpy(dest->buffer,source->buffer,dest->allocated-1);
		dest->size = source->size;
	} else
		dest->errorstatus = NULL_POINTER;

	return dest->errorstatus;

}



/* dstring append
 */
int dstring_xadd(dstring *refp, const char *addme) {

	if (refp && addme) {
		if (refp->allocated < (refp->size+strlen(addme)))
			dstring_resize(refp,refp->size+strlen(addme)+1);
		strcpy(refp->buffer+refp->size,addme);
		refp->size = strlen(refp->buffer);
	} else
		refp->errorstatus = NULL_POINTER;

	return refp->errorstatus;

}



/* dstring append
 */
int dstring_add(dstring *refp, dstring *addme) {

	if (refp && addme) {
		if (refp->allocated<(refp->size+addme->size))
			dstring_resize(refp,refp->size+addme->size+1);
		strcpy(refp->buffer+refp->size,addme->buffer);
		refp->size = strlen(refp->buffer);
	} else
		refp->errorstatus = NULL_POINTER;

	return refp->errorstatus;

}



/* This is for growing size
 */
int dstring_grow (dstring *refp, size_t increase) {

	dstring_resize(refp,refp->size+increase);
	return refp->errorstatus;
};




/* returns a pointer to the error description
 */
const char *dstring_error(dstring *refp) {

	const struct internal_err_hash *pt = int_err_hash;

	while (pt->err_num!=NO_ERR) {
		if (refp->errorstatus == pt->err_num)
			break;
		pt++;
	}

	return pt->description;

}



/* Debugging purposes
 */
void dstring_dump(dstring *refp) {

	register unsigned int i,j;
	printf("*******BEG DSTRING DUMP\n*\n");
	printf("* String at: %p\n",(void*)refp);
	printf("* Size:      %d\n",refp?refp->size:0);
	printf("* Counted:   %d\n",refp?strlen(refp->buffer):0);
	printf("* Alloc On:  %p\n",(void*)refp?refp->buffer:0);
	printf("* Allocated: %d\n",refp?refp->allocated:0);
	printf("* Error:     %d ( %30s )\n",refp?refp->errorstatus:0,
	       refp?dstring_error(refp):"null");
	if (refp) {

		printf("* Dump:      ");
		for (i=0,j=0;i<refp->allocated;i++,j++) {
			if (j==10) {
				printf("\n*            ");
				j=0;
			}
			printf("%02X ",refp->buffer[i]);
		}
		printf("\n");

	}
	printf("*\n*******END DSTRING DUMP\n");

}

