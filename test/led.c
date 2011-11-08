/* 
 *  led.c - Fooling around with the on-board LED
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

#include <stdio.h>
#include <unistd.h>
#include <usb.h>

int usb_debug = 0;

#include <iuu.h>

int main(int argc, char **argv)
{

   int i;
   int ndev;
   u_int8_t status;

   status = iuu_ndevs(&ndev);

   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   } else if (ndev < 1) {
      fprintf(stdout, "No IUU devices found\n");
      return -1;
   }
   fprintf(stdout, "Number of IUU devices found: %d\n", ndev);

   //struct usb_infinity inf;
   iuu *inf;
   inf = (iuu *) calloc(1, sizeof(iuu));
   status = iuu_start(inf, ndev - 1);

   status = iuu_cts(inf);
   if (status != 0) {
      iuu_process_error(status, __FILE__, __LINE__);
      fprintf(stdout, "Unable to CTS IUU\n");
      fprintf(stdout,
              "It could because of unsufficient USB writting permissions.\n");
      return -1;
   }

   char prodname[17];
   status = iuu_name(inf, prodname);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }
   fprintf(stdout, "Product name : %s\n", prodname);

   char firmver[5];
   status = iuu_firmware(inf, firmver);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }
   fprintf(stdout, "IUU firmware version : %s\n", firmver);

   char loaderver[5];
   status = iuu_loader(inf, loaderver);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }
   fprintf(stdout, "Loader version : %s\n", loaderver);

   u_int8_t statreg;
   status = iuu_status(inf, &statreg);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }

   if (statreg & IUU_FULLCARD_IN)
      fprintf(stdout, "Card inserted in the smart card slot\n");
   if (statreg & IUU_MINICARD_IN)
      fprintf(stdout, "Card inserted in the mini SIM slot\n");
   if (!(statreg & (IUU_FULLCARD_IN | IUU_MINICARD_IN))) {
      fprintf(stdout, "No cards inserted\n");
      status = iuu_stop(inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return 0;
   }

   fprintf(stdout, "LED quick test... ");
   iuu_led(inf, 0xF000, 0x0000, 0x0000, 0xFF);
   sleep(1);
   iuu_led(inf, 0x000F, 0x0000, 0x0000, 0xFF);
   sleep(1);
   iuu_led(inf, 0x0000, 0xF000, 0x0000, 0xFF);
   sleep(1);
   iuu_led(inf, 0x0000, 0x000F, 0x0000, 0xFF);
   sleep(1);
   iuu_led(inf, 0x0000, 0x0000, 0xF000, 0xFF);
   sleep(1);
   iuu_led(inf, 0x0000, 0x0000, 0x000F, 0xFF);
   sleep(1);
   fprintf(stdout, "done!\n");

   fprintf(stdout, "LED long test...\n");
   iuu_led(inf, 0x0000, 0x0000, 0x0000, 0xFF);
   for (i = 0x0000; i < 0x4000; i = i + 0x0010)
      iuu_led(inf, i, 0x0000, 0x0000, 0xFF);
   for (i = 0x4000; i > 0x0000; i = i - 0x0010)
      iuu_led(inf, i, 0x0000, 0x0000, 0xFF);
   for (i = 0x0000; i < 0x4000; i = i + 0x0010)
      iuu_led(inf, 0x0000, i, 0x0000, 0xFF);
   for (i = 0x4000; i > 0x0000; i = i - 0x0010)
      iuu_led(inf, 0x0000, i, 0x0000, 0xFF);
   for (i = 0x0000; i < 0x4000; i = i + 0x0010)
      iuu_led(inf, 0x0000, 0x0000, i, 0xFF);
   for (i = 0x4000; i > 0x0000; i = i - 0x0010)
      iuu_led(inf, 0x0000, 0x0000, i, 0xFF);
   for (i = 0x0000; i < 0x4000; i = i + 0x0010)
      iuu_led(inf, i, i, 0x0000, 0xFF);
   for (i = 0x4000; i > 0x0000; i = i - 0x0010)
      iuu_led(inf, i, i, 0x0000, 0xFF);
   for (i = 0x0000; i < 0x4000; i = i + 0x0010)
      iuu_led(inf, i, 0x0000, i, 0xFF);
   for (i = 0x4000; i > 0x0000; i = i - 0x0010)
      iuu_led(inf, i, 0x0000, i, 0xFF);
   for (i = 0x0000; i < 0x4000; i = i + 0x0010)
      iuu_led(inf, 0x0000, i, i, 0xFF);
   for (i = 0x4000; i > 0x0000; i = i - 0x0010)
      iuu_led(inf, 0x0000, i, i, 0xFF);

   iuu_stop(inf);
   free(inf);

   return 0;
}
