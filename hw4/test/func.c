typedef int INT;

int f0(int a){ return a; }
int f1(int a){ return f0(a) + 1; }
void f1(int a){} /* error */
void f2(int a){ return a; } /* error */
void f2a(int a){ return 3; } /* error */
int f3(int a){ return; } /* error */
int f4a(INT a){ return INT + 2; } /* error */
void f4(){ return; }
int f5(float a, int a){ return a; } /* error */
int f6(int a[3]){ return a; } /* error */
int f7(int b[][5]){ return b[0][0]; }
int f7a(int b[][5]){ return b[0]; } /*error */
int f7b(float a[]) { return a[2]; } /* warning */
float f7c() { return 0; } /* warning */
void f8(int b[2.6]) {} /* error */
void f9(int b[]) {}
void f10(int b[5]) {}
int main(){
	float f0;
	int i;
  int ii[10];
  i(2); /* error */
  f0(2); /* error */
  f100(); /* error */
  f1(); /* error */
  f4();
  f4 = 3; /* error */
  i = f1 - 3; /* error */
  i = ii[f1] - 3; /* error */
  i = ii[f1(3)] - 3;
  f4(2); /* error */
  f2(); /* error */
  f9(i); /* error */
  f9(ii);
	return 0;
}
