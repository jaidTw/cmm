void fun(int a){
	write("This is a.");
}

float foo(float a) {
  return a;
}

int main(){
	int a[2];
	int b[5][3];
  int i;
  float f;
  fun(a);
  fun(a[5]);
  fun(a[1][2]);
  fun(b);
	fun(b[3]);
	fun(b[3][10]);
  fun(a + b);
  fun(i);
  fun(f);
  foo(i);
  foo(f);
  
	return 0;
}
