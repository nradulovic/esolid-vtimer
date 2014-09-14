/*
 * This file is part of eSolid.
 *
 * Copyright (C) 2010 - 2013 Nenad Radulovic
 *
 * eSolid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eSolid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eSolid.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementacija V timera
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "plat/sys_lock.h"
#include "timer/ntimer.h"

/*=========================================================  LOCAL MACRO's  ==*/

#define TIMER_SIGNATURE                     ((ncpu_reg)0xdeedbeefu)

#define NODE_TO_TIMER(node)                                                     \
    CONTAINER_OF(node, struct ntimer, list)

/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/

/**@brief       Evaluate all running timers
 * @details     This function must be called from a system timer interrupt
 *              routine.
 * @iclass
 */
static void timer_evaluate_i(void);

/*=======================================================  LOCAL VARIABLES  ==*/

static const NMODULE_INFO_CREATE("Timer", "Nenad Radulovic");

static struct ntimer g_timer_sentinel;

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/

static void timer_evaluate_i(void)
{
    if (!ndlist_is_empty(&g_timer_sentinel.list)) {
        struct ntimer * current;

        current = NODE_TO_TIMER(ndlist_next(&g_timer_sentinel.list));
        NREQUIRE(NAPI_USAGE, TIMER_SIGNATURE == current->signature);
        --current->rtick;

        while (current->rtick == 0u) {
            struct ntimer * tmp;

            NREQUIRE(NAPI_USAGE, TIMER_SIGNATURE == current->signature);
            ndlist_remove(&current->list);
            ndlist_init(&current->list);
            NOBLIGATION(current->signature = ~TIMER_SIGNATURE);
            tmp     = current;
            current = NODE_TO_TIMER(ndlist_next(&g_timer_sentinel.list));
            tmp->fn(tmp->arg);
        }
    }
}

/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/


void nmodule_timer_init(void)
{
    ndlist_init(&g_timer_sentinel.list);
    g_timer_sentinel.rtick = NCORE_TIMER_MAX;
    ncore_timer_init(NCORE_TIMER_ONE_TICK);
    ncore_timer_enable();
    ncore_timer_isr_enable();
    ntimer_set_handler(timer_evaluate_i, 0);
}

void ntimer_init(
    struct ntimer *             timer)
{
    NREQUIRE(NAPI_POINTER, timer != NULL);
    NREQUIRE(NAPI_OBJECT,  timer->signature != TIMER_SIGNATURE);

    ndlist_init(&timer->list);
}



void ntimer_cancel_i(
    struct ntimer *             timer)
{
    NREQUIRE(NAPI_POINTER, timer != NULL);

    if (!ndlist_is_empty(&timer->list)) {
        NREQUIRE(NAPI_OBJECT,  timer->signature == TIMER_SIGNATURE);

        if (&g_timer_sentinel != NODE_TO_TIMER(ndlist_next(&timer->list))) {
            NODE_TO_TIMER(ndlist_next(&timer->list))->rtick += timer->rtick;
        }
        ndlist_remove(&timer->list);
        ndlist_init(&timer->list);
    }
    NOBLIGATION(timer->signature = ~TIMER_SIGNATURE);
}



void ntimer_cancel(
    struct ntimer *             timer)
{
    nsys_lock                   sys_lock;

    nsys_lock_enter(&sys_lock);
    ntimer_cancel_i(timer);
    nsys_lock_exit(&sys_lock);
}



void ntimer_start_i(
    struct ntimer *             timer,
    ncore_timer_tick            tick,
    void                     (* fn)(void *),
    void *                      arg)
{
    struct ntimer *             current;

    NREQUIRE(NAPI_POINTER, timer != NULL);
    NREQUIRE(NAPI_USAGE,   timer->signature != TIMER_SIGNATURE);
    NREQUIRE(NAPI_RANGE,   tick > 1u);
    NREQUIRE(NAPI_POINTER, fn != NULL);

    timer->fn  = fn;
    timer->arg = arg;
    current    = NODE_TO_TIMER(ndlist_next(&g_timer_sentinel.list));

    while (current->rtick < tick) {
        tick   -= current->rtick;
        current = NODE_TO_TIMER(ndlist_next(&current->list));
    }
    ndlist_add_before(&current->list, &timer->list);
    timer->rtick = tick;

    if (&g_timer_sentinel != current) {
        current->rtick -= tick;
    }
    NOBLIGATION(timer->signature = TIMER_SIGNATURE);
}



void ntimer_start(
    struct ntimer *             timer,
    ncore_timer_tick            tick,
    void                     (* fn)(void *),
    void *                      arg)
{
    nsys_lock                   sys_lock;

    nsys_lock_enter(&sys_lock);
    ntimer_start_i(timer, tick, fn, arg);
    nsys_lock_exit(&sys_lock);
}



bool ntimer_is_running_i(
    const struct ntimer *       timer) {

    NREQUIRE(NAPI_POINTER, timer != NULL);

    if (!ndlist_is_empty(&timer->list)) {
        NREQUIRE(NAPI_USAGE, timer->signature == TIMER_SIGNATURE);

        return (true);
    } else {

        return (false);
    }
}



ncore_timer_tick ntimer_remaining(
    const struct ntimer *       timer)
{
    nsys_lock                   sys_lock;
    ncore_timer_tick            remaining;

    remaining = 0u;
    nsys_lock_enter(&sys_lock);

    if (ntimer_is_running_i(timer)) {

        do {
            remaining += timer->rtick;
            timer      = NODE_TO_TIMER(ndlist_prev(&timer->list));
        } while (timer != &g_timer_sentinel);
    }
    nsys_lock_exit(&sys_lock);

    return (remaining);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of timer.c
 ******************************************************************************/
