/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------




* Needed to increase stack size so that the printf would work correctly
* (no stack overflow).
* RTX_Conf_CM.c Default thread and Main thread stack size set to
* 2000.
* startup_stm32f407xx.s (Startup) change stack to 2000 (no change)
* Needed to include Run Time Environment->Compiler->I/O->STDOUT (select ITM)
* for printf.
 changed #define PLL_M      8 in system_stm32f4xx.c
* This works! 
look at apnt_230.pdf
Follow step #5 (PLL_M), #14 Configure SWV
Make sure 168MHz set Xtal and Core Clock for debug trace

changed RTOS kernal timer clock freq. to 168 MHz
*---------------------------------------------------------------------------*/


#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "cmsis_os.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include "Driver_USART.h" // USART API


/* USART Driver */
extern ARM_DRIVER_USART Driver_USART4;
osThreadId tid_myUART_Thread;

void myUART_Thread (void const *arg);                           // function prototype for Thread_1
osThreadDef (myUART_Thread, osPriorityNormal, 1, 0);            // define Thread_1

void myUSART_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:    
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
    case ARM_USART_EVENT_SEND_COMPLETE:
    case ARM_USART_EVENT_TX_COMPLETE:
        /* Success: Wakeup Thread */
        osSignalSet(tid_myUART_Thread, 0x01);
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}
 




int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  printf("test\n");
	
	// create the semaphore
	// start with a count of 1
	  
  tid_myUART_Thread = osThreadCreate (osThread (myUART_Thread), NULL);         // create the thread
  if (tid_myUART_Thread == NULL) {                                        // handle thread creation
    // Failed to create a thread
  };

 
	
  osKernelStart ();                         // start thread execution 
}


void myUART_Thread(void const *arg){
	  static ARM_DRIVER_USART * USARTdrv = &Driver_USART4;
    ARM_DRIVER_VERSION     version;
    ARM_USART_CAPABILITIES drv_capabilities;
    char                   cmd;
 
    version = USARTdrv->GetVersion();
    if (version.api < 0x200)   /* requires at minimum API version 2.00 or higher */
    {                          /* error handling */
        return;
    }
    drv_capabilities = USARTdrv->GetCapabilities();
    if (drv_capabilities.event_tx_complete == 0)
    {                          /* error handling */
        return;
    }
 
    /*Initialize the USART driver */
    USARTdrv->Initialize(myUSART_callback);
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 4800 Bits/sec */
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 4800);
     
    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
 
    USARTdrv->Send("\nPress Enter to receive a message", 34);
    osSignalWait(0x01, osWaitForever);
     
    while (1)
    {
        USARTdrv->Receive(&cmd, 1);          /* Get byte from UART */
        osSignalWait(0x01, osWaitForever);
        if (cmd == 13)                       /* CR, send greeting  */
        {
          USARTdrv->Send("\nHello World!", 12);
          osSignalWait(0x01, osWaitForever);
        }
 
    }
	
}

