/* 全局变量与数组 */
bool flag;
int one[1];
int hex4[4];
int cube2x2x2[2][2][2];
string s2d[2][2];
float farr[3];

/* 运算符优先与一元/自增自减测试 */
int opTest(int x) {
    return x * x-- - x-- % (x + 1) - -(x - 2);
}

/* 对 2×2 矩阵求和 */
int sum2x2(int m[2][2]) {
    int s = 0;
    int i, j;
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            s = s + m[i][j];
    return s;
}

int odd(int n) {
    if (n == 0)
        return 0;
    else
        return 1;
}
int even(int n) {
    if (n == 0)
        return 1;
    else
        return odd(n - 1);
}

void main() {
    int i, j, k, temp;
    int x = 0;

    /* 多层 shadowing */
    {
        int x = 1;
        {
            int x = 2;
            println x;
        }
        println x;
    }
    println x;

    /* 单句 for */
    for (i = 0; i < 1; i++)
        println i;

    /* 单句 foreach 逆序 */
    foreach (j : 2 .. 0)
        println j;

    /* 动态填充 2×2 矩阵 */
    int mat[2][2];
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            mat[i][j] = i * 2 + j;

    /* 调用 sum2x2 */
    int total = sum2x2(mat);

    /* 调用 opTest */
    int v = opTest(3);

    int m = even(5) + odd(5);

    /* 连续的 print/println 单句 */
    print "total=";   println total;
    print "opTest=";  println v;
    print "mutual=";  println m;

    /* 单句 if-else 测试 char literal */
    if ('a' < 'z')
        print "chars ok";
    else
        print "fail";
    println v;

    /* 读入并回显 */
    string name;
    read name;
    println name;

    /* 空语句 */
    ;
}
