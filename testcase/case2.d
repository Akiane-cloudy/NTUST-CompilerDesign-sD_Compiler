/* Global constants and variables */
const float PI = 3.14159;
bool ready;
int flat[1];
int matrix2x3[2][3];
string labels[2][2];
int cube2x2x2[2][2][2];

/* Utility functions */

// Compute factorial recursively
int fact(int n) {
    if (n <= 1) return 1;
    else        return n * fact(n - 1);
}

// Sum a 1D int array of length 4
int sum4(int arr[4], int len) {
    int i, s = 0;
    for (i = 0; i < len; i++)
        s = s + arr[i];
    return s;
}

// Reverse a 2×2 string matrix in place
void reverse2x2(string m[2][2]) {
    string tmp = m[0][0];
    m[0][0] = m[1][1];
    m[1][1] = tmp;
    tmp    = m[0][1];
    m[0][1] = m[1][0];
    m[1][0] = tmp;
}

void main() {
    int i, j, k, acc, val;
    string name;

    /* Nested block and shadowing */
    {
        int flat[2];
        flat[0] = 9;
        flat[1] = 8;
        acc = flat[0] + flat[1];
        println acc;
    }

    /* Initialize matrix2x3 */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 3; j++) {
            matrix2x3[i][j] = i * 3 + j;
        }
    }

    /* Initialize 3D cube */
    for (i = 0; i < 2; i++)
    for (j = 0; j < 2; j++)
    for (k = 0; k < 2; k++)
        cube2x2x2[i][j][k] = i + j + k;

    /* Exercise operator precedence and unary */
    val = - (1 + 2) * 3 / 2 % 2 - -5;
    println val;

    /* Logical and comparison */
    ready = (val > 0 && !(val == 3 || val < -1)) || false;
    print "Ready? ";
    println ready;

    /* foreach forward and reverse */
    foreach (i : 0 .. 1) {
        labels[i][i] = "X";
    }
    foreach (j : 1 .. 0)
        labels[1-j][j] = "Y";

    /* Reverse the 2×2 labels */
    reverse2x2(labels);
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            println labels[i][j];

    /* Test 1D array sum */
    int small[4];
    for (i = 0; i < 4; i++) small[i] = i * 2;
    acc = sum4(small, 4);
    println acc;          

    /* Test recursion */
    int f5 = fact(5);
    println f5;           

    /* While loop */
    i = 0;
    while (i < 3) {
        println i;
        i++;
    }

    /* Read and echo */
    read name;
    print "Hello, ";
    println name;

    /* Empty statement */
    ;
}
