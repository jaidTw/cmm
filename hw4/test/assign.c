int main() {
   int a;
   float b;
   int c[10];
   float d[5];
   int e[2][2];
   a[2][2] = 3; /*error */
   a[2] = 3; /*error*/
   a = b[2]; /* error */
   c[3] = d; /* error */
   c = 6; /* error */
   c = d[2]; /* error */
   c = d; /* error */
   c[3] = 5;
   e[2][1] = 1;
   e[1] = 1; /*error */
   a = 5 + c; /* error */
   a = 5 + c; /* error */
   a = 5 + e[1]; /* error */
   a = 5 + e[1][1] + d[4];
   a = 5;
   a = 2.3;
   b = 4;
   b = 1.2;
   return 0;
}
