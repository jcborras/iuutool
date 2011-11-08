/* 
 *  atr.c - Getting a card's ATR
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
   u_int8_t status;
   int ndev;
   status = iuu_ndevs(&ndev);

   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   } else if (ndev < 1) {
      fprintf(stdout, "No IUU devices found\n");
      return -1;
   }
   fprintf(stdout, "Number of IUU devices found: %d\n", ndev);

   struct usb_infinity inf;
   status = iuu_start(&inf, ndev - 1);

   status = iuu_cts(&inf);
   if (status != 0) {
      iuu_process_error(status, __FILE__, __LINE__);
      fprintf(stdout, "Unable to CTS IUU.\n");
      fprintf(stdout,
              "It could because of unsufficient USB writting permissions.\n");
      return -1;
   }

   char prodname[17];
   status = iuu_name(&inf, prodname);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }
   fprintf(stdout, "Product name : %s\n", prodname);

   char firmver[5];
   status = iuu_firmware(&inf, firmver);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }
   fprintf(stdout, "IUU firmware version : %s\n", firmver);

   char loaderver[5];
   status = iuu_loader(&inf, loaderver);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }
   fprintf(stdout, "Loader version : %s\n", loaderver);

   fprintf(stdout, "Setting the LED");
   status = iuu_led(&inf, 0x0000, 0x1000, 0x0000, 0x80);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }

   fprintf(stdout, "\nSetting CLK");
   status = iuu_clk(&inf, IUU_CLK_3579000);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }

   fprintf(stdout, "\nEnabling UART");
   status = iuu_uart_on(&inf);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }

   fprintf(stdout, "\nYou've got 10 seconds to insert the card");
   fflush(stdout);
   sleep(10);

   u_int8_t statreg;
   status = iuu_status(&inf, &statreg);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }

   if (statreg & IUU_FULLCARD_IN)
      fprintf(stdout, "Card inserted in the smart card slot\n");
   if (statreg & IUU_MINICARD_IN)
      fprintf(stdout, "Card inserted in the mini SIM slot\n");
   if (!(statreg & (IUU_FULLCARD_IN | IUU_MINICARD_IN))) {
      fprintf(stdout, "No cards inserted\n");
      status = iuu_stop(&inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return 0;
   }

   int i;
   for (i = 0; i < 5; i++) {
      fprintf(stdout, "\nReseting card");
      status = iuu_reset(&inf, 0x0C);
      if (status != IUU_OPERATION_OK) {
         iuu_process_error(status, __FILE__, __LINE__);
         status = iuu_stop(&inf);
         iuu_process_error(status, __FILE__, __LINE__);
         return -1;
      }

      usleep(500000);
      //sleep(4);

      fprintf(stdout, "\nGetting the card ATR");
      u_int8_t atrl, atr[300];
      status = iuu_get_atr(&inf, atr, &atrl);
      if (status != IUU_OPERATION_OK) {
         iuu_process_error(status, __FILE__, __LINE__);
         status = iuu_stop(&inf);
         iuu_process_error(status, __FILE__, __LINE__);
         return -1;
      }
      fprintf(stdout, "\n");
      iuu_print_atr(atr, atrl);
   }

   status = iuu_stop(&inf);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
   }

   return 0;
}
