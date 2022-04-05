#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
//#include "stm.h"

#include <immintrin.h>
#include <rtmintrin.h>

int aborted;

void fib(int n, int *r) {
  unsigned status = 0;
  
  if (n<2) {
    // //  sigjmp_buf * checkPoint = stm_start((stm_tx_attr_t) {0}); 
    // //     if (checkPoint != 0) sigsetjmp(*checkPoint, 0);
    // //  stm_store(r,stm_load(r)+n);
    // //  stm_commit();

  // o sigjmp ali seria o if begin, o stm store é aonde é feito a soma do ponteiro compartilhado com a soma do ponteiro compartilhado + n
  // o stm commit é o end, e provavlemente o fall back tbm
    if((status= _xbegin()) == _XBEGIN_STARTED){
        *r = (*r) + n;
        _xend();
    }
    else{
	#pragma omp atomic
        aborted++;
        fib(n, r);
    }

  }
  else {
    #pragma omp task 
     fib(n-1,r);
    //#pragma omp task 
     fib(n-2,r);
    #pragma omp taskwait
  }
}

int main( int argc, char **argv ) {
 int n, t, r = 0;
 aborted = 0;

 t = atoi(argv[1]);
 n = atoi(argv[2]);
 if( n > 40 || n < 5 ) n = 20;
// printf("Calculando n = %d\n", n );

 double start = omp_get_wtime();

 #pragma omp parallel num_threads(t)
 {
  // // stm_init_thread();
  #pragma omp single
   fib(n,&r);
  // // stm_exit_thread();
 }

 printf("%f\n", omp_get_wtime()-start); 
 printf("fibo(%d) = %d\n", n, r );
printf("Abortos: %d\n",aborted);

 return 0;
}
 
 
