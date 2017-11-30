int f(int a){}

int g(int a, float b){
  return ;
  return a;
  return b;
  return 1;
  return 3.14;
}

int h(int a[100][1000], float b[][222]){}

int main(){
  int ivar;
  int iivar = 10, iarr[10][100][1000];
  float fvar;
  float ffvar, iivar, farr[2][22][222];
  ivar = f(ivar);
  fvar = g(ivar, fvar);
  ffvar = f(fvar, fvar);
  iivar = h(iarr, farr);
  h(iarr[10], farr[1]);
  h(iarr[1][2][3], farr[1][2]);
  iivar = iarr[1.0];
  return 0;
}