/******************************************************************************
 *                AtomWeb: An embedded web server                             *
 *                                                                            *
 * Copyright (c) 2015 Gong Ke                                                 *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 * 1.Redistributions of source code must retain the above copyright notice,   *
 * this list of conditions and the following disclaimer.                      *
 * 2.Redistributions in binary form must reproduce the above copyright        *
 * notice, this list of conditions and the following disclaimer in the        *
 * documentation and/or other materials provided with the distribution.       *
 * 3.Neither the name of the Gong Ke; nor the names of its contributors may   *
 * be used to endorse or promote products derived from this software without  *
 * specific prior written permission.                                         *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR           *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *****************************************************************************/

#ifndef _AW_TOOLS_H_
#define _AW_TOOLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <libgen.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#ifdef AW_DEBUG
#define MEMWATCH
#define MEMWATCH_STDIO
#include <memwatch.h>
#endif

#define AW_MACRO_BEGIN do{
#define AW_MACRO_END   }while(0)

#ifdef AW_DEBUG
	#define AW_ASSERT(a) assert(a)
	#define AW_INFO(a)\
		AW_MACRO_BEGIN\
			printf("I/\"%s\" %d: ", __FILE__, __LINE__);\
			printf a;\
			printf("\n");\
		AW_MACRO_END
	#define AW_ERROR(a)\
		AW_MACRO_BEGIN\
			printf("E/\"%s\" %d: ", __FILE__, __LINE__);\
			printf a;\
			printf("\n");\
		AW_MACRO_END
	#define AW_WARNING(a)\
		AW_MACRO_BEGIN\
			printf("W/\"%s\" %d: ", __FILE__, __LINE__);\
			printf a;\
			printf("\n");\
		AW_MACRO_END
	#define AW_FATAL(a)\
		AW_MACRO_BEGIN\
			printf("F/\"%s\" %d: ", __FILE__, __LINE__);\
			printf a;\
			printf("\n");\
			abort();\
		AW_MACRO_END
#else
	#define AW_ASSERT(a)
	#define AW_INFO(a)
	#define AW_ERROR(a)
	#define AW_WARNING(a)
	#define AW_FATAL(a)  abort()
#endif

extern void  aw_str_concat (char **ptr, char *add);
extern void  aw_str_printf (char **ptr, const char *fmt, ...);
extern char* aw_str_cname (const char *lname, char *cbuf, int size);
extern const char* aw_mime_get_type (const char *suffix);

#ifdef __cplusplus
}
#endif

#endif

