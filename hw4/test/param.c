typedef int INT;
void fun(int a){}
int f1(int a) { return a; }

void foo(float a) {}
void fuc(int a[2]) {}
void bar(int a[2][3]) {}
void baz(int a, int b, int cc) {}

int main(){
	int a[2];
	int b[5][3];
  int i;
  float f;
  fun(a); /* error */
  fun(a[5]);
  fun(a[1][2]); /* error */
  fun(b); /* error */
  fun(INT); /* error */
  fun(b[3]); /* error */
	fun(b[3][10]);
  fun(a + b); /* error */
  fun(fun(3)); /* error */
  f1(f1(2));
  f1(f1(2) + 3);
  fun(i);
  fun(f);
  foo(i);
  foo(f);
  fuc(a);
  fuc(b); /* error */
  fuc(b[1]);
  fuc(0); /* error */
  fuc(2 + 3); /* error */
  fuc(INT + 3); /* error */
  fuc(foo + 3); /* error */
  fuc(foo(2) + 3); /* error */
  fuc(f1(3) + 3); /* error */
  bar(a); /* error */
  bar(a[0]); /* error */
  bar(b);
  bar(b[2]); /* error */
  bar(b[3][2]); /* error */
  fun(2);
  fun(2 + 5);
  fun(2 + f1(3));
  fun(INT); /* error */
  fun(fun); /* error */
  fun(f1(2) + 3, f(2) + f(5)); /*error */
  fun(2, 3); /* error */
  fun(2, 3, 4); /* error */
  baz(); /* error */
  baz(1); /* error */
  baz(1, 2); /* error */
  baz(1, 2, 3);
  baz(1, 2, 3, 4); /* error */
	return 0;
}
