// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file az_precondition.h
 *
 * @brief This header defines the types and functions your application uses
 *        to override the default precondition failure behavior.
 *
 *        Public SDK functions validate the arguments passed to them in an effort
 *        to ensure that calling code is passing valid values. The valid value is
 *        called a contract precondition. If an SDK function detects a precondition
 *        failure (invalid argument value), then by default, it calls a function that
 *        places the calling thread into an infinite sleep state; other threads
 *        continue to run.
 *
 *        To override the default behavior, implement a function matching the
 *        az_precondition_failed_fn function signature and then, in your application's
 *        initialization (before calling any Azure SDK function), call
 *        az_precondition_failed_set_callback passing it the address of your function.
 *        Now, when any Azure SDK function detects a precondition failure, it will invoke
 *        your callback instead. You might override the callback to attach a debugger or
 *        perhaps to reboot the device rather than allowing it to continue running with
 *        unpredictable behavior.
 *
 *        Also, if you define the NO_PRECONDITION_CHECKING symbol when compiling the SDK
 *        code, all of the Azure SDK precondition checking will be excluding making the
 *        binary code smaller and faster. We recommend doing this before you ship your code.
 */

#ifndef _az_PRECONDITION_H
#define _az_PRECONDITION_H

#include <stdbool.h>
#include <stddef.h>

#include <az_span.h>

#include <_az_cfg_prefix.h>

/**
 * @brief az_precondition_failed_fn defines the signature of the callback function that application
 * developers can write in order to override the default precondition failure behavior.
 */
typedef void (*az_precondition_failed_fn)();

/**
 * @brief  az_precondition_failed_set_callback allows your application to override the default
 * behavior in response to an SDK function detecting an invalid argument (precondition failure).
 * Call this function once when your application initializes and before you call and Azure SDK
 * functions.
 *
 * @param az_precondition_failed_callback A pointer to the function that will be invoked when
 *                                        an Azure SDK function detects a precondition failure.
 */
void az_precondition_failed_set_callback(az_precondition_failed_fn az_precondition_failed_callback);

#include <_az_cfg_suffix.h>

#endif // _az_PRECONDITION_H
