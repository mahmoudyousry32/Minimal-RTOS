#include <stdint.h>
#include "miros.h"
#include "stm32f4xx_hal.h"


extern OSthread green;
extern OSthread red;
OSthread  * volatile current_thread ; // points to the current thread
OSthread  * volatile next_thread; // points to the next thread
OSthread *thread_arr[33]; //an array of pointers to threads
uint8_t thread_num = 0; //number of threads started or created
uint8_t currentIdx = 0; //index to indicate which thread is currently running


// Idle thread for when there are no threads running or preempted ,OS calls this thread when all threads are blocked
// or when there are no threads created
OSthread idle_thread;
uint32_t stack_idle[40] __attribute__ ((aligned (8)));
uint32_t *sp_idle = &stack_idle[40];

uint32_t OS_readyset; //a "register" where each bit corresponds to a thread's state and whether its ready to run or blocked
						// 1 : thread is ready to run  		   0 : thread is blocked
						// currently priority for execution of  threads is based on the order of their initilization served in A First-in-First-Out manner
						// idle thread is not represented here as its always ready to run
void idle_thread_f();
void OS_init(){

	OSthread_Sched(&idle_thread,&idle_thread_f,stack_idle,sizeof(stack_idle));
}


void OS_delay(uint32_t ticks){				 //blocks the caller task for the amount of ticks
	__disable_irq(); //disable interrupts
	//Q_ASSERT(current_thread != &idle_thread) //Make sure that the calling thread is not the idle thread as idle thread MUST NOT BE BLOCKED
	current_thread->timeout = ticks;  //sets the timeout member of the current thread to the number of ticks
												//which will be decremented with each tick(interrupt triggerd by systick timer)

	OS_readyset &= ~(1U<<(currentIdx-1U)); //blocks the calling thread

		OS_Sched();//calls the scheduler to schedule the next thread in the array to be executed
		__enable_irq();
}
void OSthread_Sched(OSthread* my_thread, OSthreadhandler my_threadhandler, void* current_sp, uint32_t stk_size) {
    uint32_t* my_sp = (uint32_t*)((((uint32_t)current_sp + stk_size) / 8) * 8); //moves the stack pointer to the begining of the stack frame and ensures that
                                                                                // the stack pointer is a multiple of 8 to align the data
                                                                                // its like malloc() where it allocates a number of bytes depending on stk_size
                                                                                // aligned to an address multiple of 8
    																		    //*(current_sp) == my_stack[0] note ;
    																			// my_sp should point one element beyond the last element in the stack
    																			//stack size use sizeof(my_stack)

    //Q_ASSERT(thread_num < MAX_THREADS);

    *(--my_sp) = (1U << 24); //PUSH XPSR
    *(--my_sp) = (uint32_t)my_threadhandler; //PUSH RA AND LOAD IT WITH ADDRESS OF THREAD HANDLER
    *(--my_sp) = LR; //PUSH LR
    *(--my_sp) = R12; //PUSH R12
    *(--my_sp) = R3; //PUSH R3
    *(--my_sp) = R2; //PUSH R2
    *(--my_sp) = R1; //PUSH R1
    *(--my_sp) = R0; //PUSH R0
    *(--my_sp) = R11; //PUSH R11
    *(--my_sp) = R10; //PUSH R10
    *(--my_sp) = R9; //PUSH R9
    *(--my_sp) = R8; //PUSH R8
    *(--my_sp) = R7; //PUSH R7
    *(--my_sp) = R6; //PUSH R6
    *(--my_sp) = R5; //PUSH R5
    *(--my_sp) = R4; //PUSH R4
    my_thread->sp = my_sp; //Saves the stack pointer which is now at the top of the stack (pointing to the last element entered) ;

    uint32_t *stck_limit = (uint32_t*)(((((uint32_t)current_sp - 1U) / 8) + 1U) * 8); // fill the rest of the stack with deadbeef
    for (my_sp = my_sp - 1; my_sp >= stck_limit; --my_sp) {
        *my_sp = 0xdeadbeef;
    }
    //Check for overflowing of the threads array using an assertion
    //if this assertion fails go to Q_ON_ASSERT function
    //or simple go to the hardfault handler if an overflow occurs




    //after initiating the stack for "my_thread" store the thread in the  thread array
    thread_arr[thread_num] = my_thread; //my_thread is already a pointer

    if(thread_num > 0 ){//all threads are created ready by default
    	OS_readyset  |= (1U << (thread_num - 1U));
    }

    thread_num++; //increment thread_num
    my_thread->priorty = thread_num;

}
//MUST BE CALLED WHEN INTERRUPTS ARE DISABLED FROM A CRITICAL SECTION
void OS_Sched(){
	currentIdx =  0; //this is to start searching for the thread with the highest priority each time the OS_Sche is called
					//if no thread with a higher priority than the current one running is found then currentIdx will increment till it reaches the current thread running
					//and nothing changes
	if(OS_readyset == 0U ){		//no threads are active so give control to the idle task
		next_thread = thread_arr[0];
	}
	else{
		//check which thread is ready to run by iterating over the OS_readyset bits
		do{
			currentIdx++; //increment currentIdx by 1 to start at the beginning of the thread array
			if(currentIdx == thread_num) currentIdx = 1; //if we reach the last thread stored (entered) in the array overflow so that the first thread in the array executes on the next scheduling

		}while((OS_readyset & (1U<<(currentIdx - 1)))==0U); //keep iterating through the OS_readyset bits until a set bit is reached,
															// which indicates that this thread is ready to run next
		next_thread = thread_arr[currentIdx] ; 			//schedule the next thread to run
	}

	if(next_thread != current_thread )  //if the next thread to run is the current thread then dont call PendSV handler
	*((uint32_t volatile *)0xE000ED04) = 0x10000000; // trigger PendSV to load the stack frame of the next thread to run
}

void OS_run(){
	OS_startup();
	__disable_irq(); //interrupts must be disabled when context switching to avoid an interrupt firing within a context switch
	OS_Sched(); //calls the scheduler to schedule the first process to run
	__enable_irq();

	//Q_ERROR(); ASSERT AN ERROR AS THE CONTROL SHOULD NEVER BE GIVEN BACK TO OS_run();

}

void OS_tick(){
	uint8_t n;
	for(n = 1 ; n < thread_num ; n++){
		if(thread_arr[n]->timeout != 0U)
			thread_arr[n]->timeout--;
		if(thread_arr[n]->timeout == 0U)
		OS_readyset |= 1U << (n-1);
	}
}

__attribute__ ((naked))
void PendSV_Handler(void){
	__asm volatile (
	    /* __disable_irq(); */
	    "  CPSID         I                 \n"

	    /* if (OS_curr != (OSThread *)0) { */
	    "  LDR           r1,=current_thread       \n"
	    "  LDR           r1,[r1,#0x00]     \n"
	    "  CMP           r1,#0             \n"
	    "  BEQ           PendSV_restore    \n"

	    /*     push registers r4-r11 on the stack */
	#if (__ARM_ARCH == 6)               // if ARMv6-M...
	    "  SUB           sp,sp,#(8*4)     \n" // make room for 8 registers r4-r11
	    "  MOV           r0,sp            \n" // r0 := temporary stack pointer
	    "  STMIA         r0!,{r4-r7}      \n" // save the low registers
	    "  MOV           r4,r8            \n" // move the high registers to low registers...
	    "  MOV           r5,r9            \n"
	    "  MOV           r6,r10           \n"
	    "  MOV           r7,r11           \n"
	    "  STMIA         r0!,{r4-r7}      \n" // save the high registers
	#else                               // ARMv7-M or higher
	    "  PUSH          {r4-r11}          \n"
	#endif                              // ARMv7-M or higher

	    /*     OS_curr->sp = sp; */
	    "  LDR           r1,=current_thread       \n"
	    "  LDR           r1,[r1,#0x00]     \n"
	    "  MOV           r0,sp             \n"
	    "  STR           r0,[r1,#0x00]     \n"
	    /* } */

	    "PendSV_restore:                   \n"
	    /* sp = OS_next->sp; */
	    "  LDR           r1,=next_thread       \n"
	    "  LDR           r1,[r1,#0x00]     \n"
	    "  LDR           r0,[r1,#0x00]     \n"
	    "  MOV           sp,r0             \n"

	    /* OS_curr = OS_next; */
	    "  LDR           r1,=next_thread       \n"
	    "  LDR           r1,[r1,#0x00]     \n"
	    "  LDR           r2,=current_thread       \n"
	    "  STR           r1,[r2,#0x00]     \n"

	    /* pop registers r4-r11 */
	#if (__ARM_ARCH == 6)               // if ARMv6-M...
	    "  MOV           r0,sp             \n" // r0 := top of stack
	    "  MOV           r2,r0             \n"
	    "  ADDS          r2,r2,#(4*4)      \n" // point r2 to the 4 high registers r7-r11
	    "  LDMIA         r2!,{r4-r7}       \n" // pop the 4 high registers into low registers
	    "  MOV           r8,r4             \n" // move low registers into high registers
	    "  MOV           r9,r5             \n"
	    "  MOV           r10,r6            \n"
	    "  MOV           r11,r7            \n"
	    "  LDMIA         r0!,{r4-r7}       \n" // pop the low registers
	    "  ADD           sp,sp,#(8*4)      \n" // remove 8 registers from the stack
	#else                               // ARMv7-M or higher
	    "  POP           {r4-r11}          \n"
	#endif                              // ARMv7-M or higher

	    /* __enable_irq(); */
	    "  CPSIE         I                 \n"

	    /* return to the next thread */
	    "  BX            lr                \n"
	    );


}


