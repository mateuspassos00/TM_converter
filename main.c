/*********************************************************************************************** 

O programa faz a conversão entre o modelo de Sipser (fita limitada à esquerda) e o modelo padrão
(fita duplamente infinita). A saída será um arquivo 'arquivo.out' na pasta 'saidas'.

LINUX
- compilar: gcc main.c conversor.c -o conversorMT
- executar: ./conversorMT entradas/arquivo_entrada.txt

WINDOWS
- compilar: gcc main.c conversor.c -o conversorMT
- executar: .\conversorMT entradas\arquivo_entrada.txt

***********************************************************************************************/
#include "conversor.h"

int read_input_file(const char *type, struct transition **transicoes, int *tam, char *arquivo) {
    FILE *input_file = fopen(arquivo, "rt");
    if(!input_file) {perror("falha ao abrir arquivo de entrada"); return 0;}

    fscanf(input_file, "%s\n", type);
    while(!feof(input_file)) {
        (*tam)++;
        *transicoes = (struct transition*) realloc(*transicoes, *tam * sizeof(struct transition));
        if(!*transicoes) {perror("falha ao armazenar transicoes"); return 0;}
        
        fscanf(input_file, "%s %c %c %c %s\n", (*transicoes)[*tam - 1].current_state,
                                                &(*transicoes)[*tam - 1].symbol_read,
                                                &(*transicoes)[*tam - 1].symbol_write,
                                                &(*transicoes)[*tam - 1].movement,
                                                (*transicoes)[*tam - 1].dest_state);
    }
    
    fclose(input_file);

    return 1;
}

int write_output_file(struct transition* transicoes, int *tam, const char* type) {
    FILE *output_file = fopen("saidas/arquivo.out", "wt");
    if(!output_file) {perror("falha ao abrir arquivo de saida"); return 0;}
    
    if(type[1] == 'S') fprintf(output_file, ";I\n");
    else fprintf(output_file, ";S\n");
    for(int i = 0; i < *tam; i++) {
        fprintf(output_file, "%s %c %c %c %s\n", transicoes[i].current_state,
                                                transicoes[i].symbol_read,
                                                transicoes[i].symbol_write,
                                                transicoes[i].movement,
                                                transicoes[i].dest_state);
    }
    
    fclose(output_file);

    return 1;
}

int main(int argc, char *argv[]) {
    
    struct transition *transicoes = NULL;
    int tam = 0; // numero de transições
    char type[2]; // tipo da máquina

    // carregando as transições na memória
    if(!read_input_file(type, &transicoes, &tam, argv[1])) return 1;
    
    // checando o tipo de máquina e fazendo a conversão
    if (type[1] == 'S') sipserToStandard(&transicoes, &tam);
    else standardToSipser(&transicoes, &tam);

    // gravando as novas transições no arquivo de saída
    if(!write_output_file(transicoes, &tam, type)) return 2;
    
    // liberando memória
    free(transicoes);

    return 0;
}