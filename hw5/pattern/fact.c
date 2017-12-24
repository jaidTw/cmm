int n;
int fact() {
    if (n == 1) {
        return n;
    }
    else {
      n =n-1;
      return (n*fact());
    }
}

int MAIN() {
    int result;
    write("Enter a number:");
    n = read();
    n = n+1;
    if (n > 1) {
        result = fact();
    } else {
      result = 1;
    }
    write("The factorial is ");
    write(result);
    write("\n");
}
