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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p_hash.h"

#define p_roundup2(x,y) (((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#define STRSIZE 4096
#define STEPS 64

static unsigned int
bernstein_hash(const void *key, unsigned int limit, unsigned int seed)
{
	int c;
	unsigned int hash = seed;
	const char *k = (const char *)key;

	while ((c = *k++) != '\0')
		hash = (hash * 33) + c;

	return (hash % limit);
}

static unsigned int
jen_hash(const void *key, unsigned int limit, unsigned int seed)
{
	unsigned int hash = seed;
	const char *k = (const char *)key;
	int c;

	while ((c = *k++) != '\0') {
		hash += c;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return (hash & (limit -1));
}


struct p_hash *
p_hash_new(unsigned int (*hash_func)(const void*, unsigned int, unsigned int))
{
	struct p_hash *h;

	h = calloc(1, sizeof(struct p_hash));
	if (h == NULL)
		return (NULL);

	h->cap = STEPS;
	h->table = calloc(h->cap, sizeof(struct p_hash_entry *));
	h->hash = hash_func ? hash_func : jen_hash;

	return (h);
}

static void
p_hash_disconnect_entry(struct p_hash *h, struct p_hash_entry *e)
{
	struct p_hash_entry *prev;

	h->table[e->index] = NULL;

	h->len--;

	if (e == h->first) {
		h->first = e->next;
		return;
	}

	for (prev = h->first; prev != NULL && prev->next != e; prev = prev->next);

	if (prev != NULL)
		prev->next = e->next;
}

static void
p_hash_entry_free(struct p_hash *h, struct p_hash_entry *e)
{
	p_hash_disconnect_entry(h, e);

	free(e->key);
	if (e->free)
		e->free(e->value);
	free(e);
}

void
p_hash_free(struct p_hash *h)
{
	struct p_hash_entry *e;

	if (h == NULL)
		return;

	while ((e = h->first))
		p_hash_entry_free(h, e);

	free(h);
}

static int
p_hash_grow(struct p_hash *h)
{
	struct p_hash_entry **ntable;
	struct p_hash_entry *cur;
	size_t ni;
	size_t ncap;

	ncap = h->cap;
	ncap *= 2;
	ncap = p_roundup2(ncap, STRSIZE);

	ntable = calloc(ncap, sizeof(struct p_hash_entry *));
	if (ntable == NULL)
		return (0);

	/* rehash to take in account new hash table size */
	for (cur = h->first; cur != NULL; cur = cur->next) {
		h->last = cur;
		ni = h->hash(cur->key, ncap, 0);
		cur->index = ni;
		ntable[ni] = cur;
	}
	/* Ensure last is alwasy NULL */
	h->last->next = NULL;
	free(h->table);
	h->table = ntable;
	h->cap = ncap;

	return (1);
}

static struct p_hash_entry *
p_hash_lookup(struct p_hash *h, const char *key)
{
	struct p_hash_entry *e;

	h->cache = h->hash(key, h->cap, 0);
	e = h->table[h->cache];
	while (e != NULL) {
		if (strcmp(e->key, key) == 0)
			break;
		e = e->next;
	}

	return (e);
}

int
p_hash_insert(struct p_hash *h, const char *key, void *value, void (*freecb)(void*))
{
	struct p_hash_entry *e;

	/* Check for duplicate key */
	if (p_hash_lookup(h, key))
		return (0);

	/* resize if 75 % of the capacity used */
	if (h->len >= h->cap * 75 / 100 )
		if (!p_hash_grow(h))
			return (0);

	e = calloc(1, sizeof(struct p_hash_entry));
	if (e == NULL)
		return (0);

	if (h->last != NULL)
		h->last->next = e;
	if (h->first == NULL)
		h->first = e;

	e->key = strdup(key);
	e->value = value;
	e->free = freecb;
	e->index = h->hash(e->key, h->cap, 0);
	e->next = NULL;
	h->last = e;
	h->last->next = NULL;
	h->table[e->index] = e;
	h->len++;

	return (1);
}

int
p_hash_replace(struct p_hash *h, const char *key, void *value, void (*freecb)(void *))
{
	struct p_hash_entry *e;

	e = p_hash_lookup(h, key);
	if (e == NULL)
		return (p_hash_insert(h, key, value, freecb));

	if (e->free != NULL)
		e->free(e->value);
	e->value = value;

	return (1);
}

int
p_hash_remove(struct p_hash *h, const char *key)
{
	struct p_hash_entry *e;

	e = p_hash_lookup(h, key);
	if (e == NULL)
		return (0);

	p_hash_entry_free(h, e);

	return (1);
}

void *
p_hash_find(struct p_hash *h, const char *key)
{
	struct p_hash_entry *e;

	e = p_hash_lookup(h, key);
	if (e == NULL)
		return (NULL);

	return (e->value);
}

int
p_hash_rename(struct p_hash *h, const char *okey, const char *nkey)
{
	struct p_hash_entry *old, *new;

	old = p_hash_lookup(h, okey);
	if (old == NULL)
		return (-1);

	new = p_hash_lookup(h, nkey);
	if (new != NULL)
		return (0);

	p_hash_disconnect_entry(h, old);
	free(old->key);
	old->key = strdup(nkey);
	old->index = h->cache;
	if (h->first == NULL)
		h->first = old;
	h->last = old;
	old->next = NULL;
	h->table[h->cache] = old;
	h->len++;

	return (1);
}

size_t
p_hash_len(struct p_hash *h)
{
	return (h->len);
}

int
p_hash_foreach(struct p_hash *h, int (*cb)(const char *key, void *data, void *cookie), void *cookie)
{
	struct p_hash_entry *e;

	for (e = h->first ; e != NULL; e = e->next) {
		if (!cb(e->key, e->value, cookie))
			return (0);
	}

	return (1);
}

struct p_hash_entry *
p_hash_first(struct p_hash *h)
{
	return (h->first);
}

struct p_hash_entry *
p_hash_next(struct p_hash_entry *e)
{
	return (e->next);
}
