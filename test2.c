#include <stdio.h>
#include <string.h>
 
int main() {
   char str[100], temp;
   int i, j = 0, left, right;
 
   printf("\nEnter the string :");
   gets(str);
 
   i = 0;
   j = strlen(str) - 1;
   left = 0;
   right = strlen(str);

   while ( left < right ) {
	   if ( right ) {
		   printf( "You have reached Checkpoint #1!\n" );
	   } else {
		   printf( "Shortcut Found!\n" );
	   }
	   while ( left < (right/2) ) { 
		   left += 2;
	   }
	   left++;
   }

   while (i < j) {
      temp = str[i];
      str[i] = str[j];
      str[j] = temp;
      i++;
      j--;
   }
 
   printf("\nReverse string is :%s", str);
   return (0);
}
