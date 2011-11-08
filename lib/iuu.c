/* 
 *  iuutool - a port of WBE's Infinity USB Unlimited SDK
 * 
 *  Copyright (C) 2006 Juan Carlos Borrás 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation. 
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this program; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <usb.h>

#include <iuu.h>

enum iuu_usb_params {
   IUU_USB_VENDOR_ID = 0x104f,
   IUU_USB_PRODUCT_ID = 0x0004,
   IUU_USB_OP_TIMEOUT = 0x0200
};

/* Programmer commands */
enum iuu_command {
   IUU_NO_OPERATION = 0x00,
   IUU_GET_FIRMWARE_VERSION = 0x01,
   IUU_GET_PRODUCT_NAME = 0x02,
   IUU_GET_STATE_REGISTER = 0x03,
   IUU_SET_LED = 0x04,
   IUU_WAIT_MUS = 0x05,
   IUU_WAIT_MS = 0x06,

   IUU_GET_LOADER_VERSION = 0x50,
   IUU_RST_SET = 0x52,
   IUU_RST_CLEAR = 0x53,
   IUU_SET_VCC = 0x59,

   IUU_UART_ENABLE = 0x49,
   IUU_UART_DISABLE = 0x4A,
   IUU_UART_WRITE_I2C = 0x4C,
   IUU_UART_ESC = 0x5E,
   IUU_UART_TRAP = 0x54,
   IUU_UART_TRAP_BREAK = 0x5B,
   IUU_UART_RX = 0x56,

   IUU_AVR_ON = 0x21,
   IUU_AVR_OFF = 0x22,
   IUU_AVR_1CLK = 0x23,
   IUU_AVR_RESET = 0x24,
   IUU_AVR_RESET_PC = 0x25,
   IUU_AVR_INC_PC = 0x26,
   IUU_AVR_INCN_PC = 0x27,
   IUU_AVR_PREAD = 0x29,
   IUU_AVR_PREADN = 0x2A,
   IUU_AVR_PWRITE = 0x28,
   IUU_AVR_DREAD = 0x2C,
   IUU_AVR_DREADN = 0x2D,
   IUU_AVR_DWRITE = 0x2B,
   IUU_AVR_PWRITEN = 0x2E,

   IUU_EEPROM_ON = 0x37,
   IUU_EEPROM_OFF = 0x38,
   IUU_EEPROM_WRITE = 0x39,
   IUU_EEPROM_WRITEX = 0x3A,
   IUU_EEPROM_WRITE8 = 0x3B,
   IUU_EEPROM_WRITE16 = 0x3C,
   IUU_EEPROM_WRITEX32 = 0x3D,
   IUU_EEPROM_WRITEX64 = 0x3E,
   IUU_EEPROM_READ = 0x3F,
   IUU_EEPROM_READX = 0x40,
   IUU_EEPROM_BREAD = 0x41,
   IUU_EEPROM_BREADX = 0x42,

   IUU_PIC_CMD = 0x0A,
   IUU_PIC_CMD_LOAD = 0x0B,
   IUU_PIC_CMD_READ = 0x0C,
   IUU_PIC_ON = 0x0D,
   IUU_PIC_OFF = 0x0E,
   IUU_PIC_RESET = 0x16,
   IUU_PIC_INC_PC = 0x0F,
   IUU_PIC_INCN_PC = 0x10,
   IUU_PIC_PWRITE = 0x11,
   IUU_PIC_PREAD = 0x12,
   IUU_PIC_PREADN = 0x13,
   IUU_PIC_DWRITE = 0x14,
   IUU_PIC_DREAD = 0x15
};

enum iuu_extra_command {
   IUU_UART_NOP = 0x00,
   IUU_UART_CHANGE = 0x02,
   IUU_UART_TX = 0x04,
   IUU_DELAY_MS = 0x06
};

/* 
 Table for Inverse to Direct Convention conversion
 (taken from somewhere in OpenSC)
*/
static const u_int8_t inverse2direct[0x100] = {
   0xff, 0x7f, 0xbf, 0x3f, 0xdf, 0x5f, 0x9f, 0x1f,
   0xef, 0x6f, 0xaf, 0x2f, 0xcf, 0x4f, 0x8f, 0xf,
   0xf7, 0x77, 0xb7, 0x37, 0xd7, 0x57, 0x97, 0x17,
   0xe7, 0x67, 0xa7, 0x27, 0xc7, 0x47, 0x87, 0x7,
   0xfb, 0x7b, 0xbb, 0x3b, 0xdb, 0x5b, 0x9b, 0x1b,
   0xeb, 0x6b, 0xab, 0x2b, 0xcb, 0x4b, 0x8b, 0xb,
   0xf3, 0x73, 0xb3, 0x33, 0xd3, 0x53, 0x93, 0x13,
   0xe3, 0x63, 0xa3, 0x23, 0xc3, 0x43, 0x83, 0x3,
   0xfd, 0x7d, 0xbd, 0x3d, 0xdd, 0x5d, 0x9d, 0x1d,
   0xed, 0x6d, 0xad, 0x2d, 0xcd, 0x4d, 0x8d, 0xd,
   0xf5, 0x75, 0xb5, 0x35, 0xd5, 0x55, 0x95, 0x15,
   0xe5, 0x65, 0xa5, 0x25, 0xc5, 0x45, 0x85, 0x5,
   0xf9, 0x79, 0xb9, 0x39, 0xd9, 0x59, 0x99, 0x19,
   0xe9, 0x69, 0xa9, 0x29, 0xc9, 0x49, 0x89, 0x9,
   0xf1, 0x71, 0xb1, 0x31, 0xd1, 0x51, 0x91, 0x11,
   0xe1, 0x61, 0xa1, 0x21, 0xc1, 0x41, 0x81, 0x1,
   0xfe, 0x7e, 0xbe, 0x3e, 0xde, 0x5e, 0x9e, 0x1e,
   0xee, 0x6e, 0xae, 0x2e, 0xce, 0x4e, 0x8e, 0xe,
   0xf6, 0x76, 0xb6, 0x36, 0xd6, 0x56, 0x96, 0x16,
   0xe6, 0x66, 0xa6, 0x26, 0xc6, 0x46, 0x86, 0x6,
   0xfa, 0x7a, 0xba, 0x3a, 0xda, 0x5a, 0x9a, 0x1a,
   0xea, 0x6a, 0xaa, 0x2a, 0xca, 0x4a, 0x8a, 0xa,
   0xf2, 0x72, 0xb2, 0x32, 0xd2, 0x52, 0x92, 0x12,
   0xe2, 0x62, 0xa2, 0x22, 0xc2, 0x42, 0x82, 0x2,
   0xfc, 0x7c, 0xbc, 0x3c, 0xdc, 0x5c, 0x9c, 0x1c,
   0xec, 0x6c, 0xac, 0x2c, 0xcc, 0x4c, 0x8c, 0xc,
   0xf4, 0x74, 0xb4, 0x34, 0xd4, 0x54, 0x94, 0x14,
   0xe4, 0x64, 0xa4, 0x24, 0xc4, 0x44, 0x84, 0x4,
   0xf8, 0x78, 0xb8, 0x38, 0xd8, 0x58, 0x98, 0x18,
   0xe8, 0x68, 0xa8, 0x28, 0xc8, 0x48, 0x88, 0x8,
   0xf0, 0x70, 0xb0, 0x30, 0xd0, 0x50, 0x90, 0x10,
   0xe0, 0x60, 0xa0, 0x20, 0xc0, 0x40, 0x80, 0x0
};

// Taken from nftytool */
struct usb_device *iuu_get_device(int device);
struct usb_endpoint_descriptor *iuu_get_ep_desc(iuu * inf,
                                                u_int8_t direction);

// Returns the number of WBE's IUUs connected to the 
// USB bus or 0 if none is present
iuu_error iuu_ndevs(int *numdev)
{
   struct usb_bus *bus;
   struct usb_bus *busses;
   struct usb_device *dev;

   usb_init();
   usb_find_busses();
   usb_find_devices();
   busses = usb_get_busses();
   *numdev = 0;

   for (bus = busses; bus; bus = bus->next)
      for (dev = bus->devices; dev; dev = dev->next)
         if ((dev->descriptor.idVendor == IUU_USB_VENDOR_ID) &&
             (dev->descriptor.idProduct == IUU_USB_PRODUCT_ID))
            (*numdev)++;

   return IUU_OPERATION_OK;
}

// Establishes all communication mechanisms with the IUU selected with
// the parameter devnum
iuu_error iuu_start(iuu * inf, int devnum)
{
   usb_init();
   usb_find_busses();
   usb_find_devices();

   inf->dev = iuu_get_device(devnum);
   if (!inf->dev)
      return IUU_DEVICE_NOT_FOUND;

   inf->handle = usb_open(inf->dev);
   if (!inf->handle)
      return IUU_INVALID_HANDLE;

   iuu_error status;
   status = usb_claim_interface(inf->handle, 0);
   if (status != 0)
      return IUU_INVALID_INTERFACE;

   inf->ep_out = iuu_get_ep_desc(inf, USB_ENDPOINT_OUT);
   inf->ep_in = iuu_get_ep_desc(inf, USB_ENDPOINT_IN);

   return IUU_OPERATION_OK;
}

// Releases all established communication with the IUU It is good
// practice to call it before closing your application using IUUs
// since the device will go to a sober state
iuu_error iuu_stop(iuu * inf)
{
   iuu_error status;

   status = usb_release_interface(inf->handle, 0);
   if (status != 0) {
      status = IUU_INVALID_INTERFACE;
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = usb_reset(inf->handle);
   if (status != 0) {
      status = IUU_INVALID_HANDLE;
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = usb_close(inf->handle);
   if (status != 0) {
      status = IUU_INVALID_HANDLE;
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   return IUU_OPERATION_OK;
}

// This is not UART related but IUU USB driver related or something
// like that. Basically no IUU will accept any commands from the USB
// host unless it has received the following message
iuu_error iuu_cts(iuu * inf)
{
   char bmRequestType = 0x03;
   char bRequest = 0x02;
   int wValue = 0x02;
   int wIndex = 0x00;
   char *data = NULL;
   int length = 0x00;
   int timeout = 1000;

   char rt;
   iuu_error status;
// USB bmRequestType 

   const int USB_TYPE_HOST_TO_DEV = (0x00 << 7);
   //const int USB_TYPE_DEV_TO_HOST =    (0x01 << 7);

   rt = 0 | USB_TYPE_HOST_TO_DEV | USB_TYPE_STANDARD |
       USB_RECIP_ENDPOINT;
   //fprintf(stdout, "rt = %d\n", rt);
   //status = usb_control_msg(inf.handle, USB_REQ_SET_FEATURE, 0x02, 0x02, 0x00, NULL, 0x00, 1000);
   //status = usb_control_msg(inf.handle, USB_REQ_SET_FEATURE, rt, 0x02, 0x00, NULL, 0x00, 1000);
   status =
       usb_control_msg(inf->handle, bmRequestType, bRequest, wValue,
                       wIndex, data, length, timeout);
   /*status = usb_control_msg(inf->handle, 0x03, 0x02, 0x02, 0x00, NULL, 0x00, 1000); */
   return status;
}

// Reads/gets a stream of data from the IUU through the USB bus
iuu_error iuu_read(iuu * inf, u_int8_t * buf, int len)
{
   iuu_error status;
   status = usb_bulk_read(inf->handle, inf->ep_in->bEndpointAddress,
                          (char *)buf, len, IUU_USB_OP_TIMEOUT);

   if (status < 0) {
      iuu_process_error(status, __FILE__, __LINE__);
      return IUU_READ_ERROR;
   }

   return IUU_OPERATION_OK;
}

// Writes/sends a stream of data from the IUU through the USB bus
iuu_error iuu_write(iuu * inf, u_int8_t * buf, int len)
{
   iuu_error status;
   status = usb_bulk_write(inf->handle, inf->ep_out->bEndpointAddress,
                           (char *)buf, len, IUU_USB_OP_TIMEOUT);

   if (status < 0) {
      iuu_process_error(status, __FILE__, __LINE__);
      return IUU_WRITE_ERROR;
   }

   return IUU_OPERATION_OK;
}

// Sends a NOP command to the IUU. Doesn't do anything but helps to
// check that messages go through the USB. Use iuu_status() to check
// the opposite direction
iuu_error iuu_nop(iuu * inf)
{
   int status;
   u_int8_t buf = IUU_NO_OPERATION;

   status = iuu_write(inf, &buf, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// Gets the firmware version of the IUU and returns it in ver.  Since
// the firmware version is returned as 4 bytes, then ver must have had
// allocated at least 5 bytes of memory before it's being used as a
// parameter here.
iuu_error iuu_firmware(iuu * inf, char *ver)
{
   int status;

   ver[0] = IUU_GET_FIRMWARE_VERSION;

   status = iuu_write(inf, (unsigned char *)ver, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, (unsigned char *)ver, 4);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   ver[4] = '\0';               // If we do it here, the application won't notice
   return status;
}

// Gets the product name of the IUU and returns it in ver. Since the
// product name is returned as 16 bytes of ascii characters by the IUU
// then name must have allocated at least 17 bytes of memory before
// it's being used as a paramer.
iuu_error iuu_name(iuu * inf, char *name)
{
   int status;
   name[0] = IUU_GET_PRODUCT_NAME;

   status = iuu_write(inf, (unsigned char *)name, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, (unsigned char *)name, 16);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   name[16] = '\0';             // If we do it here, the application won't notice
   //fprintf(stdout, "Retrieved string: %s\n", name);
   return status;
}

// Gets the loader version of the IUU and returns it in ver. Since the
// product name is returned as 4 bytes of ascii characters by the IUU
// then ver must have allocated at least 5 bytes of memory before it's
// being used as a parameter.
iuu_error iuu_loader(iuu * inf, char *ver)
{
   int status;

   ver[0] = IUU_GET_LOADER_VERSION;

   status = iuu_write(inf, (unsigned char *)ver, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, (unsigned char *)ver, 4);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   ver[4] = '\0';               // If we do it here, the application won't notice
   return status;
}

// Gets the state register of the IUU and returns it in st.  Use enum
// iuu_status_register_t to interpret its results.  Use this function
// also if you want to test the receiving capabilities.
iuu_error iuu_status(iuu * inf, u_int8_t * st)
{
   iuu_error status;
   *st = IUU_GET_STATE_REGISTER;

   status = iuu_write(inf, st, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, st, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Sets the IUU led. R, G and B are the intensity values for red,
// green and blue respectively while f is the blinking frequency
// Notice that color intensities and frequency values are NOT
// independent variables. The higher the LED intensity the longer it
// takes to go back to darkness and to achieve the blinking effect.
iuu_error iuu_led(iuu * inf, u_int16_t R, u_int16_t G, u_int16_t B,
                  u_int8_t f)
{
   iuu_error status;
   u_int8_t buf[8];

   buf[0] = IUU_SET_LED;
   buf[1] = R & 0xFF;
   buf[2] = (R >> 8) & 0xFF;
   buf[3] = G & 0xFF;
   buf[4] = (G >> 8) & 0xFF;
   buf[5] = B & 0xFF;
   buf[6] = (B >> 8) & 0xFF;
   buf[7] = f;

   status = iuu_write(inf, buf, 8);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return IUU_OPERATION_OK;
}

// Sets the value for Vcc (a.k.a. ISO7816 card contact C1)
// Use the values described in enum iuu_vcc_t
iuu_error iuu_vcc(iuu * inf, enum iuu_vcc_t vcc)
{
   iuu_error status;
   u_int8_t buf[2];

   buf[0] = IUU_SET_VCC;

   if (vcc == IUU_VCC_5V)
      buf[1] = vcc;
   else if (vcc == IUU_VCC_3V)
      buf[1] = vcc;
   else {
      status = IUU_INVALID_PARAMETER;
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_write(inf, buf, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Sets the frequency of the ISO7816 CLK or Clock signal
// (a.k.a. smart card contact C3)
// Lots of thanks to WBE for provinding the code to do it
iuu_error iuu_clk(iuu * inf, int dwFrq)
{
   /*
      if (!CheckSDKInput (hDevice))
      return SDK_INVALID_HANDLE;

      DWORD dwDevice = ConvertHandleToIndex (hDevice);

      if ((dwFrq < 760000 || dwFrq > 25000000) && dwFrq != 0)
      return SDK_INVALID_PARAMETER;

      SDK_STATUS sdk_status = SDK_SUCCESS;
      DWORD dwWritten;
    */
   int status;
   char WriteBuffer[255];
   int Count = 0;
   unsigned char FrqGenAdr = 0x69;
   unsigned char DIV = 0;       /* 8bit */
   unsigned char XDRV = 0;      /* 8bit */
   /* //0b'110xxxxx' = 0x0C //3 */
   unsigned char PUMP = 0;      /* 3bit */
   unsigned char PBmsb = 0;     /* 2bit */
   unsigned char PBlsb = 0;     /* 8bit */
   unsigned char PO = 0;        /* 1bit */
   unsigned char Q = 0;         /* 7bit */
   /* 24bit = 3bytes */
   unsigned int P = 0;
   unsigned int P2 = 0;
   int frq = (int)dwFrq;

   if (frq == 0) {
      WriteBuffer[Count++] = IUU_UART_WRITE_I2C;        /* //0x4C */
      WriteBuffer[Count++] = FrqGenAdr << 1;
      WriteBuffer[Count++] = 0x09;
      WriteBuffer[Count++] = 0x00;      /* //Adr = 0x09 */
      /*
         EnterCriticalSection (&instances[dwDevice]->IOCriticalSection);
         sdk_status = INFUNLTD_Write (hDevice, WriteBuffer, Count, &dwWritten);
         LeaveCriticalSection (&instances[dwDevice]->IOCriticalSection);
         return sdk_status;
       */
      status = iuu_write(inf, (u_int8_t *) WriteBuffer, Count);
      if (status != 0) {
         iuu_process_error(status, __FILE__, __LINE__);
         return status;
      }
   } else if (frq == 3579000) {
      DIV = 100;
      P = 1193;
      Q = 40;
      XDRV = 0;
   } else if (frq == 3680000) {
      DIV = 105;
      P = 161;
      Q = 5;
      XDRV = 0;
   } else if (frq == 6000000) {
      DIV = 66;
      P = 66;
      Q = 2;
      XDRV = 0x28;
   } else {
      unsigned int result = 0;
      unsigned int tmp = 0;
      unsigned int check;
      unsigned int check2;
      char found = 0x00;

      unsigned int lQ = 2;
      unsigned int lP = 2055;
      unsigned int lDiv = 4;
      /*for (unsigned int lQ = 2; lQ <= 47 && !found; lQ++)
         for (unsigned int lP = 2055; lP >= 8 && !found; lP--)
         for (unsigned int lDiv = 4; lDiv <= 127 && !found; lDiv++) */
      for (lQ = 2; lQ <= 47 && !found; lQ++)
         for (lP = 2055; lP >= 8 && !found; lP--)
            for (lDiv = 4; lDiv <= 127 && !found; lDiv++) {
               /* tmp = ((12000000*lP)/(lQ*lDiv)); */
               tmp = (12000000 / lDiv) * (lP / lQ);
               if (abs((int)(tmp - frq)) < abs((int)(frq - result))) {
                  check2 = (12000000 / lQ);
                  if (check2 < 250000)
                     continue;
                  check = (12000000 / lQ) * lP;
                  if (check > 400000000)
                     continue;
                  if (check < 100000000)
                     continue;
                  if (lDiv < 4 || lDiv > 127)
                     continue;
                  result = tmp;
                  P = lP;
                  DIV = lDiv;
                  Q = lQ;
                  if (result == frq)
                     found = 0x01;      /* true */
               }
            }
   }
   P2 = ((P - PO) / 2) - 4;
   DIV = DIV;
   PUMP = 0x04;
   PBmsb = (P2 >> 8 & 0x03);
   PBlsb = P2 & 0xFF;
   PO = (P >> 10) & 0x01;
   Q = Q - 2;

   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /* 0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x09;
   WriteBuffer[Count++] = 0x20; /* Adr = 0x09 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /* 0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x0C;
   WriteBuffer[Count++] = DIV;  /* Adr = 0x0C */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /* 0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x12;
   WriteBuffer[Count++] = XDRV; /* Adr = 0x12 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x13;
   WriteBuffer[Count++] = 0x6B; /* Adr = 0x13 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x40;
   WriteBuffer[Count++] = (0xC0 | ((PUMP & 0x07) << 2)) | (PBmsb & 0x03);       /* Adr = 0x40 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x41;
   WriteBuffer[Count++] = PBlsb;        /* Adr = 0x41 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x42;
   WriteBuffer[Count++] = Q | (((PO & 0x01) << 7));     /* Adr = 0x42 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x44;
   WriteBuffer[Count++] = (char)0xFF;   /* Adr = 0x44 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x45;
   WriteBuffer[Count++] = (char)0xFE;   /* Adr = 0x45 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x46;
   WriteBuffer[Count++] = 0x7F; /* Adr = 0x46 */
   WriteBuffer[Count++] = IUU_UART_WRITE_I2C;   /*  0x4C */
   WriteBuffer[Count++] = FrqGenAdr << 1;
   WriteBuffer[Count++] = 0x47;
   WriteBuffer[Count++] = (char)0x84;   /* Adr = 0x47 */
   /*
      EnterCriticalSection (&instances[dwDevice]->IOCriticalSection);
      sdk_status = INFUNLTD_Write (hDevice, WriteBuffer, Count, &dwWritten);
      LeaveCriticalSection (&instances[dwDevice]->IOCriticalSection);
    */

   status = iuu_write(inf, (u_int8_t *) WriteBuffer, Count);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Sets the RST signal for a total of wt milliseconds 
//
// According to ISO7816, for asynchronous transmissions, the ATR
// starts being transmited from the card no earlier than 400 etus nor
// later than 40k etus. Since 1 etu is 1/9600 or more accurately
// 372/3k579 one would hold RST active for 12milliseconds (i.e. 40k
// etus) ensuring that the ATR trasmission has started before clearing
// RST. Therefore wt = 0x0C is a safe value
iuu_error iuu_reset(iuu * inf, u_int8_t wt)
{
   iuu_error status;

   status = iuu_uart_flush(inf);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   u_int8_t buf[4];
   buf[0] = IUU_RST_SET;
   buf[1] = IUU_DELAY_MS;
   buf[2] = wt;                 /* miliseconds */
   buf[3] = IUU_RST_CLEAR;

   status = iuu_write(inf, buf, 4);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Initilizes the IUU UART (a.k.a. Phoenix interface) with default
// values 9600E1 (9600 bps, Even parity, 1 stop bits)
iuu_error iuu_uart_on(iuu * inf)
{
   iuu_error status;
   u_int8_t buf[4];

   buf[0] = IUU_UART_ENABLE;
   buf[1] = (u_int8_t) ((IUU_BAUD_9600 >> 8) & 0x00FF); /* high byte */
   buf[2] = (u_int8_t) (0x00FF & IUU_BAUD_9600);        /* low byte */
   buf[3] =
       (u_int8_t) (0x0F0 & IUU_ONE_STOP_BIT) | (0x07 & IUU_PARITY_EVEN);

   status = iuu_write(inf, buf, 4);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_uart_flush(inf);        // iuu_reset() the card after iuu_uart_on()
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Diables the IUU UART (a.k.a. the Phoenix interface)
iuu_error iuu_uart_off(iuu * inf)
{
   iuu_error status;
   u_int8_t buf = IUU_UART_DISABLE;

   status = iuu_write(inf, &buf, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Changes the IUU UART (a.k.a. Phoenix interface) settings.
// Use the variables from the types iuu_uart_baudrate, 
// iuu_uart_parity and iuu_uart_stopbits as function params
iuu_error iuu_uart_set(iuu * inf, iuu_uart_baudrate br,
                       iuu_uart_parity parity, iuu_uart_stopbits sbits)
{
   iuu_error status;
   u_int8_t buf[5];

   buf[0] = IUU_UART_ESC;
   buf[1] = IUU_UART_CHANGE;
   buf[2] = (u_int8_t) ((br >> 8) & 0x00FF);    /* high byte */
   buf[3] = (u_int8_t) (0x00FF & br);   /* low byte */
   buf[4] = (u_int8_t) (parity | sbits);        /* both parity and stop now */

   status = iuu_write(inf, buf, 5);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// The equivalent to INFUNLTD_Phoenix_ChangeCustomBaud()
// Sets a non standard baud rate and returns the closest
// baud rate actually set
iuu_error iuu_uart_baud(iuu * inf, u_int32_t baud, u_int32_t * actual,
                        iuu_uart_parity parity)
{

   //SDK_STATUS sdk_status = SDK_SUCCESS;
   //unsigned char dataout[10];
   //DWORD dwWritten = 0;
   //unsigned int DataCount = 0;

   iuu_error status;
   u_int8_t dataout[5];
   u_int8_t DataCount = 0;

   if (baud < 1200 || baud > 230400)
      return IUU_INVALID_PARAMETER;

   unsigned char T1Frekvens = 0;
   unsigned char T1reload = 0;
   unsigned int T1FrekvensHZ = 0;

   if (baud > 977) {
      T1Frekvens = 3;
      T1FrekvensHZ = 500000;
   }

   if (baud > 3906) {
      T1Frekvens = 2;
      T1FrekvensHZ = 2000000;
   }

   if (baud > 11718) {
      T1Frekvens = 1;
      T1FrekvensHZ = 6000000;
   }

   if (baud > 46875) {
      T1Frekvens = 0;
      T1FrekvensHZ = 24000000;
   }

   T1reload =
       256 -
       (unsigned
        char)(((float)
               ((float)T1FrekvensHZ /
                (float)((float)2.0 * (float)baud))));

   dataout[DataCount++] = IUU_UART_ESC; // magic number here:  ENTER_FIRMWARE_UPDATE;
   dataout[DataCount++] = IUU_UART_CHANGE;      // magic number here:  CHANGE_BAUD; 
   dataout[DataCount++] = T1Frekvens;
   dataout[DataCount++] = T1reload;

   /* *actual = (unsigned int)(((float)T1FrekvensHZ / (float)((float)256 - (float)T1reload)) / (float)2.0); */
   *actual =
       (u_int32_t) (((float)T1FrekvensHZ /
                     (float)((float)256 -
                             (float)T1reload)) / (float)2.0);

   switch (parity & 0x0F) {
   case IUU_PARITY_NONE:
      dataout[DataCount++] = 0x00;
      break;
   case IUU_PARITY_EVEN:
      dataout[DataCount++] = 0x01;
      break;
   case IUU_PARITY_ODD:
      dataout[DataCount++] = 0x02;
      break;
   case IUU_PARITY_MARK:
      dataout[DataCount++] = 0x03;
      break;
   case IUU_PARITY_SPACE:
      dataout[DataCount++] = 0x04;
      break;
   default:
      return IUU_INVALID_PARAMETER;
      break;
   }

   switch (parity & 0xF0) {
   case IUU_ONE_STOP_BIT:
      dataout[DataCount - 1] |= IUU_ONE_STOP_BIT;
      break;

   case IUU_TWO_STOP_BITS:
      dataout[DataCount - 1] |= IUU_TWO_STOP_BITS;
      break;
   default:
      return IUU_INVALID_PARAMETER;
      break;
   }

   //sdk_status = INFUNLTD_Write(hDevice,dataout,DataCount,&dwWritten);
   status = iuu_write(inf, dataout, DataCount);
   //if(dwWritten != DataCount) 
   //return SDK_INVALID_HANDLE;
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Reads len bytes from the IUU UART fifo and copies them
// starting from the address addr.
// Beware! addr must point a to a previously allocated block
// o memory
iuu_error iuu_uart_rx(iuu * inf, u_int8_t * addr, u_int8_t * len)
{
   iuu_error status;
   u_int8_t rxcmd = IUU_UART_RX;

   status = iuu_write(inf, &rxcmd, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, len, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, addr, *len);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Transmits len bytes of data starting from address addr.
//
// The official SDK from WBE allows you to send more than 255 bytes by
// splitting the chunk in smaller bits. 255 bytes in the maximum
// payload size of the USB mesasges between the USB host and the USB
// device supported by the iuu. The iuu uart connection also allows
// you to read back the bytes you sent through the phoenix interface.
iuu_error iuu_uart_tx(iuu * inf, u_int8_t * addr, u_int8_t len)
{
   iuu_error status;
   u_int8_t *buf;

   buf = (u_int8_t *) calloc(len + 3, sizeof(u_int8_t));
   buf[0] = IUU_UART_ESC;
   buf[1] = IUU_UART_TX;
   buf[2] = len;
   memcpy(&buf[3], addr, len);

   status = iuu_write(inf, buf, len + 3);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   free(buf);

   return status;
}

// Squeezes in a number of NOP operations between the bytes sent to
// the phoenix interface. Since the iuu firmware does not provide
// specific means to deal with the EGT bit (the minimum time between
// bytes one must wait until the inserted card has fully accepted the
// last byte sent) this is a workaround. The EGT is stated in the TC1
// byte of the card ATR and is given in etus.
iuu_error iuu_uart_txnops(iuu * inf, u_int8_t * data, u_int8_t len,
                          u_int8_t nops)
{
   iuu_error status;
   u_int8_t *buf;
   int i, K, L;

   K = 4 + nops;                // 4 bytes for each one to the uart plus nops bytes
   L = len * K;
   buf = (u_int8_t *) calloc(L, sizeof(u_int8_t));

   for (i = 0; i < len; i++) {
      buf[i * K + 0] = IUU_UART_ESC;
      buf[i * K + 1] = IUU_UART_TX;
      buf[i * K + 2] = 0x01;
      buf[i * K + 3] = data[i];
      memset(&buf[i * K + 4], IUU_NO_OPERATION, nops);
   }

   status = iuu_write(inf, buf, L);

   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   free(buf);
   return status;
}

// Squeezes in a number of "wait ms milliseconds" operations between
// the bytes sent to the phoenix interface. Since the iuu firmware
// does not provide specific means to deal with the EGT bit (the
// minimum time between bytes one must wait until the inserted card
// has fully accepted the last byte sent) this is a workaround that
// actually uses the firmware capabilities. The EGT is stated in the
// TC1 byte of the card ATR and is given in etus.
iuu_error iuu_uart_txm(iuu * inf, u_int8_t * data, u_int8_t len,
                       u_int8_t ms)
{
   iuu_error status;
   u_int8_t *buf;
   int i, K, L;

   K = 6;                       // 4 bytes for each one to the uart plus 2 for each wait cmd
   L = len * K;
   buf = (u_int8_t *) calloc(L, sizeof(u_int8_t));

   for (i = 0; i < len; i++) {
      buf[i * K + 0] = IUU_UART_ESC;
      buf[i * K + 1] = IUU_UART_TX;
      buf[i * K + 2] = 0x01;
      buf[i * K + 3] = data[i];
      buf[i * K + 4] = IUU_WAIT_MS;
      buf[i * K + 5] = ms;
   }

   status = iuu_write(inf, buf, L);

   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   free(buf);
   return status;
}

// Squeezes in a number of "wait 10*microseconds" operations between
// the bytes sent to the phoenix interface. Since the iuu firmware
// does not provide specific means to deal with the EGT bit (the
// minimum time between bytes one must wait until the inserted card
// has fully accepted the last byte sent) this is a workaround that
// actually uses the firmware capabilities. The EGT is stated in the
// TC1 byte of the card ATR and is given in etus.
iuu_error iuu_uart_txmu(iuu * inf, u_int8_t * data, u_int8_t len,
                        u_int8_t mus)
{
   iuu_error status;
   u_int8_t *buf;
   int i, K, L;

   K = 6;                       // 4 bytes for each one to the uart plus 2 for each wait cmd
   L = len * K;
   buf = (u_int8_t *) calloc(L, sizeof(u_int8_t));

   for (i = 0; i < len; i++) {
      buf[i * K + 0] = IUU_UART_ESC;
      buf[i * K + 1] = IUU_UART_TX;
      buf[i * K + 2] = 0x01;
      buf[i * K + 3] = data[i];
      buf[i * K + 4] = IUU_WAIT_MUS;
      buf[i * K + 5] = mus;     /* 10 times mus actually */
   }

   status = iuu_write(inf, buf, L);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   free(buf);
   return status;
}

// Toggles the RST signal (a.k.a. ISO7816 C2 connector), waits wt*10
// milliseconds and sends the value of cmd through the I/O line (this
// command is really fishy first of all because the resolution will
// most likely depend on the CLK frequency)
// PENDING: Not tested. Is this function of any use?
iuu_error iuu_uart_trap(iuu * inf, u_int8_t wt, u_int8_t cmd)
{
   u_int8_t buf[3];

   buf[0] = IUU_UART_TRAP;
   buf[1] = wt;                 // waiting time
   buf[2] = cmd;                // command byte

   iuu_error status;
   status = iuu_write(inf, buf, 3);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Sets the I/0 signal (a.k.a. ISO7816 C7) signal low, toggles the RST
// signal (a.k.a. ISO7816 C2 connector), waits wt*10 milliseconds and
// sends the value of cmd through the I/O line (this command is really
// fishy again because the resolution will most likely depend on the
// CLK frequency)
// PENDING: Not tested. Is this function of any use?
iuu_error iuu_uart_break(iuu * inf, u_int8_t wt, u_int8_t cmd)
{
   iuu_error status;
   u_int8_t buf[3];

   buf[0] = IUU_UART_TRAP_BREAK;
   buf[1] = wt;                 // waiting time
   buf[2] = cmd;                // command byte

   status = iuu_write(inf, buf, 3);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// The guys at WBE do it twice, but I think it is an overkill
iuu_error iuu_uart_flush(iuu * inf)
{
   int i;
   u_int8_t datalen = 0;
   iuu_error status;
   u_int8_t rxcmd = IUU_UART_RX;
   u_int8_t datain[256];

   for (i = 0; i < 2; i++) {
      status = iuu_write(inf, &rxcmd, 1);
      if (status != IUU_OPERATION_OK) {
         iuu_process_error(status, __FILE__, __LINE__);
         return status;
      }

      status = iuu_read(inf, &datalen, 1);
      if (status != IUU_OPERATION_OK) {
         iuu_process_error(status, __FILE__, __LINE__);
         return status;
      }

      if (datalen > 0) {
         status = iuu_read(inf, datain, datalen);
         if (status != IUU_OPERATION_OK) {
            iuu_process_error(status, __FILE__, __LINE__);
            return status;
         }
      }
   }
   return status;
}

// EEPROM through device related commands

// Power on
iuu_error iuu_eeprom_on(iuu * inf)
{
   int status;
   u_int8_t buf = IUU_EEPROM_ON;

   status = iuu_write(inf, &buf, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// Power off
iuu_error iuu_eeprom_off(iuu * inf)
{
   int status;
   u_int8_t buf = IUU_EEPROM_OFF;

   status = iuu_write(inf, &buf, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// write byte, 8 bit address
iuu_error iuu_eeprom_write(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                           u_int8_t data)
{

   int status;
   u_int8_t buf[4];

   buf[0] = IUU_EEPROM_WRITE;
   buf[1] = ctrl;
   buf[2] = addr;
   buf[3] = data;

   status = iuu_write(inf, buf, 4);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// write byte, 16 bit address
iuu_error iuu_eeprom_writex(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                            u_int8_t data)
{

   int status;
   u_int8_t buf[5];

   buf[0] = IUU_EEPROM_WRITEX;
   buf[1] = ctrl;
   buf[2] = (u_int8_t) (addr & 0x00FF);
   buf[3] = (u_int8_t) ((addr >> 8) & 0xFF00);
   buf[4] = data;

   status = iuu_write(inf, buf, 5);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// write 8 bytes page, 8 bit address
iuu_error iuu_eeprom_write8(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                            u_int8_t * data)
{
   int status;
   u_int8_t buf[11];

   buf[0] = IUU_EEPROM_WRITE8;
   buf[1] = ctrl;
   buf[2] = addr;
   memcpy(&buf[3], data, 8);

   status = iuu_write(inf, buf, 11);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// write 16 bytes page, 8 bit address
iuu_error iuu_eeprom_write16(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                             u_int8_t * data)
{

   int status;
   u_int8_t buf[19];

   buf[0] = IUU_EEPROM_WRITE16;
   buf[1] = ctrl;
   buf[2] = addr;
   memcpy(&buf[3], data, 16);

   status = iuu_write(inf, buf, 19);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// write 32 bytes page, 16 bits address
iuu_error iuu_eeprom_writex32(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                              u_int8_t * data)
{

   int status;
   u_int8_t buf[36];

   buf[0] = IUU_EEPROM_WRITEX32;
   buf[1] = ctrl;
   buf[2] = (u_int8_t) (addr & 0x00FF);
   buf[3] = (u_int8_t) ((addr >> 8) & 0xFF00);
   memcpy(&buf[4], data, 32);

   status = iuu_write(inf, buf, 36);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

//write 64 bytes page, 16 bits address
iuu_error iuu_eeprom_writex64(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                              u_int8_t * data)
{
   int status;
   u_int8_t buf[68];

   buf[0] = IUU_EEPROM_WRITEX64;
   buf[1] = ctrl;
   buf[2] = (u_int8_t) (addr & 0x00FF);
   buf[3] = (u_int8_t) ((addr >> 8) & 0xFF00);
   memcpy(&buf[4], data, 64);

   status = iuu_write(inf, buf, 68);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// read 1 byte, inc 8bit address
iuu_error iuu_eeprom_read(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                          u_int8_t * data)
{
   int status;
   u_int8_t buf[3];

   buf[0] = IUU_EEPROM_READ;
   buf[1] = ctrl;
   buf[2] = addr;

   status = iuu_write(inf, buf, 3);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// read 1 byte, inc 16bit address
iuu_error iuu_eeprom_readx(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                           u_int8_t * data)
{
   int status;
   u_int8_t buf[4];

   buf[0] = IUU_EEPROM_READ;
   buf[1] = ctrl;
   buf[2] = (u_int8_t) (addr & 0x00FF);
   buf[3] = (u_int8_t) ((addr >> 8) & 0xFF00);

   status = iuu_write(inf, buf, 4);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// read n bytes, inc 8bit address
iuu_error iuu_eeprom_bread(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                           u_int8_t n, u_int8_t * data)
{
   int status;
   unsigned char buf[4];

   buf[0] = IUU_EEPROM_BREAD;
   buf[1] = ctrl;
   buf[2] = addr;
   buf[3] = n;

   status = iuu_write(inf, buf, 4);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, n);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;

}

// read n bytes, inc 16bit address
iuu_error iuu_eeprom_breadx(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                            u_int8_t n, u_int8_t * data)
{
   int status;
   unsigned char buf[5];

   buf[0] = IUU_EEPROM_BREADX;
   buf[1] = ctrl;
   buf[2] = (u_int8_t) (0x00FF & addr);
   buf[3] = (u_int8_t) ((addr >> 8) & 0x00FF);
   buf[4] = n;

   status = iuu_write(inf, buf, 5);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, n);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// turns power supply on
iuu_error iuu_avr_on(iuu * inf)
{
   int status;
   u_int8_t cmd = IUU_AVR_ON;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// turns power supply on
iuu_error iuu_avr_off(iuu * inf)
{
   int status;
   u_int8_t cmd = IUU_AVR_OFF;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// makes one single pulse on SCK
iuu_error iuu_avr_1clk(iuu * inf)
{
   int status;
   u_int8_t cmd = IUU_AVR_1CLK;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// sets reset high for 100 usec and the turns it low
// (implemented in the iuu firmware, obviously)
iuu_error iuu_avr_reset(iuu * inf)
{
   int status;
   u_int8_t cmd = IUU_AVR_RESET;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// resets the internal program counter
iuu_error iuu_avr_resetpc(iuu * inf)
{
   int status;
   u_int8_t cmd = IUU_AVR_RESET_PC;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// increments the internal program counter by 1
iuu_error iuu_avr_inc(iuu * inf)
{
   int status;
   u_int8_t cmd = IUU_AVR_INC_PC;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// increments the internal program counter by len
iuu_error iuu_avr_incn(iuu * inf, u_int8_t n)
{
   int status;
   u_int8_t cmd[2];
   cmd[0] = IUU_AVR_INCN_PC;
   cmd[1] = n;

   status = iuu_write(inf, cmd, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// reads a word from prog mem to data
// increments the internal program counter by 1
iuu_error iuu_avr_pread(iuu * inf, u_int8_t * data)
{
   int status;
   u_int8_t cmd = IUU_AVR_PREAD;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// reads n words from prog mem to data
// increments the internal program counter by len
iuu_error iuu_avr_preadn(iuu * inf, u_int8_t * data, unsigned char n)
{
   int status;
   u_int8_t cmd[2];
   cmd[0] = IUU_AVR_PREADN;
   cmd[1] = n;

   status = iuu_write(inf, cmd, 2);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 2 * n);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// writes a word from data to prog mem
// increments the internal program counter by 1
iuu_error iuu_avr_pwrite(iuu * inf, u_int8_t * data)
{
   int status;
   u_int8_t buf[3];
   buf[0] = IUU_AVR_PWRITE;
   buf[1] = data[0];
   buf[2] = data[1];

   status = iuu_write(inf, buf, 3);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// writes len words from data to prog mem
// increments the internal program counter by len
// PENDING: PLEASE DO REVIEW THIS CODE!!!
iuu_error iuu_avr_pwriten(iuu * inf, u_int8_t * data, u_int8_t len)
{
   int status;
   unsigned char *buf;

   buf = (unsigned char *)malloc(len * 2) + 1;
   if (!buf) {
      iuu_process_error(-1, __FILE__, __LINE__);
      return -1;
   }

   buf[0] = IUU_AVR_PWRITEN;
   memcpy(buf + 1, data, len * 2);

   status = iuu_write(inf, buf, len * 2 + 1);

   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   free(buf);
   return status;
}

// reads a byte from data mem to data
// increments the internal program counter by 1
iuu_error iuu_avr_dread(iuu * inf, u_int8_t * data)
{
   int status;
   u_int8_t cmd = IUU_AVR_DREAD;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// reads len bytes from data mem to buffer data
// increments the internal program counter by len
iuu_error iuu_avr_dreadn(iuu * inf, u_int8_t * data, u_int8_t len)
{
   int status;
   u_int8_t cmd[2];

   cmd[0] = IUU_AVR_DREADN;
   cmd[1] = len;

   status = iuu_write(inf, cmd, 2);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, len);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// writes data to data mem
// increments the internal program counter by 1
iuu_error iuu_avr_dwrite(iuu * inf, u_int8_t data)
{
   int status;
   u_int8_t buf[2];
   buf[0] = IUU_AVR_DWRITE;
   buf[1] = data;

   status = iuu_write(inf, buf, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

iuu_error iuu_pic_cmd(iuu * inf, u_int8_t cmd)
{
   int status;
   u_int8_t buf[2];
   buf[0] = IUU_PIC_CMD;
   buf[1] = cmd;

   status = iuu_write(inf, buf, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

iuu_error iuu_pic_cmd_load(iuu * inf, u_int8_t cmd, u_int8_t * data)
{
   int status;
   u_int8_t buf[4];
   buf[0] = IUU_PIC_CMD_LOAD;
   buf[1] = cmd;
   buf[2] = data[0];
   buf[3] = data[1];

   status = iuu_write(inf, buf, 4);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

iuu_error iuu_pic_cmd_read(iuu * inf, u_int8_t data, u_int8_t * resp)
{
   int status;
   u_int8_t buf[2];
   buf[0] = IUU_PIC_CMD_READ;
   buf[1] = data;

   status = iuu_write(inf, buf, 2);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, resp, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

iuu_error iuu_pic_on(iuu * inf)
{
   int status;
   u_int8_t buf = IUU_PIC_ON;

   status = iuu_write(inf, &buf, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

iuu_error iuu_pic_off(iuu * inf)
{
   int status;
   u_int8_t buf = IUU_PIC_OFF;

   status = iuu_write(inf, &buf, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

iuu_error iuu_pic_reset(iuu * inf)
{
   int status;
   u_int8_t buf = IUU_PIC_RESET;

   status = iuu_write(inf, &buf, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// increments the program counter by 1
iuu_error iuu_pic_inc(iuu * inf)
{
   int status;
   u_int8_t cmd = IUU_PIC_INC_PC;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// increments the program counter by len
iuu_error iuu_pic_incn(iuu * inf, u_int8_t n)
{
   int status;
   u_int8_t cmd[2];
   cmd[0] = IUU_PIC_INCN_PC;
   cmd[1] = n;

   status = iuu_write(inf, cmd, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);
   return status;
}

// writes a word to prog mem
// increases program counter with 1
iuu_error iuu_pic_pwrite(iuu * inf, u_int8_t * data)
{
   int status;
   u_int8_t buf[3];
   buf[0] = IUU_PIC_PWRITE;
   buf[1] = data[0];
   buf[2] = data[1];

   status = iuu_write(inf, buf, 3);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// reads a word from prog mem
// prog counter is increased by 1
iuu_error iuu_pic_pread(iuu * inf, u_int8_t * data)
{
   int status;
   u_int8_t cmd = IUU_PIC_PREAD;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// reads len words from prog mem to data
// prog counter is increased by len
iuu_error iuu_pic_preadn(iuu * inf, u_int8_t * data, unsigned char n)
{
   int status;
   u_int8_t cmd[2];
   cmd[0] = IUU_PIC_PREADN;
   cmd[1] = n;

   status = iuu_write(inf, cmd, 2);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 2 * n);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// writes a word to pic data mem
// prog counter is increased by 1
iuu_error iuu_pic_dwrite(iuu * inf, u_int8_t * data)
{
   int status;
   u_int8_t buf[3];
   buf[0] = IUU_PIC_DWRITE;
   buf[1] = data[0];
   buf[2] = data[1];

   status = iuu_write(inf, buf, 3);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// writes a word to pic data mem
// prog counter is increased by 1
iuu_error iuu_pic_dread(iuu * inf, u_int8_t * data)
{
   int status;
   u_int8_t cmd = IUU_PIC_DREAD;

   status = iuu_write(inf, &cmd, 1);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return status;
   }

   status = iuu_read(inf, data, 2);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   return status;
}

// Retrieves the ATR from a card that actually supports it.  atr must
// have had 88 bytes allocated which equals to the maximum length of
// an Answer To Reset.
iuu_error iuu_get_atr(iuu * inf, u_int8_t * atr, u_int8_t * len)
{
   int i;
   unsigned char tmp;
   iuu_error status;

   status = iuu_uart_rx(inf, atr, len);
   if (status != IUU_OPERATION_OK)
      iuu_process_error(status, __FILE__, __LINE__);

   atr[*len] = '\0';

   // If the card uses Inverse Convention, gotta recode the bytes
   if (atr[0] == 0x03) {
      for (i = 0; i < *len; i++) {
         tmp = inverse2direct[atr[i]];
         atr[i] = tmp;
      }
   }

   return status;
}

// Prints the card ATR as two hex digits per byte
void iuu_print_atr(u_int8_t * atr, u_int8_t atrl)
{
   if (atrl == 0) {
      fprintf(stdout, "No ATR\n");
   } else {
      u_int8_t i, msb, lsb;

      fprintf(stdout, "ATR: ");
      for (i = 0; i < atrl; i++) {
         msb = (0x0F & (atr[i] >> 4));
         lsb = (0x0F & atr[i]);
         fprintf(stdout, "%x", msb);
         fprintf(stdout, "%x ", lsb);
      }
   }
   fprintf(stdout, "\n");
}

// What did it happen and where
void iuu_process_error(iuu_error err, char *filename, int linenumber)
{
   fprintf(stderr, "Error %d in %s:%d\n", err, filename, linenumber);
   fflush(stderr);
}

// Returns a USB handle for device number devn
struct usb_device *iuu_get_device(int devn)
{
   struct usb_device *dev;
   struct usb_bus *bus;

   for (bus = usb_get_busses(); bus; bus = bus->next) {
      for (dev = bus->devices; dev; dev = dev->next) {
         if (dev->descriptor.idVendor == IUU_USB_VENDOR_ID &&
             dev->descriptor.idProduct == IUU_USB_PRODUCT_ID) {
            /*fprintf(stdout, "Found one!: %d\n", devnum); */
            if (!devn) {
               /*fprintf(stdout, "Returning the one!\n"); */
               return dev;
            } else
               devn--;
         }
      }
   }
   return NULL;
}

struct usb_endpoint_descriptor *iuu_get_ep_desc(iuu * inf, u_int8_t dir)
{
   int i;
   struct usb_interface_descriptor *interface;
   struct usb_endpoint_descriptor *ep;

   interface = &inf->dev->config->interface->altsetting[0];

   for (i = 0; i < interface->bNumEndpoints; ++i) {
      ep = &interface->endpoint[i];
      if (dir) {
         if ((ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) &&
             (ep->bEndpointAddress & USB_ENDPOINT_TYPE_BULK)) {
            return ep;
         }
      } else {
         if (!(ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) &&
             (ep->bEndpointAddress & USB_ENDPOINT_TYPE_BULK)) {
            return ep;
         }
      }
   }
   return NULL;
}
