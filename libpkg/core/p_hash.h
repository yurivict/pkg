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

#ifndef _P_HASH_H
#define _P_HASH_H

struct p_hash_entry;

struct p_hash {
	size_t len;
	size_t cap;
	struct p_hash_entry **table;
	unsigned int (*hash)(const void *, unsigned int, unsigned int);
	unsigned int cache;
	struct p_hash_entry *first;
	struct p_hash_entry *last;
};

struct p_hash_entry {
	unsigned int index;
	char *key;
	void *value;
	void (*free)(void *);
	struct p_hash_entry *next;
};

struct p_hash *p_hash_new(unsigned int (*hash_func)(const void*, unsigned int, unsigned int));

void p_hash_free(struct p_hash *h);
int p_hash_insert(struct p_hash *h, const char *key, void *value, void (*freecb)(void*));
int p_hash_replace(struct p_hash *h, const char *key, void *value, void (*freecb)(void *));
int p_hash_remove(struct p_hash *h, const char *key);
void *p_hash_find(struct p_hash *h, const char *key);
int p_hash_rename(struct p_hash *h, const char *okey, const char *nkey);
size_t p_hash_len(struct p_hash *h);
int p_hash_foreach(struct p_hash *h, int (*cb)(const char *key, void *data, void *cookie), void *cookie);
struct p_hash_entry *p_hash_first(struct p_hash *h);
struct p_hash_entry *p_hash_next(struct p_hash_entry *e);
const char *p_hash_key(struct p_hash_entry *e);
void *p_hash_value(struct p_hash_entry *e);

#endif

