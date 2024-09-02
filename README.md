# Files and dependincies 
This implementation of an RTOS is created for an STM32F429ZIx MCU to be specific,
## miro.c
includes all the RTOS functions defintions
## BSP.c
includes functions related to the specific board used which in this case was the STM32F429I-DISCOVERY board
## main.c
contains two simple threads initiated a thread that blinks the green led on the board and another thread that blinks the red led on the board
this is just a simple illustration example to show that the OS applies context switching between tasks based on thier priority 
tasks or threads created first by the OSthread_Sched() function implicitly have higher priorites than tasks created later on

# NOTE
- the code is documented relatively well. in the future i will probably try to improve more on it by adding semaphores and synchornization between tasks but for now
  its a really good starting point for learning about RTOS in general 


