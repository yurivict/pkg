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
#include <string.h>

#include "p_string.h"

#define nullterm(p) do {               \
	(p)->buf[(p)->len] = '\0';     \
} while (0)

static int
p_string_grow(struct p_string *p)
{
	p->cap += p->step;
	p->buf = reallocf(p->buf, p->cap * sizeof(char));
	if (p->buf == NULL)
		return (0);

	return (1);
}

struct p_string *
p_string_new(size_t sz)
{
	struct p_string *p;

	p = calloc(1, sizeof(struct p_string));
	p->step = sz ? sz : BUFSIZ;

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
	if (str == NULL)
		return (0);

	if (len == 0)
		len = strlen(str);

	while (p->len + len >= p->cap)
		if (!p_string_grow(p))
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

	while (len >= p->cap)
		if (!p_string_grow(p))
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
