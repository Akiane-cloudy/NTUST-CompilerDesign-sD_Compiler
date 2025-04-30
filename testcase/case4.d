void outer() {
    void inner() {  // error: nested function declarations are not allowed
        ;
    }
}

void main() {
    outer();
}
