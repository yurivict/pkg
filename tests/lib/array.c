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
#include <core/p_array.h>

ATF_TC(p_array);

ATF_TC_HEAD(p_array, tc)
{
	atf_tc_set_md_var(tc, "descr", "p_array");
}

static int
testfailure(void *data __unused, unsigned int idx, void *cookie)
{
	int *cnt = (int *) cookie;

	ATF_REQUIRE_EQ(idx, *cnt);

	(*cnt) ++;

	return (0);
}

static int
test(void *data __unused, unsigned int idx, void *cookie)
{
	int *cnt = (int *) cookie;

	ATF_REQUIRE_EQ(idx, *cnt);
	if (idx == 0)
		ATF_REQUIRE_STREQ(data, "plop");
	if (idx == 1)
		ATF_REQUIRE_STREQ(data, "new");

	(*cnt) ++;

	return (1);
}

ATF_TC_BODY(p_array, tc)
{
	struct p_array *a;

	a = p_array_new();
	ATF_REQUIRE(a != NULL);
	ATF_REQUIRE_EQ(a->cap, 0);
	ATF_REQUIRE_EQ(a->len, 0);

	p_array_push(a, "plop");
	ATF_REQUIRE_EQ(a->cap, 64);
	ATF_REQUIRE_EQ(p_array_len(a), 1);
	ATF_REQUIRE_STREQ(p_array_get(a, 0), "plop");

	p_array_push(a, "new");
	p_array_push(a, "again");
	ATF_REQUIRE_EQ(a->cap, 64);
	ATF_REQUIRE_EQ(a->len, 3);
	ATF_REQUIRE_STREQ(p_array_get(a, 0), "plop");
	ATF_REQUIRE_STREQ(p_array_get(a, 1), "new");
	ATF_REQUIRE_STREQ(p_array_get(a, 2), "again");

	ATF_REQUIRE_STREQ(p_array_pop(a), "again");
	ATF_REQUIRE_EQ(p_array_len(a), 2);
	ATF_REQUIRE_STREQ(p_array_get(a, 0), "plop");
	ATF_REQUIRE_STREQ(p_array_get(a, 1), "new");

	p_array_push(a, "again");
	ATF_REQUIRE_STREQ(p_array_get(a, 0), "plop");
	ATF_REQUIRE_STREQ(p_array_get(a, 1), "new");
	ATF_REQUIRE_STREQ(p_array_get(a, 2), "again");
	ATF_REQUIRE_EQ(p_array_remove(a, "new"), 1);
	ATF_REQUIRE_EQ(p_array_len(a), 2);
	ATF_REQUIRE_STREQ(p_array_get(a, 0), "plop");
	ATF_REQUIRE_STREQ(p_array_get(a, 1), "again");

	p_array_push(a, "new");
	ATF_REQUIRE_EQ(p_array_del(a, 5), 0);
	ATF_REQUIRE_STREQ(p_array_get(a, 0), "plop");
	ATF_REQUIRE_STREQ(p_array_get(a, 1), "again");
	ATF_REQUIRE_STREQ(p_array_get(a, 2), "new");
	ATF_REQUIRE_EQ(p_array_del(a, 1), 1);
	ATF_REQUIRE_STREQ(p_array_get(a, 0), "plop");
	ATF_REQUIRE_STREQ(p_array_get(a, 1), "new");

	int cnt = 0;
	ATF_REQUIRE_EQ(p_array_foreach(a, testfailure, &cnt), 0);
	ATF_REQUIRE_EQ(cnt, 1);
	cnt = 0;
	ATF_REQUIRE_EQ(p_array_foreach(a, test, &cnt), 1);
	ATF_REQUIRE_EQ(cnt, 2);

}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, p_array); 

	return (atf_no_error());
}

