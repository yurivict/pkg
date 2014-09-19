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

struct p_hash_entry {
	unsigned int index;
	char *key;
	void *value;
	void (*free)(void *);
	struct p_hash_entry *next;
};

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

struct p_hash *
p_hash_new(size_t sz, unsigned int (*hash_func)(const void*, unsigned int, unsigned int))
{
	struct p_hash *h;

	h = calloc(1, sizeof(struct p_hash));
	if (h == NULL)
		return (NULL);

	h->step = sz ? sz : BUFSIZ;
	h->cap = h->step;
	h->table = calloc(h->cap, sizeof(struct p_hash_entry *));
	h->hash = hash_func ? hash_func : bernstein_hash;

	return (h);
}

static void
p_hash_disconnect_entry(struct p_hash *h, struct p_hash_entry *e)
{
	struct p_hash_entry *cur, **prev;

	cur = h->table[e->index];
	prev = &(h->table[e->index]);
	while (cur != NULL) {
		if (cur == e)
			break;
		prev = &(cur->next);
		cur = cur->next;
	}
	*prev = e->next;
	h->cap--;
	h->len--;
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
	size_t i;
	struct p_hash_entry *cur, *next;

	if (h == NULL)
		return;

	for (i = 0; i < h->cap; i++) {
		cur = h->table[i];      
		while (cur != NULL) {
			next = cur->next;
			p_hash_entry_free(h, cur);
			cur = next;
		}
	}
	free(h);
}

static int
p_hash_grow(struct p_hash *h)
{
	struct p_hash_entry **ntable;
	struct p_hash_entry *cur, *next;
	size_t i;
	size_t ni;
	size_t ncap;

	ncap = h->cap;
	ncap += h->step;

	ntable = calloc(ncap, sizeof(struct p_hash_entry *));
	if (ntable == NULL)
		return (0);

	/* rehash to take in account new hash table size */
	for (i = 0; i < h->cap; i++) {
		cur = h->table[i];
		while (cur != NULL) {
			next = cur->next;
			ni = h->hash(cur->key, h->cap, 0);
			cur->index = ni;
			cur->next = ntable[ni];
			ntable[ni] = cur;
			cur = next;
		}
	}
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

	e->key = strdup(key);
	e->value = value;
	e->free = freecb;
	e->index = h->cache;
	e->next = h->table[h->cache];
	h->table[h->cache] = e;
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
	old->next = h->table[h->cache];
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
	struct p_hash_entry *cur;

	cur = p_hash_first(h);
	while (cur != NULL) {
		if (!cb(cur->key, cur->value, cookie))
			return (0);
		cur = p_hash_next(cur);
	}

	return (1);
}


struct p_hash_entry *
p_hash_first(struct p_hash *h)
{
	struct p_hash_entry *cur;
	size_t i;

	for (i = 0; i < h->cap; i++) {
		cur = h->table[i];
		if (cur != NULL)
			return (cur);
	}

	return (NULL);
}

struct p_hash_entry *
p_hash_next(struct p_hash_entry *e)
{

	return (e->next);
}

const char *
p_hash_key(struct p_hash_entry *e)
{
	return (e->key);
}

void *
p_hash_value(struct p_hash_entry *e)
{
	return (e->value);
}
