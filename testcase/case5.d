/* no_scope_tests.sd
 * 測試只有 simple statement（非 block）作為 if/else/for/foreach 主體時的行為。
 */

/* 1. if 單句合法 */
void main1() {
    int x = -5;
    if (x < 0)
        x = -x;
    println x;
}

/* 2. if-else 單句合法 */
void main2() {
    int y = 2;
    if (y % 2 == 0)
        print "even";
    else
        print "odd";
    println y;
}

/* 3. 嵌套 if-else 單句 */
void main3() {
    int z = 0;
    if (z > 0)
        z = 1;
    else
        z = 0;
    println z;
}

/* 4. for 單句合法 */
void main4() {
    int i;
    for (i = 0; i < 3; i++)
        println i;    // 0 1 2
}

/* 5. foreach 單句合法 */
void main5() {
    int j = 0;
    foreach (j : 0 .. 2)
        j = j + 10;
    println j;
}

/* 6. if 裡放宣告 */
void main6() {
    int a = 1;
    if (a > 0)
        int b = 2;
    println a;
}

/* 7. for 裡放宣告 */
void main7() {
    int c = 0;
    for (c = 0; c < 1; c++)
        int d = 3;
    println c;
}

/* 8. foreach 裡放宣告*/
void main8() {
    int e = 0;
    foreach (e : 1 .. 1)
        string s = "hi";
    println e;
}

void main(){}