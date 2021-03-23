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

#ifndef LIBSLACK_LIST_H
#define LIBSLACK_LIST_H

#include <stdarg.h>

#include <sys/types.h>

#include <slack/hdr.h>
#include <slack/locker.h>

typedef struct List List;
typedef struct Lister Lister;
typedef void list_release_t(void *item);
typedef void *list_copy_t(const void *item);
typedef int list_cmp_t(const void *a, const void *b);
typedef void list_action_t(void *item, size_t *index, void *data);
typedef void *list_map_t(void *item, size_t *index, void *data);
typedef int list_query_t(void *item, size_t *index, void *data);

_begin_decls
List *list_create(list_release_t *destroy);
List *list_make(list_release_t *destroy, ...);
List *list_vmake(list_release_t *destroy, va_list args);
List *list_copy(const List *src, list_copy_t *copy);
List *list_create_with_locker(Locker *locker, list_release_t *destroy);
List *list_make_with_locker(Locker *locker, list_release_t *destroy, ...);
List *list_vmake_with_locker(Locker *locker, list_release_t *destroy, va_list args);
List *list_copy_with_locker(Locker *locker, const List *src, list_copy_t *copy);
int list_rdlock(const List *list);
int list_wrlock(const List *list);
int list_unlock(const List *list);
void list_release(List *list);
void *list_destroy(List **list);
int list_own(List *list, list_release_t *destroy);
int list_own_unlocked(List *list, list_release_t *destroy);
list_release_t *list_disown(List *list);
list_release_t *list_disown_unlocked(List *list);
void *list_item(const List *list, ssize_t index);
void *list_item_unlocked(const List *list, ssize_t index);
int list_item_int(const List *list, ssize_t index);
int list_item_int_unlocked(const List *list, ssize_t index);
int list_empty(const List *list);
int list_empty_unlocked(const List *list);
ssize_t list_length(const List *list);
ssize_t list_length_unlocked(const List *list);
ssize_t list_last(const List *list);
ssize_t list_last_unlocked(const List *list);
List *list_remove(List *list, ssize_t index);
List *list_remove_unlocked(List *list, ssize_t index);
List *list_remove_range(List *list, ssize_t index, ssize_t range);
List *list_remove_range_unlocked(List *list, ssize_t index, ssize_t range);
List *list_insert(List *list, ssize_t index, void *item);
List *list_insert_unlocked(List *list, ssize_t index, void *item);
List *list_insert_int(List *list, ssize_t index, int item);
List *list_insert_int_unlocked(List *list, ssize_t index, int item);
List *list_insert_list(List *list, ssize_t index, const List *src, list_copy_t *copy);
List *list_insert_list_unlocked(List *list, ssize_t index, const List *src, list_copy_t *copy);
List *list_append(List *list, void *item);
List *list_append_unlocked(List *list, void *item);
List *list_append_int(List *list, int item);
List *list_append_int_unlocked(List *list, int item);
List *list_append_list(List *list, const List *src, list_copy_t *copy);
List *list_append_list_unlocked(List *list, const List *src, list_copy_t *copy);
List *list_prepend(List *list, void *item);
List *list_prepend_unlocked(List *list, void *item);
List *list_prepend_int(List *list, int item);
List *list_prepend_int_unlocked(List *list, int item);
List *list_prepend_list(List *list, const List *src, list_copy_t *copy);
List *list_prepend_list_unlocked(List *list, const List *src, list_copy_t *copy);
List *list_replace(List *list, ssize_t index, ssize_t range, void *item);
List *list_replace_unlocked(List *list, ssize_t index, ssize_t range, void *item);
List *list_replace_int(List *list, ssize_t index, ssize_t range, int item);
List *list_replace_int_unlocked(List *list, ssize_t index, ssize_t range, int item);
List *list_replace_list(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy);
List *list_replace_list_unlocked(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy);
List *list_extract(const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_extract_unlocked(const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_extract_with_locker(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_extract_with_locker_unlocked(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_push(List *list, void *item);
List *list_push_unlocked(List *list, void *item);
List *list_push_int(List *list, int item);
List *list_push_int_unlocked(List *list, int item);
void *list_pop(List *list);
void *list_pop_unlocked(List *list);
int list_pop_int(List *list);
int list_pop_int_unlocked(List *list);
void *list_shift(List *list);
void *list_shift_unlocked(List *list);
int list_shift_int(List *list);
int list_shift_int_unlocked(List *list);
List *list_unshift(List *list, void *item);
List *list_unshift_unlocked(List *list, void *item);
List *list_unshift_int(List *list, int item);
List *list_unshift_int_unlocked(List *list, int item);
List *list_splice(List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_splice_unlocked(List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_splice_with_locker(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_splice_with_locker_unlocked(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy);
List *list_sort(List *list, list_cmp_t *cmp);
List *list_sort_unlocked(List *list, list_cmp_t *cmp);
void list_apply(List *list, list_action_t *action, void *data);
void list_apply_rdlocked(List *list, list_action_t *action, void *data);
void list_apply_wrlocked(List *list, list_action_t *action, void *data);
void list_apply_unlocked(List *list, list_action_t *action, void *data);
List *list_map(List *list, list_release_t *destroy, list_map_t *map, void *data);
List *list_map_unlocked(List *list, list_release_t *destroy, list_map_t *map, void *data);
List *list_map_with_locker(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data);
List *list_map_with_locker_unlocked(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data);
List *list_grep(List *list, list_query_t *grep, void *data);
List *list_grep_unlocked(List *list, list_query_t *grep, void *data);
List *list_grep_with_locker(Locker *locker, List *list, list_query_t *grep, void *data);
List *list_grep_with_locker_unlocked(Locker *locker, List *list, list_query_t *grep, void *data);
ssize_t list_query(List *list, ssize_t *index, list_query_t *query, void *data);
ssize_t list_query_unlocked(List *list, ssize_t *index, list_query_t *query, void *data);
Lister *lister_create(List *list);
Lister *lister_create_rdlocked(List *list);
Lister *lister_create_wrlocked(List *list);
Lister *lister_create_unlocked(const List *list);
void lister_release(Lister *lister);
void lister_release_unlocked(Lister *lister);
void *lister_destroy(Lister **lister);
void *lister_destroy_unlocked(Lister **lister);
int lister_has_next(Lister *lister);
void *lister_next(Lister *lister);
int lister_next_int(Lister *lister);
void lister_remove(Lister *lister);
int list_has_next(List *list);
void list_break(List *list);
void *list_next(List *list);
int list_next_int(List *list);
void list_remove_current(List *list);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
