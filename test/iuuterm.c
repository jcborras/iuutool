/* 
 *  iuuterm - a terminal for communication with the WBE's Infinity 
 *  USB Unlimited SDK
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

#include <ctype.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <usb.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <iuu.h>

#define CHECKPOINT fprintf(stdout, "%s:%d Checkpoint\n", __FILE__, __LINE__); fflush(stdout);

char hex2dec(char in)
{
   switch (tolower(in)) {
   case '0':
      return 0x00;
      break;
   case '1':
      return 0x01;
      break;
   case '2':
      return 0x02;
      break;
   case '3':
      return 0x03;
      break;
   case '4':
      return 0x04;
      break;
   case '5':
      return 0x05;
      break;
   case '6':
      return 0x06;
      break;
   case '7':
      return 0x07;
      break;
   case '8':
      return 0x08;
      break;
   case '9':
      return 0x09;
      break;
   case 'a':
      return 0x0a;
      break;
   case 'b':
      return 0x0b;
      break;
   case 'c':
      return 0x0c;
      break;
   case 'd':
      return 0x0d;
      break;
   case 'e':
      return 0x0e;
      break;
   case 'f':
      return 0x0f;
      break;
   default:
      fprintf(stdout, "Bailing out at %s:%d\n", __FILE__, __LINE__);
   }
   return 0;
}

char dec2hex(u_int8_t in)
{

   if (in > 0x0f) {
      fprintf(stdout,
              "%s:%d Input parameter (%d) is bigger than 0x0F\n",
              __FILE__, __LINE__, in);
   }

   switch (in) {
   case '0':
      return 0x00;
      break;
   case 0x00:
      return '0';
      break;
   case 0x01:
      return '1';
      break;
   case 0x02:
      return '2';
      break;
   case 0x03:
      return '3';
      break;
   case 0x04:
      return '4';
      break;
   case 0x05:
      return '5';
      break;
   case 0x06:
      return '6';
      break;
   case 0x07:
      return '7';
      break;
   case 0x08:
      return '8';
      break;
   case 0x09:
      return '9';
      break;
   case 0x0a:
      return 'A';
      break;
   case 0x0b:
      return 'B';
      break;
   case 0x0c:
      return 'C';
      break;
   case 0x0d:
      return 'D';
      break;
   case 0x0e:
      return 'E';
      break;
   case 0x0f:
      return 'F';
      break;
   default:
      fprintf(stdout, "Bailing out at %s:%d\n", __FILE__, __LINE__);
   }
   return 0;
}

// returns a byte out of two xdigits
char hex2bin(char in1, char in2)
{

   char un = hex2dec(in1);      // upper nibble
   un = (un << 4);

   char ln = hex2dec(in2);      // lower nibble

   return (un | ln);
}

// removes everything but xdigits
char *xdigit_only(char *in)
{

   //fprintf(stdout, "%s:%d xdigit(in) : %s\n" , __FILE__, __LINE__, in); fflush(stdout);

   int i, j = 0;
   for (i = 0; i < strlen(in); i++) {
      if (isxdigit(in[i]))
         j++;
   }

   char *out = calloc(j + 1, sizeof(char));

   j = 0;
   for (i = 0; i < strlen(in); i++) {
      if (isxdigit(in[i])) {
         out[j] = in[i];
         j++;
      }
   }

   //fprintf(stdout, "%s:%d xdigit(out) : %s\n" , __FILE__, __LINE__, out); fflush(stdout);

   return out;
}

// returns a byte stream out of pairs of xdigits
char *xdigit2bin(char *in, int *L)
{

   //fprintf(stdout, "%s:%d xdigit(in) : %s\n" , __FILE__, __LINE__, in); fflush(stdout);

   if ((strlen(in) % 2) == 0)
      fprintf(stdout, "Even (%d) number of xdigits. We are cool.\n",
              strlen(in));
   else {
      fprintf(stdout,
              "Odd (%d) number of xdigits. Bailing out at %s:%d\n",
              strlen(in), __FILE__, __LINE__);
      exit(-1);
   }

   *L = strlen(in) / 2;

   char *out = calloc(*L + 1, sizeof(char));

   int i;
   for (i = 0; i < *L; i++)
      out[i] = hex2bin(in[2 * i], in[2 * i + 1]);

   //fprintf(stdout, "%s:%d xdigit(out) : %s\n" , __FILE__, __LINE__, out); fflush(stdout);
   /*
      fprintf(stdout, "\nBinary string with length=%d :", *L);
      for (i=0; i<*L; i++) 
      fprintf(stdout, " %x" ,  (0x00ff & out[i]));
      fprintf(stdout, "\n");
      fflush(stdout);
    */
   return out;
}

int help_command(void)
{

   fprintf(stdout, "Usage:\n");
   fprintf(stdout,
           "A terminal for WB Electronics' Infinity USB Unlimited devices.\n\n");
   fprintf(stdout, "  start           : Locates and initializes IUU\n");
   fprintf(stdout, "  stop            : Stops IUU\n");
   fprintf(stdout,
           "  read n          : Read n (base 10, n<255) bytes from the device\n");
   fprintf(stdout,
           "  write xx xx ... : Write individual bytes in hex format to the devicen\n");
   fprintf(stdout,
           "                    Remember that marshalling is little endian\n");
   fprintf(stdout, "  sf n            : Sets CLK frequency\n");
//  fprintf(stdout, "  rx      : Read from phoenix fifo\n");
//  fprintf(stdout, "  tx      : Write to phoenix fifo (i.e. tx fe 34 01 10)\n");
   fprintf(stdout, "  help            : Shows this help\n");
   fprintf(stdout, "  quit            : Quits\n");
   fprintf(stdout, "\n");
   return 0;

}

int read_command(struct usb_infinity *inf, char *str)
{

   int i;
   char needle[] = "read";
   char *ptr = strstr(str, needle);

   if (ptr == NULL) {
      fprintf(stdout, "%s:%d Null pointer! Bad\n", __FILE__, __LINE__);
      fflush(stdout);
   } else {
      fprintf(stdout, "%s:%d Found needle %s! Good\n", __FILE__,
              __LINE__, ptr + strlen(needle));
      fflush(stdout);
   }

   fprintf(stdout, "%s:%d Passing string: %s\n", __FILE__, __LINE__,
           ptr + strlen(needle));
   fflush(stdout);

   int len = atoi(ptr + strlen(needle));

   fprintf(stdout, "%s:%d Number of bytes to read: %d\n",
           __FILE__, __LINE__, len);
   fflush(stdout);

   //u_int8_t buf[] = "cacabeefcaca";
   //iuu_error status = 0;

   u_int8_t *buf = calloc(len + 1, sizeof(u_int8_t));
   iuu_error status = iuu_read(inf, buf, len);

   if (status)
      iuu_process_error(status, __FILE__, __LINE__);
   else {
      u_int8_t un, ln, tmp;

      fprintf(stdout, "Bytes read: ");
      for (i = 0; i < len; i++) {
         tmp = 0x00F0 & buf[i];
         un = (tmp >> 4);
         ln = 0x000F & buf[i];

         fprintf(stdout, " %c%c", dec2hex(un), dec2hex(ln));
      }
      fprintf(stdout, "\n");

      fprintf(stdout, "String from: ");
      for (i = 0; i < len; i++) {
         if isprint
            (buf[i])
                fprintf(stdout, "%c", buf[i]);
         else
            fprintf(stdout, " ");
      }
      fprintf(stdout, "\n");
   }

   free(buf);

   return 0;
}

int sf_command(struct usb_infinity *inf, char *str)
{

   char needle[] = "sf";
   char *ptr = strstr(str, needle);

   if (ptr == NULL) {
      fprintf(stdout, "%s:%d Null pointer! Bad\n", __FILE__, __LINE__);
      fflush(stdout);
   } else {
      fprintf(stdout, "%s:%d Found needle %s! Good\n", __FILE__,
              __LINE__, ptr + strlen(needle));
      fflush(stdout);
   }

   fprintf(stdout, "%s:%d Passing string: %s\n", __FILE__, __LINE__,
           ptr + strlen(needle));
   fflush(stdout);

   int freq = atoi(ptr + strlen(needle));

   fprintf(stdout, "%s:%d Frequency to set: %d\n",
           __FILE__, __LINE__, freq);
   fflush(stdout);

   iuu_error status = iuu_clk(inf, freq);

   if (status)
      iuu_process_error(status, __FILE__, __LINE__);

   return 0;
}

int write_command(struct usb_infinity *inf, char *str)
{

   char needle[] = "write";
   char *ptr = strstr(str, needle);

   /*
      if (ptr==NULL) { 
      fprintf(stdout, "%s:%d Null pointer! Bad\n", __FILE__, __LINE__); 
      fflush(stdout);
      } else { 
      fprintf(stdout, "%s:%d Found needle %s! Good\n", __FILE__, __LINE__, ptr+strlen(needle)); 
      fflush(stdout); 
      }
    */

   //fprintf(stdout, "%s:%d Passing string: %s\n", __FILE__, __LINE__, ptr+strlen(needle)); fflush(stdout);

   char *xdo = xdigit_only(ptr + strlen(needle));

   //fprintf(stdout, "%s:%d Passing string: %s\n", __FILE__, __LINE__, xdo); fflush(stdout);

   int l;
   char *binstr = xdigit2bin(xdo, &l);

   //fprintf(stdout, "Resulting string: %s with length=%d\n", binstr, l);

   /*
      int i;
      for (i=0; i<l; i++)
      fprintf(stdout, " %x", binstr[i]);
      fprintf(stdout, "\n");
      fflush(stdout);
    */

   iuu_error status = iuu_write(inf, (u_int8_t *) binstr, l);
   if (status)
      iuu_process_error(status, __FILE__, __LINE__);

   free(xdo);
   free(binstr);

   return 0;
}

int rx_command(struct usb_infinity *inf, char *in)
{
   /* Pending */
   return 0;
}

int tx_command(struct usb_infinity *inf, char *in)
{
   /* Pending */
   return 0;
}

int stop_command(struct usb_infinity *inf)
{

   iuu_stop(inf);

   return 0;
}

int
init_regexp_command(regex_t * help_cmd, regex_t * quit_cmd,
                    regex_t * read_cmd, regex_t * write_cmd,
                    regex_t * rx_cmd, regex_t * tx_cmd,
                    regex_t * start_cmd, regex_t * stop_cmd,
                    regex_t * sf_cmd)
{

   int status;

   status =
       regcomp(help_cmd, "^[[:space:]]*help[[:space:]]*$",
               REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status =
       regcomp(quit_cmd, "^[[:space:]]*quit[[:space:]]*$",
               REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status = regcomp(write_cmd,
                    "^[[:space:]]*write[[:space:]]+([[:xdigit:]]{2}[[:space:]]*)+$",
                    REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status = regcomp(read_cmd,
                    "^[[:space:]]*read[[:space:]]+[[:digit:]]{1,3}[[:space:]]*$",
                    REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status =
       regcomp(rx_cmd, "^[[:space:]]*rx[[:space:]]*$", REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status =
       regcomp(tx_cmd,
               "^[[:space:]]*tx[[:space:]]+([[:xdigit:]]{2}[[:space:]]*)+$",
               REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status =
       regcomp(start_cmd, "^[[:space:]]*start[[:space:]]*$",
               REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status =
       regcomp(stop_cmd, "^[[:space:]]*stop[[:space:]]*$",
               REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   status = regcomp(sf_cmd,
                    "^[[:space:]]*sf[[:space:]]+[[:digit:]]+[[:space:]]*$",
                    REG_EXTENDED);
   if (status)
      fprintf(stdout, "%s:%d:%s\n", __FILE__, __LINE__,
              strerror(status));

   return 0;

}

int start_command(struct usb_infinity *inf)
{

   int ndev;
   u_int8_t status;

   status = iuu_ndevs(&ndev);
   if (ndev < 1) {
      fprintf(stdout, "No IUU devices found\n");
      return -1;
      //exit(-1);
   }
   fprintf(stdout, "Number of IUU devices found: %d\n", ndev);
   fflush(stdout);

   iuu_start(inf, ndev - 1);

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

   fprintf(stdout,
           "Setting the programming voltage in the device to 5.0V\n");
   iuu_vcc(inf, IUU_VCC_5V);
   status = iuu_status(inf, &statreg);
   if (status != IUU_OPERATION_OK) {
      iuu_process_error(status, __FILE__, __LINE__);
      status = iuu_stop(inf);
      iuu_process_error(status, __FILE__, __LINE__);
      return -1;
   }

   return 0;
}

int main(int argc, char **argv)
{

   fprintf(stdout,
           "\niuuterm version 0.1, Copyright (C) 2006 Juan Carlos Borrás\n\n");
   fprintf(stdout,
           "iuuterm comes with ABSOLUTELY NO WARRANTY; This is free\n");
   fprintf(stdout,
           "software, and you are welcome to redistribute it under\n");
   fprintf(stdout, "certain conditions.\n\n");

   struct usb_infinity my_iuu;
   //device_test(&my_iuu);

   int status;
   char *prompt = ">> ";
   regex_t helpcmd, quitcmd, readcmd, writecmd, rxcmd, txcmd;
   regex_t startcmd, stopcmd, sfcmd;

   init_regexp_command(&helpcmd, &quitcmd, &readcmd, &writecmd, &rxcmd,
                       &txcmd, &startcmd, &stopcmd, &sfcmd);

   regmatch_t match_ptr;
   char *line = NULL;
   //char *line = "      read 12434    ";
   int matched = 1;

   while (1) {
      if (!matched)
         fprintf(stdout, "Unrecognized command or syntax error: %s\n",
                 line);
      matched = 0;

      if (line != NULL)
         free(line);

      line = readline(prompt);
      add_history(line);

      status = regexec(&helpcmd, line, 0, &match_ptr, 0);
      if (status == REG_NOERROR) {
         //fprintf(stdout, "HELP command\n");
         //fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         help_command();
         continue;
      }

      status = regexec(&quitcmd, line, 0, &match_ptr, 0);
      if (status == REG_NOERROR) {
         fprintf(stdout, "Quitting...\n");
         //fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         stop_command(&my_iuu);
         exit(0);
         break;
      }

      status = regexec(&readcmd, line, 0, &match_ptr, 0);
      if (status == REG_NOERROR) {
         fprintf(stdout, "READ command\n");
         fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         read_command(&my_iuu, line);
         continue;
      }

      status = regexec(&writecmd, line, 0, &match_ptr, 0);
      if (status == REG_NOERROR) {
         //fprintf(stdout, "WRITE command\n");
         //fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         write_command(&my_iuu, line);
         continue;
      }

      status = regexec(&rxcmd, line, 0, &match_ptr, 0);
      if (status == REG_NOERROR) {
         fprintf(stdout, "RX command\n");
         fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         rx_command(&my_iuu, line);
         continue;
      }

      status = regexec(&txcmd, line, 1, &match_ptr, 0);
      if (status == REG_NOERROR) {
         fprintf(stdout, "TX command\n");
         fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         tx_command(&my_iuu, line);
         continue;
      }

      status = regexec(&startcmd, line, 1, &match_ptr, 0);
      if (status == REG_NOERROR) {
         fprintf(stdout, "START command\n");
         fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         start_command(&my_iuu);
         continue;
      }

      status = regexec(&stopcmd, line, 1, &match_ptr, 0);
      if (status == REG_NOERROR) {
         fprintf(stdout, "STOP command\n");
         fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         stop_command(&my_iuu);
         continue;
      }

      status = regexec(&sfcmd, line, 1, &match_ptr, 0);
      if (status == REG_NOERROR) {
         fprintf(stdout, "SET FREQUENCY command\n");
         fprintf(stdout, "%d -> %s\n", strlen(line), line);
         matched++;
         sf_command(&my_iuu, line);
         continue;
      }

   }

   return 0;
}
