int foo(int a) {
    {
        int a = 10;
        return a;
    }
}

void main() {
    int a = foo(5);
    println a;
}
