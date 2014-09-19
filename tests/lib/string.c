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
#include <core/p_string.h>

ATF_TC(p_string);

ATF_TC_HEAD(p_string, tc)
{
	atf_tc_set_md_var(tc, "descr", "p_string");
}

ATF_TC_BODY(p_string, tc)
{
	struct p_string *p;

	p = p_string_new(0);
	ATF_REQUIRE(p != NULL);
	ATF_REQUIRE_EQ(p->step, BUFSIZ);

	p_string_free(p);

	p = p_string_new(2);
	ATF_REQUIRE(p != NULL);
	ATF_REQUIRE_EQ(p->step, 2);
	ATF_REQUIRE_EQ(p->cap, 0);
	ATF_REQUIRE_EQ(p->len, 0);

	p_string_append(p, "test", 0);
	ATF_REQUIRE_EQ(p->cap, 6);
	ATF_REQUIRE_EQ(p->len, 4);
	ATF_REQUIRE(p->buf[4] == '\0');
	ATF_REQUIRE_STREQ(p->buf, "test");

	p_string_append(p, "test", 1);
	ATF_REQUIRE_EQ(p->cap, 6);
	ATF_REQUIRE_EQ(p->len, 5);
	ATF_REQUIRE(p->buf[5] == '\0');
	ATF_REQUIRE_STREQ(p->buf, "testt");
	ATF_REQUIRE_STREQ(p_string_data(p), "testt");

	p_string_append(p, "  ", 0);
	ATF_REQUIRE_EQ(p->cap, 8);
	ATF_REQUIRE_EQ(p->len, 7);
	ATF_REQUIRE_EQ(p_string_len(p), 7);
	ATF_REQUIRE_STREQ(p->buf, "testt  ");

	p_string_rtrim(p);
	ATF_REQUIRE_STREQ(p->buf, "testt");
	ATF_REQUIRE_EQ(p->len, 5);

	p_string_reset(p);
	ATF_REQUIRE(p_string_data(p) == NULL);
	ATF_REQUIRE_EQ(p->cap, 8);
	ATF_REQUIRE_EQ(p->len, 0);

	p_string_append(p, " plop", 0);
	p_string_printf(p, "ha %d\n", 12);
	ATF_REQUIRE_STREQ(p->buf, " plopha 12\n");
	ATF_REQUIRE_EQ(p->cap, 12);

	p_string_ltrim(p);
	ATF_REQUIRE_STREQ(p->buf, "plopha 12\n");
	ATF_REQUIRE_EQ(p->len, 10);

	p_string_reset(p);
	p_string_append(p, " a ", 0);
	ATF_REQUIRE_EQ(p->len, 3);
	ATF_REQUIRE_STREQ(p->buf, " a ");
	p_string_trim(p);
	ATF_REQUIRE_EQ(p->len, 1);
	ATF_REQUIRE_STREQ(p->buf, "a");

}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, p_string); 

	return (atf_no_error());
}

