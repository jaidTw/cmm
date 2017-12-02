typedef int INT;
void foo() {}
int bar() { return 1; }

int main() {
   int a;
   float b;
   int c[10];
   float d[5];
   int e[2][2];
   a[2][2] = 3; /*error */
   a[2] = 3; /* error*/
   a = b[2]; /* error */
   a = foo(); /* error */
   a = foo() + 2; /* error */
   c["test"] = 5; /* error */
   c["test" + 3] = 5; /* error */
   c[foo()] = 1; /* error */
   c[bar()] = 1;
   c[d[2]] = 1; /* error */
   c[e[2]] = 1; /* error */
   c[e[2][2] + 1] = 1;
   c[INT] = 1; /* error */
   c[INT + 2] = 3; /* error */
   c[bar() + 0.5] = 1; /* error */
   c[3] = d; /* error */
   c = d[2]; /* error */
   c = d; /* error */
   c[3] = 5;
   c[2] = d[1];
   e[2][1] = 1;
   e[1] = 1; /*error */
   a = 5 + c; /* error */
   a = 5 + e[1]; /* error */
   a = 5 + e[1][1] + d[4];
   a = 5;
   INT = 3; /* error */
   a = INT; /* error */
   a = 2.3;
   b = 4;
   b = 1.2;
   return 0;
}
