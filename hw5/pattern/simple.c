int func(int a, int b, int c) {
    write(a);
    write("\n");
    write(b);
    write("\n");
    write(c);
    a = 5;
    return a;
}

int func2(float a, float b, float c) {
    write(a);
    write("\n");
    write(b);
    write("\n");
    write(c);
    a = 100.0;
    return a;
}
int MAIN(){
    int a = 100;
    int b = 200;
    float aa = 1.0;
    func(a * 2, b * 3, 300);
    write("\n");
    aa = func2(a * 2.0, b * 3.0, 300);
    write("\n");
    write(a);
    write("\n");
    write(aa);
    write("\n");
    return 0;
}
