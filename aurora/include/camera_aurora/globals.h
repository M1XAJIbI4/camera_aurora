/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_GLOBALS_H
#define FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_GLOBALS_H

#ifdef PLUGIN_IMPL
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define PLUGIN_EXPORT
#endif

#endif /* FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_GLOBALS_H */
