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
 * @author  	Nenad Radulovic
 * @brief       Virtual timer header
 * @details     Based on virtual timers found in ChibiOS/RT
 * @defgroup    base_vtimer Virtual timer
 * @brief       Virtual timer
 *********************************************************************//** @{ */
/**@defgroup    base_vtimer_intf Interface
 * @brief       Virtual timer API
 * @{ *//*--------------------------------------------------------------------*/

#ifndef NTIMER_H
#define NTIMER_H

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>

#include "port/compiler.h"
#include "port/systimer.h"
#include "shared/debug.h"
#include "lib/list.h"

/*===============================================================  MACRO's  ==*/

/**@brief       Convert time (given in milliseconds) into ticks
 */
#define N_TIME_TO_TICK_MS(time_ms)                                              \
    ((time_ms) * CONFIG_SYSTIMER_EVENT_FREQ / 1000ul)

#define NTIMER_ATTR_ONE_SHOT				(0x1u << 0)
#define NTIMER_ATTR_REPEAT					(0x1u << 1)

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Virtual Timer structure
 */
struct ntimer
{
    struct ndlist               list;               /**<@brief Linked list    */
    nsystimer_tick              rtick;              /**<@brief Relative ticks */
    nsystimer_tick              itick;              /**<@brief Initial ticks  */
    void                     (* fn)(void *);        /**<@brief Callback       */
    void *                      arg;                /**<@brief Argument       */
#if (CONFIG_API_VALIDATION == 1)
    ndebug_magic                signature;          /**<@brief Debug signature*/
#endif
};

/**@brief       Virtual Timer structure type
 */
typedef struct ntimer ntimer;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/


void nmodule_timer_init(void);



void ntimer_init(
    struct ntimer *             timer);



/**@brief       Start a timer
 * @param       vTimer
 *              Pointer to timer structure
 * @param       tick
 *              Number of ticks to run
 * @param       fn
 *              Pointer to callback function
 * @param       arg
 *              Argument for callback function
 * @iclass
 */
void ntimer_start_i(
    struct ntimer *             timer,
	nsystimer_tick              tick,
    void                     (* fn)(void *),
    void *                      arg,
	uint8_t						flags);



/**@brief       Start a timer
 * @param       vTimer
 *              Pointer to timer structure
 * @param       tick
 *              Number of ticks to run
 * @param       fn
 *              Pointer to callback function
 * @param       arg
 *              Argument for callback function
 * @api
 */
void ntimer_start(
    struct ntimer *             timer,
	nsystimer_tick              tick,
    void                     (* fn)(void *),
    void *                      arg,
	uint8_t						flags);



/**@brief       Terminate a timer
 * @param       vTimer
 *              Pointer to timer structure
 * @iclass
 */
void ntimer_cancel_i(
    struct ntimer *             timer);



/**@brief       Terminate a timer
 * @param       vTimer
 *              Pointer to timer structure
 * @api
 */
void ntimer_cancel(
    struct ntimer *             timer);



/**@brief       Is a timer still running?
 * @param       vTimer
 *              Pointer to timer structure
 * @return      Timer state
 *  @retval     TRUE - the timer is still running
 *  @retval     FALSE - timer has finished running
 * @iclass
 */
bool ntimer_is_running_i(
    const struct ntimer *       timer);



nsystimer_tick ntimer_remaining(
    const struct ntimer *       timer);

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of ntimer.h
 ******************************************************************************/
#endif /* NTIMER_H */
