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
 * @file optimization_pass.h
 * @brief Header file for the optimization pass framework
 */

#ifndef CELLOX_OPTIMIZATION_PASS_H_
#define CELLOX_OPTIMIZATION_PASS_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "byte-code/chunk.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Result of running a single optimization pass
 */
typedef struct {
    const char* pass_name;
    bool modified;                    ///< Did this pass change the chunk?
    size_t instructions_removed;      ///< Diagnostics
    size_t constants_folded;
    size_t branches_eliminated;
} pass_result_t;

/**
 * @brief Function signature for all optimization passes
 * @param chunk The chunk to optimize
 * @return pass_result_t with diagnostics
 */
typedef pass_result_t (*optimization_pass_t)(chunk_t* chunk);

/**
 * @brief Registered pass in the pipeline
 */
typedef struct {
    const char* name;                 ///< e.g., "constant_folding"
    optimization_pass_t fn;           ///< Function pointer
    uint32_t priority;                ///< Execution order (0 = first)
    bool enabled;                     ///< Can be disabled for testing
    const char* description;
} optimization_pass_entry_t;

/**
 * @brief Pass pipeline configuration
 */
typedef struct {
    optimization_pass_entry_t* passes;
    size_t pass_count;
    size_t capacity;
    
    // Pipeline options
    bool verbose;                     ///< Print pass results?
    uint32_t max_iterations;          ///< Repeat until no changes
    bool collect_stats;               ///< Track per-pass metrics
} optimization_pipeline_t;

/**
 * @brief Statistics for a single chunk optimization
 */
typedef struct {
    const char* chunk_name;           ///< Function name
    size_t bytecode_size_before;
    size_t bytecode_size_after;
    size_t constants_before;
    size_t constants_after;
    uint64_t time_ns;                 ///< Total time for all passes
    
    pass_result_t* pass_results;      ///< Per-pass results
    size_t pass_results_count;
} optimization_stats_t;

/**
 * @brief Bitset for tracking dead code indices
 */
typedef struct {
    uint8_t* bits;                    ///< Packed bits
    size_t num_bytes;
    size_t total_indices;
} dead_code_bitset_t;

// ============================================================================
// Public API
// ============================================================================

/**
 * @brief Initialize the global optimization pipeline
 * @details Must be called once at VM startup
 */
void optimization_module_init(void);

/**
 * @brief Cleanup the global optimization pipeline
 */
void optimization_module_cleanup(void);

/**
 * @brief Run the optimization pipeline on a chunk
 * @param chunk The chunk to optimize
 * @param chunk_name Name of the chunk (for diagnostics)
 * @return Optimization statistics
 */
optimization_stats_t optimization_pipeline_run_chunk(
    chunk_t* chunk,
    const char* chunk_name
);

/**
 * @brief Enable/disable a specific pass
 * @param pass_name Name of the pass
 * @param enabled Whether the pass should run
 */
void optimization_set_pass_enabled(const char* pass_name, bool enabled);

/**
 * @brief Set pipeline verbosity
 * @param verbose Whether to print pass results
 */
void optimization_set_verbose(bool verbose);

/**
 * @brief Set maximum optimization iterations
 * @param max_iterations Maximum times to re-run the pipeline
 */
void optimization_set_max_iterations(uint32_t max_iterations);

/**
 * @brief Set optimization level similar to compiler -O levels
 * @param level Level in range [0, 3]
 * @details O0 disables optimization passes, O1 enables light optimizations,
 * O2 enables the default full safe pipeline, O3 enables full pipeline with
 * additional iteration.
 */
void optimization_set_level(uint32_t level);

/**
 * @brief Get current optimization level
 * @return Current level in range [0, 3]
 */
uint32_t optimization_get_level(void);

/**
 * @brief Print statistics for a chunk optimization
 * @param stats Statistics to print
 */
void optimization_print_stats(const optimization_stats_t* stats);

// ============================================================================
// Pass Implementations (declared in individual pass files)
// ============================================================================

pass_result_t pass_dead_code_detection(chunk_t* chunk);
pass_result_t pass_dead_code_elimination(chunk_t* chunk);
pass_result_t pass_constant_folding(chunk_t* chunk);
pass_result_t pass_algebraic_identity(chunk_t* chunk);
pass_result_t pass_branch_predication(chunk_t* chunk);
pass_result_t pass_constant_pool_dedup(chunk_t* chunk);

#ifdef __cplusplus
}
#endif

#endif // CELLOX_OPTIMIZATION_PASS_H_
