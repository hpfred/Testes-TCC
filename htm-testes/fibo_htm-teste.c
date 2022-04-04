#include <stdio.h>
#include <pthread.h>

#include <immintrin.h>
#include <rtmintrin.h>

#define NUMTHREADS 2
//#define n 6

void initThreads();
void fibo(int n);
void *fibo2(int i);

int aborted;
//int f[n+2];
int f[6+2];
//pthread_t tid[n];//[NUMTHREADS];
pthread_t tid[6];
int args[NUMTHREADS];
pthread_mutex_t globalLock;

int main(){
    pthread_mutex_init(&globalLock, NULL);
    aborted = 0;

    ///
    int n = 10;//6; Com 6 estava dando certo, mas com 10 resultados de todos os tipos e numeros de abortos variados tbm
    fibo(n);

    //printf("\nDeve ser zero: %d\n", globalVar)/

    pthread_mutex_destroy(&globalLock);
    return 0;
}

void fibo(int n){
    int i;

    f[0] = 0;
    f[1] = 1;

    for(i=2; i<=n; i++){
        pthread_create(&(tid[i-2]), NULL, fibo2, i);
    }
    for(i=2; i<=n; i++){
        pthread_join(tid[i-2], NULL);
    }

    printf("\nResultado de Fibo(%d): %d\n", n, f[n]);
    printf("\n--Nº Abortos: %d--\n", aborted);

    // if(n<=1)
    //     return n;
    // pthread_create(&(tid[1]), n-1, fibo2, NULL);
    // pthread_create(&(tid[2]), n-2, fibo2, NULL);

    // pthread_join(tid[1], NULL);
    // pthread_join(tid[2], NULL);
}

void *fibo2(int i){
    unsigned status = 0;
    ///mutex+begin
    //pthread_mutex_lock(&globalLock);
    ///Mas e se um fibo subsequente comeca antes ao fibo anterior, ele nao e pego pelo TM?
    if((status= _xbegin()) == _XBEGIN_STARTED){
        f[i] = f[i-1] + f[i-2];
        _xend();
    }
    else{
        aborted++;
        //qual um fallback bom para essa situação?
        //tentar de novo? mas isso n pode acontecer problema dele fazer
        fibo2(i);
    }
    ///mutex+end
    //pthread_mutex_unlock(&globalLock);

    // int i;

    // if(n<=1)
    //     return n;
    // return fibo3(n-1) + fibo3(n-2);
}

// int fibo3(){
//     int i;
//     int res1, res2;

//     if(n<=1)
//         return n;
//     res1 = fibo3(n-1);
//     res2 = fibo3(n-2);
    
//     //Lock aqui
//     res = res1 + res2;
    
//     return res; 
//     //return fibo3(n-1) + fibo3(n-2);
// }