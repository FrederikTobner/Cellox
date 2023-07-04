/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file garbage_collector.h
 * @brief Header file containing the declarations of functionality used for garbage collection.
 */

#ifndef CELLOX_GARBAGE_COLLECTOR_H_
#define CELLOX_GARBAGE_COLLECTOR_H_

#include "../language-models/object.h"

/** @brief Starts the garbage collection process.
 * @details The garbage collector of cellox is a precise GC.
 * That means that the garbage collector knows whether words in memory are pointers
 * and which store a value - like a string, boolean or a number.
 * The Algorithm that is used is called mark-and-sweep.
 * It consists of two phases:
 * 1. Mark phase
 * 2. Sweep phase <br>
 * In the marking phase we start at the roots and traverse through all the objects the roots refer to.
 * In the sweeping phase all the reachable objects have been marked, and therefore we can reclaim the memory that is
 * used by the unmarked objects.
 */
void garbage_collector_collect_garbage(void);

/// @brief Marks a cellox object
/// @param object The object that is marked
void garbage_collector_mark_object(object_t * object);

/// @brief Marks a cellox value
/// @param value The value that is marked
void garbage_collector_mark_value(value_t value);

#endif