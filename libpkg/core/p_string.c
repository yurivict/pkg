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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "p_string.h"

#define nullterm(p) do {               \
	(p)->buf[(p)->len] = '\0';     \
} while (0)

#define p_roundup2(x,y) (((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#define STRSIZE 4096
#define STEPS 64

static bool
p_string_grow(struct p_string *p, size_t size)
{
	size_t newsize;

	if (size < STRSIZE) {
		newsize = STEPS;
		while (newsize < size)
			newsize *=2;
	} else {
		newsize = p_roundup2(size, STRSIZE);
	}

	p->cap = newsize;
	p->buf = reallocf(p->buf, p->cap * sizeof(char));
	if (p->buf == NULL)
		return (false);

	return (true);
}

struct p_string *
p_string_new(void)
{
	struct p_string *p;

	p = calloc(1, sizeof(struct p_string));

	return (p);
}

void
p_string_reset(struct p_string *p)
{
	p->len = 0;
	nullterm(p);
}

void
p_string_free(struct p_string *p)
{
	free(p->buf);
	free(p);
}

int
p_string_append(struct p_string *p, const char *str, size_t len)
{
	size_t size;

	if (str == NULL)
		return (0);

	if (len == 0)
		len = strlen(str);

	size = p->len + len + 1;
	while (size >= p->cap)
		if (!p_string_grow(p, size))
			return (0);

	memcpy(p->buf + p->len, str, len);
	p->len += len;
	nullterm(p);

	return (1);
}

int
p_string_copy(struct p_string *p, const char *str, size_t len)
{
	if (str == NULL)
		return (0);

	if (len == 0)
		len = strlen(str);

	p_string_reset(p);

	while (len + 1 >= p->cap)
		if (!p_string_grow(p, len + 1))
			return (0);

	memcpy(p->buf, str, len);
	p->len = len;
	nullterm(p);

	return (1);
}

char *
p_string_data(struct p_string *p)
{

	if (p->len == 0)
		return (NULL);

	return (p->buf);
}

void
p_string_trim(struct p_string *p)
{
	char *c = p->buf;

	while (p->len > 0 && isspace(c[p->len -1]))
		p->len--;
	while (p->len > 0 && isspace(c[0])) {
		c++;
		p->len--;
	}

	memmove(p->buf, c, p->len);
	nullterm(p);
}

void
p_string_rtrim(struct p_string *p)
{
	char *c = p->buf;

	while (p->len > 0 && isspace(c[p->len -1]))
		p->len--;

	nullterm(p);
}

void
p_string_ltrim(struct p_string *p)
{
	char *c = p->buf;

	while (p->len > 0 && isspace(c[0])) {
		c++;
		p->len--;
	}

	memmove(p->buf, c, p->len);
	nullterm(p);
}

int
p_string_printf(struct p_string *p, const char *fmt, ...)
{
	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = p_string_vprintf(p, fmt, ap);
	va_end(ap);

	return (rc);
}

int
p_string_vprintf(struct p_string *p, const char *fmt, va_list ap)
{
	size_t len;
	char *str;
	int rc;

	len = vasprintf(&str, fmt, ap);

	if (len <= 0 || str == NULL)
		return (0);

	rc = p_string_append(p, str, len);
	free(str);

	return (rc);
}

size_t
p_string_len(struct p_string *p)
{

	return (p->len);
}
