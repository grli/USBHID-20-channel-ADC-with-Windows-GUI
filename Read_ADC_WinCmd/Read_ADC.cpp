#include "stdio.h"
#include "SLABHIDDevice.h"

#pragma comment (lib, "SLABHIDDevice.lib")

#define VID								0x10C4
#define PID								0xEAC9

// HID Report IDs
#define ID_IN_CONTROL					0xFE
#define ID_OUT_CONTROL					0xFD

#define CHANNEL_NUM						20
#define VDD_VOLTAGE                     3.28        // Ref 3.28V
#define RATIO							4095.0		// 12 bit adc
#define	REF_VOLTAGE						(VDD_VOLTAGE/8.)	// 8 samples

HID_DEVICE	m_hid;

BYTE report[512], getreport[512];

int main(int argc, char* argv[])
{
	int devnum = HidDevice_GetNumHidDevices(VID, PID);
	if( devnum == 0 )
	{
		printf( "Device not found" );
		return 1;
	}

	BYTE status = HidDevice_Open(&m_hid, 0, VID, PID, MAX_REPORT_REQUEST_XP);
	if( status != HID_DEVICE_SUCCESS )
	{
		printf( "Device open failed" );
		return 1;
	}

	HidDevice_SetTimeouts( m_hid, 100, 100 ); // get/set timeout

	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';   // Start
	report[ 2 ] =  4;    // Number
	report[ 3 ] =  0x41; // Command
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
		printf( "Device write failed" );;
	}

	DWORD bytesReturned = 0;
	BYTE err;
	err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		for( int i = 0; i < CHANNEL_NUM; i++ )
		{
			int sample = (getreport[ 2+i*2 ] << 8) | getreport[ 2 + i*2 + 1 ];
			if( (sample & 0xffff) == 0xffff )
			{
				break;
			}
			if( i == 10 )
			{
				printf("\r\n");
			}
			printf("%2d=%.2fV; ", i,sample/RATIO * REF_VOLTAGE);
		}
	}

	HidDevice_Close(m_hid);
	return 0;
}
