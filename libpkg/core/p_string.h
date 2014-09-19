/*-
 * Copyright (c) 2014 Baptiste Daroussin <bapt@FreeBSD.org>
 * All rights reserved.
 *~
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *~
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _P_STRING_H
#define _P_STRING_H

struct p_string {
	char *buf;
	size_t len;
	size_t cap;
	size_t step;
};

struct p_string *p_string_new(size_t sz);
void p_string_reset(struct p_string *p);
void p_string_free(struct p_string *p);

int p_string_append(struct p_string *p, char *str, size_t len);
char *p_string_data(struct p_string *p);
void p_string_trim(struct p_string *p);
void p_string_rtrim(struct p_string *p);
void p_string_ltrim(struct p_string *p);
int p_string_printf(struct p_string *p, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int p_string_vprintf(struct p_string *p, const char *fmt, va_list ap) __attribute__((format(printf, 2, 0)));
size_t p_string_len(struct p_string *p);

#endif
