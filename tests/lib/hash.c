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

#include <atf-c.h>
#include <pkg.h>
#include <core/p_hash.h>

ATF_TC(p_hash);

ATF_TC_HEAD(p_hash, tc)
{
	atf_tc_set_md_var(tc, "descr", "p_hash");
}

int
testfailure(const char *key, void *data, void *cookie)
{
	int *cnt = (int *)cookie;

	(*cnt)++;

	return (0);
}

int
test(const char *key, void *data, void *cookie)
{
	int *cnt = (int *)cookie;

	(*cnt)++;

	return (1);
}

ATF_TC_BODY(p_hash, tc)
{
	struct p_hash *h;
	struct p_hash_entry *e;

	h = p_hash_new(0, NULL);
	ATF_REQUIRE(h != NULL);
	ATF_REQUIRE_EQ(h->step, BUFSIZ);

	p_hash_free(h);

	h = p_hash_new(2, NULL);
	ATF_REQUIRE(h != NULL);
	ATF_REQUIRE_EQ(h->step, 2);
	ATF_REQUIRE_EQ(h->cap, 2);
	ATF_REQUIRE_EQ(h->len, 0);

	ATF_REQUIRE_EQ(p_hash_insert(h, "plop", "test", NULL), 1);
	ATF_REQUIRE_EQ(h->len, 1);
	ATF_REQUIRE_EQ(p_hash_len(h), 1);
	ATF_REQUIRE(p_hash_find(h, "plop") != NULL);
	ATF_REQUIRE_STREQ(p_hash_find(h, "plop"), "test");

	ATF_REQUIRE_EQ(p_hash_insert(h, "plop", "haha", NULL), 0);
	ATF_REQUIRE_EQ(h->len, 1);
	ATF_REQUIRE_EQ(p_hash_rename(h, "plop", "haha"), 1);
	ATF_REQUIRE_EQ(h->len, 1);
	ATF_REQUIRE(p_hash_find(h, "plop") == NULL);
	ATF_REQUIRE_STREQ(p_hash_find(h, "haha"), "test");

	ATF_REQUIRE_EQ(p_hash_replace(h, "haha", "hihi", NULL), 1);
	ATF_REQUIRE_EQ(h->len, 1);
	ATF_REQUIRE_EQ(p_hash_replace(h, "plop", "hihi", NULL), 1);
	ATF_REQUIRE_EQ(h->len, 2);

	int cnt = 0;

	ATF_REQUIRE_EQ(p_hash_foreach(h, testfailure, &cnt), 0);
	ATF_REQUIRE_EQ(cnt, 1);
	cnt = 0;
	ATF_REQUIRE_EQ(p_hash_foreach(h, test, &cnt), 1);
	ATF_REQUIRE_EQ(cnt, 2);

}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, p_hash); 

	return (atf_no_error());
}

