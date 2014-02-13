/******************************************************************************
 * This file is part of formula_display
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Nenad Radulovic
 * @brief       Implementacija V timera
 *********************************************************************//** @{ */

/*=========================================================  INCLUDE FILES  ==*/

#include <stddef.h>

#include "plat/critical.h"
#include "vtimer/vtimer.h"

/*=========================================================  LOCAL MACRO's  ==*/

#define VTIMER_SIGNATURE                  0xDEEFBEEFU

/*======================================================  LOCAL DATA TYPES  ==*/

struct VTimerBase {
    struct esVTimer *   next;
    struct esVTimer *   prev;
    esSysTimerTick          tick;
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/

static const ES_MODULE_INFO_CREATE("VTimer", "Virtual Timer", "Nenad Radulovic");

static struct VTimerBase GlobalVTimerSentinel = {
    (struct esVTimer *)&GlobalVTimerSentinel,
    (struct esVTimer *)&GlobalVTimerSentinel,
    (esSysTimerTick)(-1)
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void esVTimerInit(
    struct esVTimer *   vTimer) {

    ES_API_REQUIRE(ES_API_POINTER, vTimer != NULL);
    ES_API_REQUIRE(ES_API_OBJECT,  vTimer->signature != VTIMER_SIGNATURE);

    vTimer->next = vTimer;

    ES_API_OBLIGATION(vTimer->signature = VTIMER_SIGNATURE);
}

void esVTimerTerm(
    struct esVTimer *   vTimer) {

    esLockCtx           lockCtx;

    ES_CRITICAL_LOCK_ENTER(&lockCtx);
    esVTimerTermI(
        vTimer);
    ES_CRITICAL_LOCK_EXIT(lockCtx);
}

void esVTimerTermI(
    struct esVTimer *   vTimer) {

    ES_API_REQUIRE(ES_API_POINTER, vTimer != NULL);
    ES_API_REQUIRE(ES_API_OBJECT,  vTimer->signature == VTIMER_SIGNATURE);

    if (vTimer->next != vTimer) {

        if ((struct esVTimer *)&GlobalVTimerSentinel != vTimer->next) {
            vTimer->next->rtick += vTimer->rtick;
        }
        vTimer->prev->next = vTimer->next;
        vTimer->next->prev = vTimer->prev;
    }
    ES_API_OBLIGATION(vTimer->signature = ~VTIMER_SIGNATURE);
}

void esVTimerStart(
    struct esVTimer *   vTimer,
    esSysTimerTick      tick,
    esVTimerFn          fn,
    void *              arg) {

    esLockCtx           lockCtx;

    ES_CRITICAL_LOCK_ENTER(&lockCtx);
    esVTimerStartI(
        vTimer,
        tick,
        fn,
        arg);
    ES_CRITICAL_LOCK_EXIT(lockCtx);
}

void esVTimerStartI(
    struct esVTimer *   vTimer,
    esSysTimerTick      tick,
    esVTimerFn          fn,
    void *              arg) {

    struct esVTimer *   current;

    ES_API_REQUIRE(ES_API_POINTER, vTimer != NULL);
    ES_API_REQUIRE(ES_API_USAGE,   vTimer->signature == VTIMER_SIGNATURE);
    ES_API_REQUIRE(ES_API_RANGE,   tick > 1u);
    ES_API_REQUIRE(ES_API_POINTER, fn != NULL);

    vTimer->fn  = fn;
    vTimer->arg = arg;
    current = GlobalVTimerSentinel.next;

    while (current->rtick < tick) {
        tick   -= current->rtick;
        current = current->next;
    }
    vTimer->next = current;
    vTimer->prev = current->prev;
    vTimer->prev->next = vTimer;
    vTimer->next->prev = vTimer;
    vTimer->rtick = tick;

    if ((struct esVTimer *)&GlobalVTimerSentinel != current) {
        current->rtick -= tick;
    }
}

bool esVTimerIsRunningI(
    struct esVTimer *   vTimer) {

    ES_API_REQUIRE(ES_API_POINTER, vTimer != NULL);
    ES_API_REQUIRE(ES_API_USAGE,   vTimer->signature == VTIMER_SIGNATURE);

    if (vTimer->next != vTimer) {

        return (true);
    } else {

        return (false);
    }
}

void esVTimerEvaluateI(
    void) {

    if ((struct esVTimer *)&GlobalVTimerSentinel != GlobalVTimerSentinel.next) {
        struct esVTimer * current;

        current = GlobalVTimerSentinel.next;
        ES_API_REQUIRE(ES_API_USAGE, VTIMER_SIGNATURE == current->signature);
        --current->rtick;

        while (0U == current->rtick) {
            struct esVTimer * tmp;

            ES_API_REQUIRE(ES_API_USAGE, VTIMER_SIGNATURE == current->signature);
            current->prev->next = current->next;
            current->next->prev = current->prev;
            current->next = current;
            ES_API_OBLIGATION(current->signature = ~VTIMER_SIGNATURE);
            tmp = current;
            current = GlobalVTimerSentinel.next;
            (* tmp->fn)(tmp->arg);
        }
    }
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of vtmr.c
 ******************************************************************************/
