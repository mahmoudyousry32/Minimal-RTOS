/*
 * miros.h
 *
 *  Created on: Aug 30, 2024
 *      Author: pc
 */

#ifndef EXAMPLE_USER_MIROS_H_
#define EXAMPLE_USER_MIROS_H_

typedef struct  {
    void* sp; //stack pointer of the current thread
    uint32_t timeout;
    uint8_t priority;
}OSthread;

typedef void (*OSthreadhandler)(void);  // pointer to the the handler of the thread

enum { XPSR, RA, LR, R12, R3, R2, R1, R0,R11,R10,R9,R8,R7,R6,R5,R4 };



void OSthread_Sched(OSthread* my_thread, OSthreadhandler my_threadhandler, void* current_sp, uint32_t stk_size);
void OS_Sched();
void OS_run();
void OS_startup();
void OS_init();
void OS_delay();
void OS_tick();

#endif /* EXAMPLE_USER_MIROS_H_ */
