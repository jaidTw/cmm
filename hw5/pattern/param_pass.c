
int func(int a[1][2][3]) {
    write(a[0][0][0]);
    write("\n");
    write(a[0][1][0]);
    write("\n");
    return 0;
}

int func2(int a[2][3]) {
    write(a[0][0]);
    write("\n");
    write(a[1][0]);
    write("\n");
    return 0;
}

int MAIN(){
    int a[1][2][3];
    a[0][0][0] = 200;
    a[0][1][0] = 300;
    func(a); 
    func2(a[0]);
    return 0;
}
