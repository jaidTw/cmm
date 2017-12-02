typedef int INT;

int main() {
    int a = 0 , b = 0;
    int c[10];
    int i;
    for(i = 0, b = 0; d, a, i <= 10; i = i + 1) { /* undecl d */
      a = a + 5;
    }
    for(;;)
    for(c=5;;); /* error */
    for(a="test";;); /* error */
    for(;c < 10;); /* error */
    for(;c;); /* error */
    for(;;c = c + 1); /* error */
    for(INT = 3;;); /* error */
    for(;INT;); /* error */
    for(;;INT = 2); /* error */
    for(main = 0;;); /* error */
    for(;main;); /* error */
    for(;;main = 2); /* error */
    for(; i < 20;);
    for(g = 0;;) /* undecl g */
    for(;;d = i * 1); /* undecl d */
    for(;c < 5;); /* undecl c */
    return 0;
}
