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
    struct esVTimer *   vTimer,
    esSysTimerTick          tick,
    esVTimerFn          fn,
    void *              arg) {

    esLockCtx           lockCtx;

    ES_CRITICAL_LOCK_ENTER(&lockCtx);
    esVTimerInitI(
        vTimer,
        tick,
        fn,
        arg);
    ES_CRITICAL_LOCK_EXIT(lockCtx);
}

void esVTimerTerm(
    struct esVTimer *   vTimer) {

    esLockCtx           lockCtx;

    ES_CRITICAL_LOCK_ENTER(&lockCtx);
    esVTimerTermI(
        vTimer);
    ES_CRITICAL_LOCK_EXIT(lockCtx);
}

void esVTimerInitI(
    struct esVTimer *   vTimer,
    esSysTimerTick          tick,
    esVTimerFn          fn,
    void *              arg) {

    struct esVTimer *   current;

    ES_API_REQUIRE(ES_API_POINTER, NULL != vTimer);
    ES_API_REQUIRE(ES_API_RANGE, 1U < tick);
    ES_API_REQUIRE(ES_API_POINTER, fn);
    ES_API_REQUIRE(ES_API_USAGE, VTIMER_SIGNATURE != vTimer->signature);

    vTimer->fn = fn;
    vTimer->arg = arg;
    current = GlobalVTimerSentinel.next;

    while (current->rtick < tick) {
        tick -= current->rtick;
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
    ES_API_OBLIGATION(vTimer->signature = VTIMER_SIGNATURE);
}

void esVTimerTermI(
    struct esVTimer *   vTimer) {

    ES_API_REQUIRE(ES_API_POINTER, NULL != vTimer);
    ES_API_REQUIRE(ES_API_OBJECT, VTIMER_SIGNATURE == vTimer->signature);

    if ((struct esVTimer *)&GlobalVTimerSentinel != vTimer->next) {
        vTimer->next->rtick += vTimer->rtick;
    }
    vTimer->prev->next = vTimer->next;
    vTimer->next->prev = vTimer->prev;
    ES_API_OBLIGATION(vTimer->signature = ~VTIMER_SIGNATURE);
}

bool esVTimerIsStillRunningI(
    struct esVTimer *   vTimer) {

    bool                ans;
    struct esVTimer *   current;

    ans = false;
    current = GlobalVTimerSentinel.next;

    while ((struct esVTimer *)&GlobalVTimerSentinel != current) {

        if (vTimer == current) {
            ans = true;

            break;
        }
        current = current->next;
    }

    return (ans);
}

void esVTimerCancel(
    struct esVTimer *   vTimer) {

    esLockCtx           lockCtx;

    ES_CRITICAL_LOCK_ENTER(&lockCtx);

    if (esVTimerIsStillRunningI(vTimer) != false) {
        esVTimerTermI(
            vTimer);
    }
    ES_CRITICAL_LOCK_EXIT(lockCtx);
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
