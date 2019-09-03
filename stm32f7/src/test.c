#include<stdio.h>

//自定义printN函数
void printN(int N)
{
	int i;
	for (i = 1; i <= N; i++) {
		printf("%d\n", i);
	}
	return;
}

//声明printN函数
void printN(int N);

main(void)
{
	int N;
	printf("请输入N：");
	scanf("%d", &N); //传入参数
	printN(N); //调用printN函数
}
