# include <stdio.h>
# include <stdlib.h>
# define NULL 0
# define OK 1
# define ERROR 0
# define MAXSIZE 100 /*  矩阵中非零元的最大值 */
# define MAXRC 10             /*  矩阵的最大行值  */
 
typedef int status ;
 
 /********** 稀疏矩阵的行逻辑链接的顺序表存储表示 **********/
  
typedef struct       /*  非零元的三元组    */
{
       int i, j ;   /*    非零元的行下标和列下标    */
       int e ;
}Triple;    
 
typedef struct  /*    稀疏矩阵的行逻辑链接的顺序表         */
{
       Triple data[MAXSIZE+1];    /*    非零三元组表，data[0]未用，以下定义的数组都是从1开始 */
       int rpos[MAXRC+1];     /*    代表各行第一个非零元的序号表，其值为data的下标   */
       int mu,nu,tu;   /*    矩阵的行数、列数、非零元的个数    */
}RLSMatrix;   /*    R:row     L:logic    S:sequence      */
 
 
 /********* 基本操作的函数原型的声明 *********/
  
status CreateSMatrix_RL(RLSMatrix    * matrix);
//     创建一个稀疏矩阵；
//     输入行数、列数，支持乱序输入三元组，并计数；
//     以行为主序进行重新排列，并记录每行起始位置于matrix->rpos[row]；
//     若非零元超过 MAXSIZE或行数超过MAXRC，则返回ERROR，否则OK；
 
void PrintSMatrix_RL(RLSMatrix       * matrix);
//     输入矩阵，打印出矩阵的行数、列数、非零元个数，以及整个矩阵；
 
status MultSMatrix_RL(RLSMatrix      * M,RLSMatrix  * N,RLSMatrix * Q);
//     输入两个稀疏矩阵M和N，并初始化Q，然后计算M*N的值赋给Q；
//     如果M->mu!=N->nu或列数大于MAXRC或者计算出的非零元个数大于MAXSIZE，都返回ERROR,否则OK；
//     计算过程如下：
//     1.    由于矩阵M和Q的行数相等并且C语言以行为主序进行存储，所以以M进行逐行的扫描。
//     2.    使Q的此行逻辑表的序号等于其非零元个数Q.tu+1，以表示其行的首个元素的序号。
//     3.    从行中找到M的非零元，并以它的列值为N的行号，对N进行行的扫描，若存在，则依次计算它们，并把其值累加到一个以N中这个对应非零元的列值为序号的临时数组ctemp[ccol]中。
//     4.    在M的当前行完成扫描后，将ctemp[ccol]不为0的值，压入到Q矩阵的三元组，累加++Q.tu，若Q.tu大于了MAXSIZE，这返回ERROR。 
 
/************ main( ) 函数对矩阵乘法的实现  ************/
  
void main()
{
       RLSMatrix * M,* N,* Q;
       if(!(M=(RLSMatrix *)malloc(sizeof(RLSMatrix))))   
              exit(ERROR);
       if(!(N=(RLSMatrix *)malloc(sizeof(RLSMatrix))))    
              exit(ERROR);
       if(!(Q=(RLSMatrix *)malloc(sizeof(RLSMatrix))))    
              exit(ERROR);
       if(CreateSMatrix_RL(M)&&CreateSMatrix_RL(N))
       {
              printf("\nput out M:\n");
              PrintSMatrix_RL(M);           /*    打印出M       */
              printf("\nput out N:\n");
              PrintSMatrix_RL(N);            /*    打印出N       */
	      
              if(MultSMatrix_RL(M,N,Q))
              {
                     printf("\n\n\n M  *  N  :\n");
                     PrintSMatrix_RL(Q);     /*    计算结果       */
              }
              else
                     printf("M.mu and N.nu are not mathing\n");
       }
       else
              printf("input error.\n");
}
 
 
/*********** 基本操作的算法描述 ****************/
 
status CreateSMatrix_RL(RLSMatrix    * matrix)
//     创建一个稀疏矩阵；
//     输入行数、列数，支持乱序输入三元组，并计数；
{
       int num=0,p,q,min,temp;             //     中间变量；
       int row;
       printf("请输入矩阵的行数和列数。\n");
       scanf("%d%d",&matrix->mu,&matrix->nu);      //     输入行数、列数；
       if(matrix->mu>MAXRC)
              return ERROR;
       printf("row col val\n");
       
       scanf("%d%d%d",&matrix->data[num+1].i,&matrix->data[num+1].j,&matrix->data[num+1].e);
       while(matrix->data[num+1].i)      //     乱序输入三元组；
       {
              if(++num>MAXSIZE)
                     return ERROR;
              scanf("%d%d%d",&matrix->data[num+1].i,&matrix->data[num+1].j,&matrix->data[num+1].e);
       }
       matrix->tu=num;                  //     num的值即为此矩阵的非零元个数；
       for(p=1;p<=matrix->tu-1;++p)            //     按行为主序依次重新排列非零元
       {
              min=p;           //     使较小的行数、列数的元的序号min为当前值p；
              for(q=p+1;q<=matrix->tu;++q)           //     开始依次比较；
              {
                     if(matrix->data[min].i>matrix->data[q].i||(matrix->data[min].i==matrix->data[q].i&&matrix->data[min].j>matrix->data[q].j))
                            min=q;           //     在乱序的三元表中，始终保证min是较小的行列数的序号；
              }
              temp=matrix->data[min].i;                         //     交换行值;
	      matrix->data[min].i=matrix->data[p].i;
              matrix->data[p].i=temp;
              temp=matrix->data[min].j;                         //     交换列值；
              matrix->data[min].j=matrix->data[p].j;
              matrix->data[p].j=temp;
              temp=matrix->data[min].e;                        //     交换元素值；
              matrix->data[min].e=matrix->data[p].e;
              matrix->data[p].e=temp;
       }
       for(row=1,num=0;row<=matrix->mu;++row)            //     记录matrix->rpos[row];
       {
              matrix->rpos[row]=num+1;
              while(matrix->data[num+1].i==row)
                     ++num;  
       }
       return OK;
}
//     时间复杂度分析：
//     1.    输入非零元：O(tu);     2.    重新排列（最坏情况下）;O(tu*(tu-1)) ;    3.    记录行逻辑表：O(mu)
void PrintSMatrix_RL(RLSMatrix       * matrix)
//     输入矩阵，打印出矩阵的行数、列数、非零元个数，以及整个矩阵；
{
       int row,col;
       int num=0;
       printf("\nrow:%d col:%d number:%d\n",matrix->mu,matrix->nu,matrix->tu);
       for(row=1;row<=matrix->mu;++row)
       {
              for(col=1;col<=matrix->nu;++col)
              {
                     if(num+1<=matrix->tu&&matrix->data[num+1].i==row&&matrix->data[num+1].j==col)
                     {
                            ++num;
                            printf("%4d",matrix->data[num].e);            /*    当扫描到非零元的行列值与之相等时，输出其值    */
                     }           
                     else
                            printf("%4d",NULL);           /*    没有非零元的地方补0 */
              }
              printf("\n");           /*    每行输入完毕后，换行         */
       }
}
 
//     时间复杂度：O(mu*nu).
status MultSMatrix_RL(RLSMatrix      * M,RLSMatrix  * N,RLSMatrix * Q)
//     输入两个稀疏矩阵M和N，并初始化Q，然后计算M*N的值赋给Q 
{
       int arow,brow,ccol;
       int * ctemp;    /*    以N的列值为序号的临时数组   */
       int tp,p,tq,q;          /*    中间变量       */
       if(M->nu!=N->mu)
              return ERROR;
       Q->mu=M->mu;           /*    初始化Q       */
       Q->nu=N->nu;
       Q->tu=0;
       if(!(ctemp=(int *)malloc((N->nu+1)*sizeof(int))))     /*    动态建立累加器    */
              exit(ERROR);
       if(M->tu*N->tu!=0)             /*    Q是非零矩阵       */
       {
              for(arow=1;arow<=M->mu;++arow)    /*    逐行扫描       */
              {
                     for(ccol=1;ccol<=N->nu;++ccol)
                            ctemp[ccol]=0;              /*    初始化累加器       */
                     Q->rpos[arow]=Q->tu+1;
                     if(arow<M->mu)          
                            tp=M->rpos[arow+1];    /*    tp是M下一行的序号   */
                     else
                            tp=M->tu+1;
                     for(p=M->rpos[arow];p<tp;++p)   /*    从M的当前行找到元素      */
                     {
                            brow=M->data[p].j;              /*    对应元在N中的行号   */
                            if(brow<N->mu)
                                   tq=N->rpos[brow+1];    /*    tq是N下一行的行号   */
                            else
                                   tq=N->tu+1;
                            for(q=N->rpos[brow];q<tq;++q)   /*    以M的对应元的列号为N的行号进行扫描      */
                            {
                                   ccol=N->data[q].j;         /*    提取对应元的列号       */
                                   ctemp[ccol]+=M->data[p].e*N->data[q].e;  
                                   /*    两个对应元的值相乘并累加到以列号为序号的累加器中       */
                            }
                     }
                     for(ccol=1;ccol<=Q->nu;++ccol)  /*    将此行非零元压缩入Q中   */
                     {
                            if(ctemp[ccol])
                            {
                                   if(++Q->tu>MAXSIZE)
                                          return ERROR;
                                   Q->data[Q->tu].i=arow;
                                   Q->data[Q->tu].j=ccol;
                                   Q->data[Q->tu].e=ctemp[ccol];
                            }
                     }
              }
       }
       return OK;
}
//     时间复杂度：O(M->mu*(N->nu+M->nu*N->nu+N->nu));            

status ResortMatrix(RLSMatrix * matrix){
  
       int num=0,p,q,min,temp;             //     中间变量；
       int row,col;
       int temp1, temp2, temp3;
         for(p=1;p<=matrix->tu-1;++p)            //     按行为主序依次重新排列非零元
       {
              min=p;           //     使较小的行数、列数的元的序号min为当前值p；
              for(q=p+1;q<=matrix->tu;++q)           //     开始依次比较；
              {
                     if(matrix->data[min].j>matrix->data[q].j||(matrix->data[min].j==matrix->data[q].j&&matrix->data[min].i>matrix->data[q].i))
                            min=q;           //     在乱序的三元表中，始终保证min是较小的行列数的序号；
              }
              temp1=matrix->data[min].i;                        //     交换行值;
              matrix->data[min].i=matrix->data[p].i;
              matrix->data[p].i=temp;
              temp2=matrix->data[min].j;                        //     交换列值；
              matrix->data[min].j=matrix->data[p].j;
              matrix->data[p].j=temp;
              temp3=matrix->data[min].e;                        //     交换元素值；
              matrix->data[min].e=matrix->data[p].e;
              matrix->data[p].e=temp;
	      printf("%4d%4d%4d",temp1,temp2,temp3); 
       }
}