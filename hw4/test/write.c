typedef int INT;
void f2() {}
int f1() {return 0;}

int main() {
  int i;
  float f;
  int a[10];
  write(); /* error */
  write(1);
  write(1.5);
  write(INT); /* error */
  write(INT+1); /* error */
  write("hi");
  write("hi" + 1); /* error */
  write(a + 1); /* error */
  write(a); /* error */
  write(f2); /* error */
  write(f2()); /* error */
  write(f1());
  write(a[0]);
  write(1, 2, 3); /* error */
}
