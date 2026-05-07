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
 * @file constant_pool_dedup.c
 * @brief Constant pool deduplication optimization pass
 */

#include "middle-end/optimization_pass.h"
#include "language-models/value_hash_table.h"

/**
 * @brief Constant pool deduplication pass (stub)
 * Merges duplicate constants in the pool
 */
pass_result_t pass_constant_pool_dedup(chunk_t* chunk) {
    (void)chunk;
    pass_result_t result = {
        .pass_name = "constant_pool_dedup",
        .modified = false,
        .instructions_removed = 0,
        .constants_folded = 0,
        .branches_eliminated = 0
    };
    
    
    value_hash_table_t table;
    value_hash_table_init(&table);

    int32_t* remap = malloc(sizeof(int32_t) * chunk->constants.count);
    int32_t new_count = 0;

    for (uint32_t i = 0; i < chunk->constants.count; ++i) {
        value_t v = chunk->constants.values[i];
        int32_t found = -1;
        for (int32_t j = 0; j < new_count; ++j) {
            if (value_values_equal(chunk->constants.values[j], v)) {
                found = j;
                break;
            }
        }
        if (found >= 0) {
            remap[i] = found;
            result.constants_folded++;
            result.modified = true;
        } else {
            chunk->constants.values[new_count] = v;
            remap[i] = new_count;
            new_count++;
        }
    }

    for (uint32_t i = 0; i < chunk->byteCodeCount; ++i) {
        uint8_t op = chunk->code[i];
        if (op == OP_CONSTANT) {
            uint32_t idx = chunk->code[i + 1]; 
            uint32_t new_idx = remap[idx];
            if (new_idx != idx) {
                chunk->code[i + 1] = (uint8_t)new_idx;
            }
        }
    }

    chunk->constants.count = new_count;
    free(remap);
    value_hash_table_free(&table);
    return result;
}
