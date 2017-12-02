int main() {
  int a[1];
  int aa[0][0][0][0][0][0][0][0][0][0][0];
  int b[2.5]; /* error */
  int c[8.6/2]; /* error */
  float d[2/0.2]; /* error */
  int e[3*4+(5-3)/2]; /* OK */
}
