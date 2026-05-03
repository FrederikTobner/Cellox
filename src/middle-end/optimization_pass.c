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
 * @file optimization_pass.c
 * @brief Implementation of the optimization pass framework and registry
 */

#include "optimization_pass.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

static optimization_pipeline_t* g_global_pipeline = NULL;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Get current time in nanoseconds
 */
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/**
 * @brief Compare passes by priority for sorting
 */
static int pass_compare_priority(const void* a, const void* b) {
    const optimization_pass_entry_t* pa = (const optimization_pass_entry_t*)a;
    const optimization_pass_entry_t* pb = (const optimization_pass_entry_t*)b;
    return (int)pa->priority - (int)pb->priority;
}

/**
 * @brief Find a pass by name
 */
static optimization_pass_entry_t* find_pass_by_name(const char* name) {
    if (g_global_pipeline == NULL) return NULL;
    
    for (size_t i = 0; i < g_global_pipeline->pass_count; i++) {
        if (strcmp(g_global_pipeline->passes[i].name, name) == 0) {
            return &g_global_pipeline->passes[i];
        }
    }
    return NULL;
}

void optimization_module_init(void) {
    if (g_global_pipeline != NULL) {
        return;  // Already initialized
    }
    
    g_global_pipeline = malloc(sizeof(optimization_pipeline_t));
    g_global_pipeline->capacity = 16;
    g_global_pipeline->pass_count = 0;
    g_global_pipeline->passes = malloc(sizeof(optimization_pass_entry_t) * g_global_pipeline->capacity);
    g_global_pipeline->verbose = false;
    g_global_pipeline->max_iterations = 1;
    g_global_pipeline->collect_stats = false;
    
    // Register all passes in order
    // Note: priority is used as execution order after sorting
    
    optimization_pass_entry_t passes[] = {
        {
            .name = "dead_code_detection",
            .fn = pass_dead_code_detection,
            .priority = 0,
            .enabled = true,
            .description = "Mark unreachable instructions"
        },
        {
            .name = "dead_code_elimination",
            .fn = pass_dead_code_elimination,
            .priority = 1,
            .enabled = true,
            .description = "Remove marked dead code"
        },
        {
            .name = "constant_folding",
            .fn = pass_constant_folding,
            .priority = 2,
            .enabled = true,
            .description = "Evaluate compile-time expressions"
        },
        {
            .name = "algebraic_identity",
            .fn = pass_algebraic_identity,
            .priority = 3,
            .enabled = true,
            .description = "Simplify algebraic expressions (x+0, x*1, etc.)"
        },
        {
            .name = "branch_predication",
            .fn = pass_branch_predication,
            .priority = 4,
            .enabled = true,
            .description = "Fold constant-condition branches"
        },
        {
            .name = "constant_pool_dedup",
            .fn = pass_constant_pool_dedup,
            .priority = 5,
            .enabled = false,
            .description = "Merge duplicate constants"
        },
        {
            .name = "dead_code_elimination_2nd",
            .fn = pass_dead_code_elimination,
            .priority = 6,
            .enabled = true,
            .description = "Final dead code cleanup"
        }
    };
    
    size_t num_passes = sizeof(passes) / sizeof(passes[0]);
    for (size_t i = 0; i < num_passes; i++) {
        if (g_global_pipeline->pass_count >= g_global_pipeline->capacity) {
            g_global_pipeline->capacity *= 2;
            g_global_pipeline->passes = realloc(
                g_global_pipeline->passes,
                sizeof(optimization_pass_entry_t) * g_global_pipeline->capacity
            );
        }
        g_global_pipeline->passes[g_global_pipeline->pass_count++] = passes[i];
    }
    
    // Sort passes by priority
    qsort(
        g_global_pipeline->passes,
        g_global_pipeline->pass_count,
        sizeof(optimization_pass_entry_t),
        pass_compare_priority
    );
}

void optimization_module_cleanup(void) {
    if (g_global_pipeline == NULL) return;
    
    free(g_global_pipeline->passes);
    free(g_global_pipeline);
    g_global_pipeline = NULL;
}

optimization_stats_t optimization_pipeline_run_chunk(
    chunk_t* chunk,
    const char* chunk_name
) {
    optimization_stats_t stats = {0};
    stats.chunk_name = chunk_name;
    stats.bytecode_size_before = chunk->byteCodeCount;
    stats.constants_before = chunk->constants.count;
    stats.pass_results = malloc(sizeof(pass_result_t) * 32);
    stats.pass_results_count = 0;
    
    if (g_global_pipeline == NULL) {
        optimization_module_init();
    }
    
    uint64_t start_time = get_time_ns();
    
    // Iterate up to max_iterations times
    for (uint32_t iteration = 0; iteration < g_global_pipeline->max_iterations; iteration++) {
        bool any_modified = false;
        
        // Run each pass in priority order
        for (size_t i = 0; i < g_global_pipeline->pass_count; i++) {
            optimization_pass_entry_t* entry = &g_global_pipeline->passes[i];
            
            if (!entry->enabled) continue;
            
            // Run the pass function
            pass_result_t result = entry->fn(chunk);
            result.pass_name = entry->name;
            
            if (g_global_pipeline->verbose) {
                printf("  [%s] %s: removed %zu instr, folded %zu const, eliminated %zu branches\n",
                    chunk_name, entry->name,
                    result.instructions_removed,
                    result.constants_folded,
                    result.branches_eliminated);
            }
            
            // Collect statistics
            if (g_global_pipeline->collect_stats && stats.pass_results_count < 32) {
                stats.pass_results[stats.pass_results_count++] = result;
            }
            
            if (result.modified) {
                any_modified = true;
            }
        }
        
        // If no pass modified anything, stop iterating
        if (!any_modified) {
            if (g_global_pipeline->verbose && iteration > 0) {
                printf("  [%s] Optimization converged after %u iterations\n",
                    chunk_name, iteration + 1);
            }
            break;
        }
    }
    
    stats.bytecode_size_after = chunk->byteCodeCount;
    stats.constants_after = chunk->constants.count;
    stats.time_ns = get_time_ns() - start_time;
    
    return stats;
}

void optimization_set_pass_enabled(const char* pass_name, bool enabled) {
    optimization_pass_entry_t* pass = find_pass_by_name(pass_name);
    if (pass != NULL) {
        pass->enabled = enabled;
    }
}

void optimization_set_verbose(bool verbose) {
    if (g_global_pipeline == NULL) {
        optimization_module_init();
    }
    g_global_pipeline->verbose = verbose;
}

void optimization_set_max_iterations(uint32_t max_iterations) {
    if (g_global_pipeline == NULL) {
        optimization_module_init();
    }
    g_global_pipeline->max_iterations = max_iterations > 0 ? max_iterations : 1;
}

void optimization_print_stats(const optimization_stats_t* stats) {
    if (stats == NULL) return;
    
    size_t size_reduction = 0;
    if (stats->bytecode_size_before > stats->bytecode_size_after) {
        size_reduction = stats->bytecode_size_before - stats->bytecode_size_after;
    }
    
    int reduction_pct = 0;
    if (stats->bytecode_size_before > 0) {
        reduction_pct = (int)((size_reduction * 100) / stats->bytecode_size_before);
    }
    
    printf("Optimization for '%s':\n", stats->chunk_name);
    printf("  Bytecode: %zu → %zu bytes (%d%% reduction)\n",
        stats->bytecode_size_before,
        stats->bytecode_size_after,
        reduction_pct);
    printf("  Constants: %zu → %zu\n",
        stats->constants_before,
        stats->constants_after);
    printf("  Time: %lu ns (%.2f µs)\n",
        stats->time_ns,
        (double)stats->time_ns / 1000.0);
}
