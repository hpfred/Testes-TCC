/// Aula 7: Programa��o Multithread com Pthreads
/// Aluno: Frederico Peixoto Antunes
/// Status: Funciona
/// Problema: Nenhum... eu acho

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

pthread_mutex_t m, n;
pthread_cond_t cond;

int NumberItems;

/// Cria variavel t como global, para pode ser usado na fun��o Push
int t;

typedef struct Nodo{
    int valor;                                  /// Vari�vel do valor a ser armazenado
    struct Nodo *prox;                          /// Ponteiro para o pr�ximo elemento da fila
}Nodo;

typedef struct Fila{
    Nodo *First;                                /// Ponteiro para o primeiro elemento da fila
    Nodo *Last;                                 /// Ponteiro para o �ltimo elemento da fila

    int tamanho;                                /// Vari�vel que guarda tamanho atual da fila
}Fila;

/// Cria estrutura Dado para passar parametros das fun��es: valor V e buffer B, como (void*) pelo pThread
typedef struct Dado{
    //int V;
    Fila *B;
}Dado;

/// Declara��o das fun��es
Fila *CriaFila();
void Push(Fila *fila, int v);
int Pop(Fila *fila);
void *Produtor(void *in);
void *Consumidor(void *in);
void imprimeFila(Fila *fila);

///              ///
///     MAIN     ///
///              ///
int main(int argc, char *argv[ ]){
    /// Inicializa varivaies com os parametros de entrada
    /*
    int ThreadsProd = atoi(argv[1]);
    int ThreadsCons = atoi(argv[2]);
    int NumberItems = atoi(argv[3]);
    //*/
    int ThreadsProd;
    scanf("%d",&ThreadsProd);
    int ThreadsCons;
    scanf("%d",&ThreadsCons);
    scanf("%d",&NumberItems);
    t = NumberItems;
    /// Cria uma instancia da estrutura Dado
    Dado Param;

    /// Inicializa as fun��es de pthreads
    pthread_t tid[ThreadsProd+ThreadsCons];
    pthread_mutex_init(&m, NULL);   // pthread init pode retornar erro, fazer teste
    pthread_cond_init(&cond, NULL);

    /// Cria e inicializa o Bufffer
    Fila *Buffer;
    Buffer = CriaFila();
    /// Passa os valores de entrada para a instancia de Dado
    Param.B = Buffer;
    //Param.V = NumberItems;

    /// Cria 'ThreadsProd' threads de Produtor
    int Prod;
    for(Prod=0; Prod<ThreadsProd; Prod++){
        pthread_create(&tid[Prod], NULL, Produtor, (void*)&Param);
    }
    /// Cria 'ThreadsCons' threads de Consumidor
    int Cons;
    for(Cons=0; Cons<ThreadsCons; Cons++){
        pthread_create(&tid[Cons+ThreadsProd], NULL, Consumidor, (void*)&Param);
    }

    /// Aguarda o t�rmino dos produtores
    for(Prod = 0; Prod<ThreadsProd; Prod++){
        pthread_join(tid[Prod], NULL);
    }
    /// Insere 'ThreadsCons' valores '-1' no Buffer
    for(Cons = 0; Cons<ThreadsCons; Cons++){
        Push(Buffer, -1);
    }

    /// Aguarda o t�rmino dos consumidores
    for(Cons = 0; Cons<ThreadsCons; Cons++){
        pthread_join(tid[Cons + ThreadsProd], NULL);
    }

    return 0;
}

///              ///
///     FILA     ///
///              ///
Fila *CriaFila(){
    Fila *fila;

    fila = (Fila *) malloc(sizeof(Fila));
    fila->First = NULL;
    fila->Last = NULL;
    fila->tamanho = 0;

    return fila;
}

void Push(Fila *fila, int v) {
    Nodo *novo = (Nodo *) malloc(sizeof(Nodo));
    novo->valor = v;
    novo->prox = NULL;

	pthread_mutex_lock(&m);                     /// Bloqueia condi��o de corrida
	while(fila->tamanho >= t){                  /// Caso o tamanho do Buffer chegue ao limite 't' estabelecido, permanece bloqueado aguardando uma sinaliza��o de espa�o foi liberado
		pthread_cond_wait(&cond, &m);
	}
    if (fila->Last != NULL)                     /// Caso fila n�o esteja vazia, adiciona item ao fim
        fila->Last->prox = novo;
    else
        fila->First = novo;

    fila->Last = novo;
    fila->tamanho++;                            /// Aumenta indicador do tamanho atual do Buffer
    //imprimeFila(fila);                        /// DEBUG
    pthread_cond_signal(&cond);                 /// Sinaliza que um elemento foi adicionado ao Buffer
    pthread_mutex_unlock(&m);
}

int Pop(Fila *fila) {
    Nodo *nodo;
	int Prod;

	pthread_mutex_lock(&m);                     /// Bloqueia condi��o de corrida
	while(fila->First == NULL)                  /// Caso o Buffer esteja vazio, permanece bloqueado aguardando a sinaliza��o de quando valor for inserido no Buffer
		pthread_cond_wait(&cond, &m);           // Quando n� de consumidores � muito maior do que o n� de itens ele se perde provavelmente nesse la�o
	nodo = fila->First;
	Prod = fila->First->valor;
	fila->First = fila->First->prox;

	if (fila->First == NULL)                    /// Caso fila esteja vazia, define ponteiro para o ultimo elemento como nulo
		fila->Last = NULL;

	free(nodo);
	fila->tamanho--;                            /// Diminui indicador do tamanho atual do Buffer
	pthread_cond_signal(&cond);                 /// Sinaliza que um elemento foi removido do Buffer
	pthread_mutex_unlock(&m);
	return Prod;
}

/// Fun��o usada para DEBUG
 void imprimeFila(Fila *fila) {
    Nodo *nodo;
    if (fila->First == NULL)
        puts("Fila Vazia!");
    else{
        printf("Fila atual: ");
        for(nodo = fila->First; nodo != NULL; nodo = nodo->prox)
            printf("%d ", nodo->valor);
        printf("Tamanho: %d\n",fila->tamanho);
    }
 }

///                    ///
///      PRODUTOR      ///
///     CONSUMIDOR     ///
///                    ///
void *Produtor(void *in){
    Dado *recebe = (Dado*)in;
    //int v = recebe->V;
    Fila *B = recebe->B;

    int temp;
    srand((int*)pthread_self() + clock());      /// Faz o seed para os n�meros randomicos n�o se repetirem

    pthread_mutex_lock(&n);
    while(NumberItems>0){
        NumberItems--;
        pthread_mutex_unlock(&n);
        temp = rand();
        printf("Produtor #%d: [%ld : %d]\n",t-NumberItems,pthread_self(),temp);
        Push(B, temp);                          /// Empurra para a fila o valor randomizado
        pthread_mutex_lock(&n);
    }
    pthread_mutex_unlock(&n);

    return NULL;                                /// T�rmino da thread
}

void *Consumidor(void *in){
    Dado *recebe = (Dado*)in;
    Fila *B = recebe->B;
    int valor;

    do{
        valor = Pop(B);                         /// Remove valor do Buffer
        if(valor!=-1)
            printf("Consumidor: [%ld : %d]\n",pthread_self(),valor);
    }while (valor != -1);                       /// Verifica se deve sair ou repetir o processo
    return NULL;                                /// T�rmino da thread
}
