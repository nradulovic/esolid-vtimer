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

#ifndef ES_VTIMER_H_
#define ES_VTIMER_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stdbool.h>

#include "plat/compiler.h"
#include "arch/systimer.h"
#include "base/debug.h"

/*===============================================================  MACRO's  ==*/

/**@brief       Convert time (given in milliseconds) into ticks
 */
#define ES_VTMR_TIME_TO_TICK_MS(time)                                           \
    ((time) * CONFIG_SYSTIMER_EVENT_FREQ / 1000u)

#if (CONFIG_API_VALIDATION == 1)
#define ES_VTIMER_INITIALIZER()                                                 \
    {NULL, NULL, 0, NULL, NULL, 0}
#else
#define ES_VTIMER_INITIALIZER()                                                 \
    {NULL, NULL, 0, NULL, NULL}
#endif

/*------------------------------------------------------  C++ extern begin  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/**@brief       Virtual Timer callback function type
 */
typedef void (* esVTimerFn)(void *);

/**@brief       Virtual Timer structure
 */
struct esVTimer {
    struct esVTimer *   next;                                                   /**< @brief Next timer in linked list.                      */
    struct esVTimer *   prev;                                                   /**< @brief Previous timer in linked list.                  */
    esSysTimerTick      rtick;                                                  /**< @brief Relative number of ticks.                       */
    esVTimerFn          fn;                                                     /**< @brief Timer callback function.                        */
    void *              arg;                                                    /**< @brief Argument for timer callback function.           */
#if (CONFIG_API_VALIDATION == 1)
    esAtomic            signature;                                              /**< @brief Timer structure signature.                      */
#endif
};

/**@brief       Virtual Timer structure type
 */
typedef struct esVTimer esVTimer;

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

void esModuleVTimerInit(
    void);

void esVTimerInit(
    struct esVTimer *   vTimer);

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
void esVTimerStartI(
    struct esVTimer *   vTimer,
    esSysTimerTick      tick,
    esVTimerFn          fn,
    void *              arg);

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
void esVTimerStart(
    struct esVTimer *   vTimer,
    esSysTimerTick      tick,
    esVTimerFn          fn,
    void *              arg);

/**@brief       Terminate a timer
 * @param       vTimer
 *              Pointer to timer structure
 * @iclass
 */
void esVTimerCancelI(
    struct esVTimer *   vTimer);

/**@brief       Terminate a timer
 * @param       vTimer
 *              Pointer to timer structure
 * @api
 */
void esVTimerCancel(
    struct esVTimer *   vTimer);

/**@brief       Is a timer still running?
 * @param       vTimer
 *              Pointer to timer structure
 * @return      Timer state
 *  @retval     TRUE - the timer is still running
 *  @retval     FALSE - timer has finished running
 * @iclasss
 */
bool esVTimerIsRunningI(
    struct esVTimer *   vTimer);


/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//** @} *//*********************************************
 * END of vtimer.h
 ******************************************************************************/
#endif /* ES_VTIMER_H_ */
