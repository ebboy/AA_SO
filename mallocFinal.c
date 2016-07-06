#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#define TAM 20000

#define TAMTESTE 550
//250 350 550 1000



struct mymallocinfo{
    int memoriaTotalOcupada;
    int memoriaTotalLivre;
    int quantChunksOcupados;
    int quantChunksLivres;
};

static inline size_t alinha(size_t size) { //alinha
    return size + (sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);
}

struct chunk { // metadado 
    struct chunk *prox, *anterior;
    size_t        size;
    int           estado; // livre ou nao
    void         *data;
};

typedef struct chunk *Chunk; //ponteiro de metadado

static void *get_inicioHeap() {
    static Chunk b = NULL; // Cria bloco b = nulo
    if (!b) {
        b = sbrk(alinha(sizeof(struct chunk)));
        if (b == (void*) -1) {
            _exit(127);
        }
        b->prox = NULL;
        b->anterior = NULL;
        b->size = 0;
        b->estado = 0;
        b->data = NULL;
    }
    return b;
}


Chunk achar_bloco(size_t s, Chunk *heap) { // Passar para worst
    Chunk c = get_inicioHeap();
    int maior = 0;
    Chunk enderecoMaior = NULL;
    while(c){
        if(c->prox && c->prox->size > maior && c->prox->estado == 1){
            maior = c->prox->size;
            enderecoMaior = c->prox;
            
        
        }
        *heap = c;
        
        /* Encontra espaço que serve */
        if(c->prox && c->prox->size - (s + sizeof(struct chunk)) < 100 && c->prox->estado == 1){
                enderecoMaior = c->prox;
                break;
        }
        /* */
        c = c->prox;

    }
    return enderecoMaior;
}
  

void juntar_proximo(Chunk c) { // JUNTA BLOCOS
    c->size = c->size + c->prox->size + sizeof(struct chunk); // Tamanho do bloco total = tamanho atual + tamanho do proximo + tamanho do metadado
    c->prox = c->prox->prox; // Proximo atual = proximo do proximo
    if (c->prox) {
        c->prox->anterior = c; // Se existir proximo, proximo->anterior = c
    }
}

void dividir_proximo(Chunk c, size_t size) { // Dividir dois blocos (será usádo para dividir a parte não usada em outro bloco)
    Chunk novoChunk = (Chunk)((char*) c + size); // posição do novo chunk! endereço C (atual) + tamanho!
    novoChunk->anterior = c; // novo->anterior aponta para o C
    novoChunk->prox = c->prox; // novo->proximo aponta para o proximo do c (inserido no meio)
    novoChunk->size = c->size - size; // tamanho do novo chunk = tamanho do C - espaço usado
    novoChunk->estado = 1; // novo chunk setado como livre
    novoChunk->data = novoChunk + 1; // ponteiro de inicio do chunk = novoChunk + 1;
    if (c->prox) {
        c->prox->anterior = novoChunk;
    }
    c->prox = novoChunk; //proximo de C = novo C
    c->size = size - sizeof(struct chunk); // tamanho de C = tamanho usado - tamanho de bloco
}

void *mymalloc(size_t size) {
    if (!size) 
      return NULL;
    size_t espaco = alinha(size + sizeof(struct chunk)); // tamanho = size + metadado
    Chunk anterior = NULL; // Cria chunk anterior = NULL
    Chunk c = achar_bloco(size, &anterior); //Procura bloco com tamanho e armazena bloco anterior
    if (!c) { 
        espaco = alinha(TAM - sizeof(struct chunk));
        Chunk novoChunk = sbrk(espaco);
        if (novoChunk == (void*) -1) {
            return NULL;
        }                                                               // DAQUI PARA BAIXO, CRIA NOVO BLOCO
        novoChunk->prox = NULL;  
        novoChunk->anterior = anterior;
        novoChunk->size = espaco - sizeof(struct chunk);
        novoChunk->data = novoChunk + 1;
        anterior->prox = novoChunk;
        c = novoChunk;
        dividir_proximo(c, alinha(size + sizeof(struct chunk)));
        
    } else if (espaco + sizeof(size_t) < c->size) { // se achou um bloco com espaço, adiciona e dá split!
        dividir_proximo(c, espaco);
    }
    c->estado = 0; // ocupado
    return c->data; //retorna inicio do bloco
}

void myfree(void *ptr) {
    if (!ptr || ptr < get_inicioHeap() || ptr > sbrk(0)) // SE ELE NAO EXISTE  OU ESTÁ FORA DA HEAP
      return;
    Chunk c = (Chunk) ptr - 1;
    if (c->data != ptr) // confere se o data bate com o ponteiro para dizer se o endereço do free é o que queremos buscar trapezeira
      return; // não bate, retorna
    c->estado = 1; // bateu = free = 1
   
    

    if (c->prox && c->prox->estado) { // Se o C proximo existe e é livre = unir C com C próximo
        juntar_proximo(c);
    }
    if (c->anterior->estado) { // Se o C anterior é livre == unir C com C anterior
        juntar_proximo(c = c->anterior);
    }
   
}

void printaHeap(){
    Chunk c = get_inicioHeap();
    int cont = 0;
    int contLivres = 0;
    while (c){ // Encontra um bloco que serve FIRST FIT
        if(c->prox){
            c = c->prox;
        printf("tamanho do bloco %d:%lu   \t %s\n", cont, c->size + sizeof(struct chunk),c->estado == 1? "Livre":"Ocupado");
        if(c->estado == 1)
            contLivres++;
        
        cont++;
    }
        else
            break;
    }
    printf("Número de blocos livres:\t %d\n", contLivres);

}



struct mymallocinfo myMallocGerency(){
    struct mymallocinfo mymallocInformacoes;

    Chunk lido = get_inicioHeap();
    int memoriaTotalOcupada = 0;
    int memoriaTotalLivre = 0;
    int quantChunksOcupados = 0;
    int quantChunksLivres = 0;
    while(lido){
        
        if(lido->estado==0){
            memoriaTotalOcupada+=(lido->size)+40;
            quantChunksOcupados++;
        } else{
            memoriaTotalLivre+=(lido->size)+40;
            quantChunksLivres++;
        }

        lido=lido->prox;
    }
    mymallocInformacoes.memoriaTotalOcupada = memoriaTotalOcupada;
    mymallocInformacoes.memoriaTotalLivre = memoriaTotalLivre;
    mymallocInformacoes.quantChunksOcupados = quantChunksOcupados;
    mymallocInformacoes.quantChunksLivres = quantChunksLivres;
    return mymallocInformacoes;
}

int main(){


    int i;
    int tamanho = TAMTESTE;
    int vet[TAMTESTE]={0};
    int **vetorMalloc,**vetorMYmalloc ;
    int random,randomFree;
    
    //for(tamanho = 500; tamanho <=2000; tamanho += 500){


        printf("\n // ----- Teste com %d ----- //", tamanho );

        vetorMalloc = (int**)malloc(sizeof(int*)*tamanho);;
        vetorMYmalloc = (int**)mymalloc(sizeof(int*)*tamanho);
        for(i=1;i<=tamanho;i++){
            random = 5 + ( rand() % 95 );
            
            randomFree = 0+( rand() % i );
            

            vetorMalloc[i-1] = (int*)malloc(sizeof(int)*random);
            vetorMYmalloc[i-1] = (int*)mymalloc(sizeof(int)*random);


            if(random<50 && vet[randomFree]==0){
                free(vetorMalloc[randomFree]);
                myfree(vetorMYmalloc[randomFree]);
                vet[randomFree]=1;
            }
        }
        //printaHeap();

        struct mallinfo mallocInfo;
        struct mymallocinfo mymallocInfo;

        mallocInfo = mallinfo();
        mymallocInfo = myMallocGerency();

        printf("\n\n---- Informações do malloc ----\n");
        printf(" - Número de chunks livres:    \t %d\n", mallocInfo.ordblks);

        printf("\n---- Informações d myMalloc ----\n");
        printf(" - Número de chunks livres:    \t %d \n",mymallocInfo.quantChunksLivres);
        
        printf("\n---- Relação de chunks livres ----\n");
        float porcentagemMllcs = ((float) mallocInfo.ordblks / mymallocInfo.quantChunksLivres) * 100;
        printf("\n Porcentagem de chunks livres no malloc, comparando-o com myMalloc:  %.2f %% \n\n\n", porcentagemMllcs);


        for(i = 0; i < tamanho; i++){
            if(vet[i]==0){
                free(vetorMalloc[i]);
                myfree(vetorMYmalloc[i]);
                vet[randomFree]=1;
            }
        }
        free(vetorMalloc);
        myfree(vetorMYmalloc);





        //malloc_stats();
    //}


    return 0;
}
