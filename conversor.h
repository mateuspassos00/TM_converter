#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define tam_nome_estado 20

struct transition {
    char current_state[tam_nome_estado], symbol_read, symbol_write, movement, dest_state[tam_nome_estado];
};

int procurarTransicao(struct transition*, int *, const char *,
                        char, char, char, const char*);
void mostrarTransicoes(struct transition*, int *);
void renomearEstado(struct transition*, int *, const char *, const char *);
void novaTransicao(struct transition**, int *, const char *,
                        char, char, char, const char*);
void sipserToStandard_setup(struct transition**, int *);
void delimitador_esquerdo(struct transition**, int *);
void sipserToStandard(struct transition**, int *);
void standardToSipser(struct transition**, int *);
void standardToSipser_setup(struct transition**, int *);
void delimitador_esquerdo_standard(struct transition**, int *);
void delimitador_direito_standard(struct transition**, int *);
int existeDesvio(struct transition*, int *, const char*, const char*);
int estados_sem_desvio(const char*, const char*);