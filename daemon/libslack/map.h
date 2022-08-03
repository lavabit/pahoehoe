/*
* libslack - http://libslack.org/
*
* Copyright (C) 1999-2002, 2004, 2010, 2020-2021 raf <raf@raf.org>
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
* 20210220 raf <raf@raf.org>
*/

#ifndef LIBSLACK_MAP_H
#define LIBSLACK_MAP_H

#include <slack/hdr.h>
#include <slack/list.h>
#include <slack/locker.h>

typedef struct Map Map;
typedef struct Mapper Mapper;
typedef struct Mapping Mapping;
typedef list_release_t map_release_t;
typedef list_copy_t map_copy_t;
typedef list_cmp_t map_cmp_t;
typedef size_t map_hash_t(size_t table_size, const void *key);
typedef void map_action_t(void *key, void *item, void *data);

_begin_decls
Map *map_create(map_release_t *destroy);
Map *map_create_sized(size_t size, map_release_t *destroy);
Map *map_create_with_hash(map_hash_t *hash, map_release_t *destroy);
Map *map_create_sized_with_hash(size_t size, map_hash_t *hash, map_release_t *destroy);
Map *map_create_with_locker(Locker *locker, map_release_t *destroy);
Map *map_create_with_locker_sized(Locker *locker, size_t size, map_release_t *destroy);
Map *map_create_with_locker_with_hash(Locker *locker, map_hash_t *hash, map_release_t *destroy);
Map *map_create_with_locker_sized_with_hash(Locker *locker, size_t size, map_hash_t *hash, map_release_t *destroy);
Map *map_create_generic(map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
Map *map_create_generic_sized(size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
Map *map_create_generic_with_locker(Locker *locker, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
Map *map_create_generic_with_locker_sized(Locker *locker, size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
int map_rdlock(const Map *map);
int map_wrlock(const Map *map);
int map_unlock(const Map *map);
void map_release(Map *map);
void *map_destroy(Map **map);
int map_own(Map *map, map_release_t *destroy);
int map_own_unlocked(Map *map, map_release_t *destroy);
map_release_t *map_disown(Map *map);
map_release_t *map_disown_unlocked(Map *map);
int map_add(Map *map, const void *key, void *value);
int map_add_unlocked(Map *map, const void *key, void *value);
int map_put(Map *map, const void *key, void *value);
int map_put_unlocked(Map *map, const void *key, void *value);
int map_insert(Map *map, const void *key, void *value, int replace);
int map_insert_unlocked(Map *map, const void *key, void *value, int replace);
int map_remove(Map *map, const void *key);
int map_remove_unlocked(Map *map, const void *key);
void *map_get(Map *map, const void *key);
void *map_get_unlocked(const Map *map, const void *key);
Mapper *mapper_create(Map *map);
Mapper *mapper_create_rdlocked(Map *map);
Mapper *mapper_create_wrlocked(Map *map);
Mapper *mapper_create_unlocked(Map *map);
void mapper_release(Mapper *mapper);
void mapper_release_unlocked(Mapper *mapper);
void *mapper_destroy(Mapper **mapper);
void *mapper_destroy_unlocked(Mapper **mapper);
int mapper_has_next(Mapper *mapper);
void *mapper_next(Mapper *mapper);
const Mapping *mapper_next_mapping(Mapper *mapper);
void mapper_remove(Mapper *mapper);
int map_has_next(Map *map);
void map_break(Map *map);
void *map_next(Map *map);
const Mapping *map_next_mapping(Map *map);
void map_remove_current(Map *map);
const void *mapping_key(const Mapping *mapping);
const void *mapping_value(const Mapping *mapping);
List *map_keys(Map *map);
List *map_keys_unlocked(Map *map);
List *map_keys_with_locker(Locker *locker, Map *map);
List *map_keys_with_locker_unlocked(Locker *locker, Map *map);
List *map_values(Map *map);
List *map_values_unlocked(Map *map);
List *map_values_with_locker(Locker *locker, Map *map);
List *map_values_with_locker_unlocked(Locker *locker, Map *map);
void map_apply(Map *map, map_action_t *action, void *data);
void map_apply_rdlocked(Map *map, map_action_t *action, void *data);
void map_apply_wrlocked(Map *map, map_action_t *action, void *data);
void map_apply_unlocked(Map *map, map_action_t *action, void *data);
ssize_t map_size(Map *map);
ssize_t map_size_unlocked(const Map *map);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
