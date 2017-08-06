#include<string.h>
#include<stdlib.h>
#include<ncurses.h>
#include<stdio.h>


void main() {
   char password[25], ch;
   int i;


   puts("Enter password : ");

   while (1) {
      if (i < 0) {
         i = 0;
      }
      ch = getch();

      if (ch == 13)
         break;
      if (ch == 8) /*ASCII value of BACKSPACE*/
      {
         putchar('b');
         putchar(NULL);
         putchar('b');
         i--;
         continue;
      }

      password[i++] = ch;
      ch = '*';
      putchar(ch);
   }

   password[i] = '\0';
   printf("\nPassword Entered : %s", password);

   getchar();
}
