/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "efm8_device.h"
#include "boot.h"
#include "flash.h"
#include "efm8ub10adc.h"
#include "usb_main.h"

// Holds the current command opcode
static uint8_t opcode;

// Holds reply to the current command
static uint8_t reply;

// ----------------------------------------------------------------------------
// Perform the bootloader verify command.
// ----------------------------------------------------------------------------
void doVerifyCmd(void)
{
}

//-----------------------------------------------------------------------------
// SiLabs_Startup() Routine
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code is run in SILABS_STARTUP.A51 (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
void SiLabs_Startup (void)
{
  // Disable the watchdog here
}

SI_SBIT(LED, SFR_P3, 0);               // P1.4 LED

// ----------------------------------------------------------------------------
// Bootloader Mainloop
// ----------------------------------------------------------------------------
void main(void)
{
  char loop;

  // Initialize the communication channel and clear the flash keys
  boot_initDevice();
  flash_setKeys(0, 0);
  flash_setBank(0);

  VREF_0_enter_DefaultMode_from_RESET();
  ADC_0_enter_DefaultMode_from_RESET();
  TIMER16_2_enter_DefaultMode_from_RESET();
  INTERRUPT_0_enter_DefaultMode_from_RESET();

  // Loop until a run application command is received
  while (true)
  {
    // Wait for a valid boot record to arrive
    boot_nextRecord();

    LED = 0;

    // Receive the command byte and convert to opcode
    opcode = boot_getByte();
    
    // Assume success - handlers will modify if there is an error
    reply = BOOT_ACK_REPLY;

    // Interpret the command opcode
    switch (opcode)
    {
      case 0x40: // Set channel
        for( loop = 0; loop < ADC_CHANNELS; loop++ )
        {
            chn_def_P[ loop ] = boot_getByte();
        }
        break;

      case 0x41:
        // ADC results share the RAM same as USB RAM
        break;

      case 0x48: // Reboot to bootloader
          *((uint8_t SI_SEG_DATA *)0x00) = 0xA5;
          RSTSRC = RSTSRC_SWRSF__SET | RSTSRC_PORSF__SET;
          break;

      case 0x49: // Get version
          break;

      default:
        // Return bootloader revision for any unrecognized command
        //reply = BL_REVISION;
        break;
    }

    // Reply with the results of the command
    boot_sendReply(reply);
  }
}
