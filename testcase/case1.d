/* Global declarations */
bool flag;
int nums[2];
float farr[1];
string sarr[2][3];

/* Multiple variable declarations with initialization (constants only) */
int a = 1, b = -2, c;
const bool CTRUE = true;
const float PIF = 3.14;

/* Procedures and Functions */

// Empty procedure
void proc1() {
    ;
}

// Function with array and string parameter
int sumBoolString(int bs[2], string msg) {
    int total = 0;
    int i;
    for (i = 0; i < 2; i++) {
        if (bs[i] == 1)
            total = total + 1;
    }
    print msg;
    println total;
    return total;
}

// Recursive Fibonacci
int fib(int n) {
    if (n <= 1)
        return n;
    else {
        return fib(n-1) + fib(n-2);
    }
}

void main() {
    /* Nested block and shadowing */
    {
        int a;
        nums[0] = 0;
        a = nums[0];
        a++;
        a--;
    }

    /* Read input */
    read c;

    /* Arithmetic and operator precedence */
    nums[0] = a * (b + c) / 2 % 3 - -1;

    /* Comparison and logical operators */
    flag = !(a < b) && (c == 0 || a >= 0);

    /* Fill arrays via loops */
    int i, j, k;
    for (i = 0; i < 5; i++)
        nums[i] = i * i;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 3; j++) {
            sarr[i][j] = "s";
        }
    }
    int cube[2][2][2];
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 2; k++) {
                cube[i][j][k] = i + j + k;
            }
        }
    }

    /* Foreach forward and reverse */
    foreach (i : 0 .. 4) {
        nums[i] = nums[i] + 1;
    }
    foreach (i : 4 .. 0)
        nums[i] = nums[i] - 1;

    /* Function invocations */
    int res1;
    res1 = sumBoolString(nums, "CountTrue:");
    println res1;
    int fib5;
    fib5 = fib(5);
    println fib5;

    /* Conditional statements */
    if (fib5 != 5)
        fib5 = 0;
    if (fib5 == 5) {
        print "Fib ok";
    } else
        print "Fib fail";
    println fib5;

    /* Procedure invocation */
    proc1();

    /* Read and print string */
    string user;
    read user;
    println user;

    /* Empty statement */
    ;
}
