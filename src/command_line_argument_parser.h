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
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>>GNU General Public  *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file command_line_argument_parser.h
 * @brief Header file containing the declarations regarrding the command line argument parser.
 */

#ifndef CELLOX_COMMAND_LINE_ARGUMENT_PARSER_H_
#define CELLOX_COMMAND_LINE_ARGUMENT_PARSER_H_

/// @brief Parses the command line arguements that were specified when cellox was called
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
/// @note if the arguments could not be parsed the program exits with an exit code indicating an command-line-usage error by the user
void command_line_argument_parser_parse(int argc, char const ** argv);

#endif