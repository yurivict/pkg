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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "p_array.h"

#define p_roundup2(x,y) (((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#define STRSIZE 4096
#define STEPS 64

static bool
p_array_grow(struct p_array *a, size_t size)
{
	size_t newsize;

	if (size < STRSIZE) {
		newsize = STEPS;
		while (newsize < size)
			newsize *=2;
	} else {
		newsize = p_roundup2(size, STRSIZE);
	}

	a->cap = newsize;
	a->data = reallocf(a->data, a->cap * sizeof(void *));
	if (a->data == NULL)
		return (false);

	return (true);
}

struct p_array *
p_array_new(void) {
	struct p_array *a;

	a = calloc(1, sizeof(struct p_array));

	return (a);
}

void
p_array_free(struct p_array *a)
{
	free(a->data);
	free(a);
}

void
p_array_flush(struct p_array *a)
{
	a->len = 0;
	a->cap = 0;

	free(a->data);
	a->data = NULL;
}

void
p_array_reset(struct p_array *a)
{
	a->len = 0;
}

int
p_array_push(struct p_array *a, void *data)
{

	if (a->len + 1 > a->cap)
		if (!p_array_grow(a, a->len + 1))
			return (0);

	a->data[a->len++] = data;

	return (1);
}

void *
p_array_pop(struct p_array *a)
{
	if (a->len == 0)
		return (NULL);

	return (a->data[--(a->len)]);
}

void *
p_array_get(struct p_array *a, unsigned int idx)
{
	if (idx >= a->len)
		return (NULL);

	return (a->data[idx]);
}

size_t
p_array_len(struct p_array *a)
{
	return (a->len);
}

int
p_array_foreach(struct p_array *a, int (*cb)(void *, unsigned int idx, void *), void *cookie)
{
	size_t i;

	for (i = 0; i < a->len; i++) {
		if (!cb(a->data[i], i, cookie))
			return (0);
	}

	return (1);
}

int
p_array_remove(struct p_array *a, void *data)
{
	size_t i;
	bool found = false;

	if (a->len == 0)
		return (0);

	for (i = 0; i < a->len; i++) {
		if (found)
			a->data[i - 1] = a->data[i];

		if (!found && (a->data[i] == data))
			found = true;
	}

	if (!found)
		return (0);

	a->len--;
	return (1);
}

int
p_array_del(struct p_array *a, unsigned int idx)
{
	size_t i;

	if (a->len <= idx)
		return (0);

	for (i = idx + 1; i < a->len; i++)
		a->data[i -1] = a->data[i];

	a->len--;

	return (1);
}
