/*
* libslack - http://libslack.org/
*
* Copyright (C) 1999-2002, 2004, 2010, 2020 raf <raf@raf.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <https://www.gnu.org/licenses/>.
*
* 20201111 raf <raf@raf.org>
*/

#ifndef LIBSLACK_STR_H
#define LIBSLACK_STR_H

#include <stdio.h>
#include <stdarg.h>

#include <regex.h>

#include <slack/hdr.h>
#include <slack/list.h>
#include <slack/locker.h>

typedef struct String String;
typedef struct StringTR StringTR;

enum StringAlignment
{
	ALIGN_LEFT       = '<',
	ALIGN_RIGHT      = '>',
	ALIGN_CENTRE     = '|',
	ALIGN_CENTER     = '|',
	ALIGN_FULL       = '='
};

enum StringTROption
{
	TR_COMPLEMENT = 1,
	TR_DELETE     = 2,
	TR_SQUASH     = 4
};

typedef enum StringAlignment StringAlignment;
typedef enum StringTROption StringTROption;

_begin_decls
String *str_create(const char *format, ...);
String *str_create_with_locker(Locker *locker, const char *format, ...);
String *str_vcreate(const char *format, va_list args);
String *str_vcreate_with_locker(Locker *locker, const char *format, va_list args);
String *str_create_sized(size_t size, const char *format, ...);
String *str_create_with_locker_sized(Locker *locker, size_t size, const char *format, ...);
String *str_vcreate_sized(size_t size, const char *format, va_list args);
String *str_vcreate_with_locker_sized(Locker *locker, size_t size, const char *format, va_list args);
String *str_copy(const String *str);
String *str_copy_unlocked(const String *str);
String *str_copy_with_locker(Locker *locker, const String *str);
String *str_copy_with_locker_unlocked(Locker *locker, const String *str);
String *str_fgetline(FILE *stream);
String *str_fgetline_with_locker(Locker *locker, FILE *stream);
void str_release(String *str);
void *str_destroy(String **str);
int str_rdlock(const String *str);
int str_wrlock(const String *str);
int str_unlock(const String *str);
int str_empty(const String *str);
int str_empty_unlocked(const String *str);
ssize_t str_length(const String *str);
ssize_t str_length_unlocked(const String *str);
char *cstr(const String *str);
ssize_t str_set_length(String *str, size_t length);
ssize_t str_set_length_unlocked(String *str, size_t length);
ssize_t str_recalc_length(String *str);
ssize_t str_recalc_length_unlocked(String *str);
String *str_clear(String *str);
String *str_clear_unlocked(String *str);
String *str_remove(String *str, ssize_t index);
String *str_remove_unlocked(String *str, ssize_t index);
String *str_remove_range(String *str, ssize_t index, ssize_t range);
String *str_remove_range_unlocked(String *str, ssize_t index, ssize_t range);
String *str_insert(String *str, ssize_t index, const char *format, ...);
String *str_insert_unlocked(String *str, ssize_t index, const char *format, ...);
String *str_vinsert(String *str, ssize_t index, const char *format, va_list args);
String *str_vinsert_unlocked(String *str, ssize_t index, const char *format, va_list args);
String *str_insert_str(String *str, ssize_t index, const String *src);
String *str_insert_str_unlocked(String *str, ssize_t index, const String *src);
String *str_append(String *str, const char *format, ...);
String *str_append_unlocked(String *str, const char *format, ...);
String *str_vappend(String *str, const char *format, va_list args);
String *str_vappend_unlocked(String *str, const char *format, va_list args);
String *str_append_str(String *str, const String *src);
String *str_append_str_unlocked(String *str, const String *src);
String *str_prepend(String *str, const char *format, ...);
String *str_prepend_unlocked(String *str, const char *format, ...);
String *str_vprepend(String *str, const char *format, va_list args);
String *str_vprepend_unlocked(String *str, const char *format, va_list args);
String *str_prepend_str(String *str, const String *src);
String *str_prepend_str_unlocked(String *str, const String *src);
String *str_replace(String *str, ssize_t index, ssize_t range, const char *format, ...);
String *str_replace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, ...);
String *str_vreplace(String *str, ssize_t index, ssize_t range, const char *format, va_list args);
String *str_vreplace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, va_list args);
String *str_replace_str(String *str, ssize_t index, ssize_t range, const String *src);
String *str_replace_str_unlocked(String *str, ssize_t index, ssize_t range, const String *src);
String *str_substr(const String *str, ssize_t index, ssize_t range);
String *str_substr_unlocked(const String *str, ssize_t index, ssize_t range);
String *str_substr_with_locker(Locker *locker, const String *str, ssize_t index, ssize_t range);
String *str_substr_with_locker_unlocked(Locker *locker, const String *str, ssize_t index, ssize_t range);
String *substr(const char *str, ssize_t index, ssize_t range);
String *substr_with_locker(Locker *locker, const char *str, ssize_t index, ssize_t range);
String *str_splice(String *str, ssize_t index, ssize_t range);
String *str_splice_unlocked(String *str, ssize_t index, ssize_t range);
String *str_splice_with_locker(Locker *locker, String *str, ssize_t index, ssize_t range);
String *str_splice_with_locker_unlocked(Locker *locker, String *str, ssize_t index, ssize_t range);
String *str_repeat(size_t count, const char *format, ...);
String *str_repeat_with_locker(Locker *locker, size_t count, const char *format, ...);
String *str_vrepeat(size_t count, const char *format, va_list args);
String *str_vrepeat_with_locker(Locker *locker, size_t count, const char *format, va_list args);
int str_tr(String *str, const char *from, const char *to, int option);
int str_tr_unlocked(String *str, const char *from, const char *to, int option);
int str_tr_str(String *str, const String *from, const String *to, int option);
int str_tr_str_unlocked(String *str, const String *from, const String *to, int option);
int tr(char *str, const char *from, const char *to, int option);
StringTR *tr_compile(const char *from, const char *to, int option);
StringTR *tr_compile_with_locker(Locker *locker, const char *from, const char *to, int option);
StringTR *str_tr_compile(const String *from, const String *to, int option);
StringTR *str_tr_compile_unlocked(const String *from, const String *to, int option);
StringTR *str_tr_compile_with_locker(Locker *locker, const String *from, const String *to, int option);
StringTR *str_tr_compile_with_locker_unlocked(Locker *locker, const String *from, const String *to, int option);
void tr_release(StringTR *table);
void *tr_destroy(StringTR **table);
int str_tr_compiled(String *str, StringTR *table);
int str_tr_compiled_unlocked(String *str, StringTR *table);
int tr_compiled(char *str, StringTR *table);
List *str_regexpr(const char *pattern, const String *text, int cflags, int eflags);
List *str_regexpr_unlocked(const char *pattern, const String *text, int cflags, int eflags);
List *str_regexpr_with_locker(Locker *locker, const char *pattern, const String *text, int cflags, int eflags);
List *str_regexpr_with_locker_unlocked(Locker *locker, const char *pattern, const String *text, int cflags, int eflags);
List *regexpr(const char *pattern, const char *text, int cflags, int eflags);
List *regexpr_with_locker(Locker *locker, const char *pattern, const char *text, int cflags, int eflags);
int regexpr_compile(regex_t *compiled, const char *pattern, int cflags);
void regexpr_release(regex_t *compiled);
List *str_regexpr_compiled(const regex_t *compiled, const String *text, int eflags);
List *str_regexpr_compiled_unlocked(const regex_t *compiled, const String *text, int eflags);
List *str_regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const String *text, int eflags);
List *str_regexpr_compiled_with_locker_unlocked(Locker *locker, const regex_t *compiled, const String *text, int eflags);
List *regexpr_compiled(const regex_t *compiled, const char *text, int eflags);
List *regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const char *text, int eflags);
String *str_regsub(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all);
String *str_regsub_unlocked(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all);
String *str_regsub_compiled(const regex_t *compiled, const char *replacement, String *text, int eflags, int all);
String *str_regsub_compiled_unlocked(const regex_t *compiled, const char *replacement, String *text, int eflags, int all);
List *str_fmt(const String *str, size_t line_width, StringAlignment alignment);
List *str_fmt_unlocked(const String *str, size_t line_width, StringAlignment alignment);
List *str_fmt_with_locker(Locker *locker, const String *str, size_t line_width, StringAlignment alignment);
List *str_fmt_with_locker_unlocked(Locker *locker, const String *str, size_t line_width, StringAlignment alignment);
List *fmt(const char *str, size_t line_width, StringAlignment alignment);
List *fmt_with_locker(Locker *locker, const char *str, size_t line_width, StringAlignment alignment);
List *str_split(const String *str, const char *delim);
List *str_split_unlocked(const String *str, const char *delim);
List *str_split_with_locker(Locker *locker, const String *str, const char *delim);
List *str_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim);
List *split(const char *str, const char *delim);
List *split_with_locker(Locker *locker, const char *str, const char *delim);
List *str_regexpr_split(const String *str, const char *delim, int cflags, int eflags);
List *str_regexpr_split_unlocked(const String *str, const char *delim, int cflags, int eflags);
List *str_regexpr_split_with_locker(Locker *locker, const String *str, const char *delim, int cflags, int eflags);
List *str_regexpr_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim, int cflags, int eflags);
List *regexpr_split(const char *str, const char *delim, int cflags, int eflags);
List *regexpr_split_with_locker(Locker *locker, const char *str, const char *delim, int cflags, int eflags);
String *str_join(const List *list, const char *delim);
String *str_join_unlocked(const List *list, const char *delim);
String *str_join_with_locker(Locker *locker, const List *list, const char *delim);
String *str_join_with_locker_unlocked(Locker *locker, const List *list, const char *delim);
String *join(const List *list, const char *delim);
String *join_with_locker(Locker *locker, const List *list, const char *delim);
int str_soundex(const String *str);
int str_soundex_unlocked(const String *str);
int soundex(const char *str);
String *str_trim(String *str);
String *str_trim_unlocked(String *str);
char *trim(char *str);
String *str_trim_left(String *str);
String *str_trim_left_unlocked(String *str);
char *trim_left(char *str);
String *str_trim_right(String *str);
String *str_trim_right_unlocked(String *str);
char *trim_right(char *str);
String *str_squeeze(String *str);
String *str_squeeze_unlocked(String *str);
char *squeeze(char *str);
String *str_quote(const String *str, const char *quotable, char quote_char);
String *str_quote_unlocked(const String *str, const char *quotable, char quote_char);
String *str_quote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char);
String *str_quote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char);
String *quote(const char *str, const char *quotable, char quote_char);
String *quote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char);
String *str_unquote(const String *str, const char *quotable, char quote_char);
String *str_unquote_unlocked(const String *str, const char *quotable, char quote_char);
String *str_unquote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char);
String *str_unquote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char);
String *unquote(const char *str, const char *quotable, char quote_char);
String *unquote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char);
String *str_encode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_encode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_encode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_encode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_decode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_decode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_decode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_decode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *encode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *encode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *decode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *decode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
String *str_lc(String *str);
String *str_lc_unlocked(String *str);
char *lc(char *str);
String *str_lcfirst(String *str);
String *str_lcfirst_unlocked(String *str);
char *lcfirst(char *str);
String *str_uc(String *str);
String *str_uc_unlocked(String *str);
char *uc(char *str);
String *str_ucfirst(String *str);
String *str_ucfirst_unlocked(String *str);
char *ucfirst(char *str);
int str_chop(String *str);
int str_chop_unlocked(String *str);
int chop(char *str);
int str_chomp(String *str);
int str_chomp_unlocked(String *str);
int chomp(char *str);
int str_bin(const String *str);
int str_bin_unlocked(const String *str);
int bin(const char *str);
int str_hex(const String *str);
int str_hex_unlocked(const String *str);
int hex(const char *str);
int str_oct(const String *str);
int str_oct_unlocked(const String *str);
int oct(const char *str);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
#ifndef strlcpy /* This is now a macro on OSX/macOS */
size_t strlcpy(char *dst, const char *src, size_t size);
#endif
#ifndef strlcat /* This is now a macro on OSX/macOS */
size_t strlcat(char *dst, const char *src, size_t size);
#endif
char *cstrcpy(char *dst, const char *src);
char *cstrcat(char *dst, const char *src);
char *cstrchr(const char *str, int c);
char *cstrpbrk(const char *str, const char *brk);
char *cstrrchr(const char *str, int c);
char *cstrstr(const char *str, const char *srch);
int asprintf(char **str, const char *format, ...);
int vasprintf(char **str, const char *format, va_list args);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
