#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#define TAM 20000

static inline size_t word_align(size_t size) { //alinha
    return size + (sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);
}

struct chunk { // metadado 
    struct chunk *next, *prev;
    size_t        size;
    int           myfree; // livre ou nao
    void         *data;
};

typedef struct chunk *Chunk; //ponteiro de metadado

static void *mymalloc_base() {
    static Chunk b = NULL; // Cria bloco b = nulo
    if (!b) {
      	//b = sbrk(0);
      	//sbrk(word_align(sizeof(struct chunk)))
        b = sbrk(word_align(sizeof(struct chunk)));
        printf("%lu\n",sizeof(struct chunk));
        if (b == (void*) -1) {
            _exit(127);
        }
        b->next = NULL;
        b->prev = NULL;
        b->size = 0;
        b->myfree = 0;
        b->data = NULL;
    }
    return b;
}

Chunk mymalloc_chunk_find(size_t s, Chunk *heap) {
    Chunk c = mymalloc_base();
    printf("mymallocbase %p\n",c );
    
    while (c && (!c->myfree || c->size < s)){ // Encontra um bloco que serve FIRST FIT
    	*heap = c;
      c = c->next;
    }
  
    return c;
}

void mymalloc_merge_next(Chunk c) { // JUNTA BLOCOS
    c->size = c->size + c->next->size + sizeof(struct chunk); // Tamanho do bloco total = tamanho atual + tamanho do proximo + tamanho do metadado
    c->next = c->next->next; // Proximo atual = proximo do proximo
    if (c->next) {
        c->next->prev = c; // Se existir proximo, proximo->anterior = c
    }
}

void mymalloc_split_next(Chunk c, size_t size) { // Dividir dois blocos (será usádo para dividir a parte não usada em outro bloco)
    Chunk newc = (Chunk)((char*) c + size); // posição do novo chunk! endereço C (atual) + tamanho!
    newc->prev = c; // novo->anterior aponta para o C
    newc->next = c->next; // novo->proximo aponta para o proximo do c (inserido no meio)
    newc->size = c->size - size; // tamanho do novo chunk = tamanho do C - espaço usado
    newc->myfree = 1; // novo chunk setado como livre
    newc->data = newc + 1; // ponteiro de inicio do chunk = newc + 1;
    if (c->next) {
        c->next->prev = newc;
    }
    c->next = newc; //proximo de C = novo C
    c->size = size - sizeof(struct chunk); // tamanho de C = tamanho usado - tamanho de bloco
}

void *mymalloc(size_t size) {
    if (!size) 
      return NULL;
    size_t length = word_align(size + sizeof(struct chunk)); // tamanho = size + metadado
    Chunk prev = NULL; // Cria chunk anterior = NULL
    Chunk c = mymalloc_chunk_find(size, &prev); //Procura bloco com tamanho e armazena bloco anterior
    printf("%p",c);
    if (!c) { // mymalloc init não funciona no nosso!!!
        length = word_align(TAM - 2*sizeof(struct chunk));
        Chunk newc = sbrk(length);
        if (newc == (void*) -1) {
            return NULL;
        }																// DAQUI PARA BAIXO, CRIA NOVO BLOCO
        newc->next = NULL;  
        newc->prev = prev;
        newc->size = length - sizeof(struct chunk);
        newc->data = newc + 1;
        prev->next = newc;
        c = newc;
        mymalloc_split_next(c, length);
    } else if (length + sizeof(size_t) < c->size) { // se achou um bloco com espaço, adiciona e dá split!
        mymalloc_split_next(c, length);
    }
    c->myfree = 0; // ocupado
    return c->data; //retorna inicio do bloco
}

void myfree(void *ptr) {
    if (!ptr || ptr < mymalloc_base() || ptr > sbrk(0)) // SE ELE NAO EXISTE  OU ESTÁ FORA DA HEAP
      return;
    Chunk c = (Chunk) ptr - 1;
    if (c->data != ptr) // confere se o data bate com o ponteiro para dizer se o endereço do free é o que queremos buscar trapezeira
      return; // não bate, retorna
    c->myfree = 1; // bateu = free = 1
    printf("libertei\n");    printf("Liberado = %d\n", c->myfree );


    if (c->next && c->next->myfree) { // Se o C proximo existe e é livre = unir C com C próximo
        mymalloc_merge_next(c);
    }
    if (c->prev->myfree) { // Se o C anterior é livre == unir C com C anterior
        mymalloc_merge_next(c = c->prev);
    }
    if (!c->next) { // Se C proximo não existe, C anterior proximo = null
        c->prev->next = NULL;
        sbrk(- c->size - sizeof(struct chunk)); //  diminui a heap
    }
}


int main(){

    int *teste,*teste2;
    teste =mymalloc(sizeof(int)*10);
    int i;
    for(i=0;i<10;i++){
        teste[i] = i;
        printf("%d\n",teste[i] );
    }

    teste2= mymalloc(sizeof(int)*50);
     myfree(teste);
    for(i=0;i<50;i++){
        teste2[i] = i;
        printf("%d\n",teste2[i] );
    }

   

    //printf("%d\n", teste->myfree );

    return 0;
}