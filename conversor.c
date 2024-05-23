#include "conversor.h"

int procurarTransicao(struct transition* transicoes, int *tam, const char *current_state, char symbol_read, char symbol_write, char movement, const char* dest_state) {
    for(int i = 0; i < *tam; i++) {
        if(!strcmp(current_state, transicoes[i].current_state) &&
            symbol_read == transicoes[i].symbol_read &&
            symbol_write == transicoes[i].symbol_write &&
            movement == transicoes[i].movement &&
            !strcmp(dest_state, transicoes[i].dest_state)) return 1;
    }

    return 0;
}

void mostrarTransicoes(struct transition* transicoes, int *tam) {
    printf("\n");
    for(int i = 0; i < *tam; i++) {
        printf("%s %c %c %c %s\n", transicoes[i].current_state,
                                    transicoes[i].symbol_read,
                                    transicoes[i].symbol_write,
                                    transicoes[i].movement,
                                    transicoes[i].dest_state);
    }
    printf("\n");
}

void renomearEstado(struct transition* transicoes, int *tam, const char *old_name, const char *new_name) {
    for(int i = 0; i < *tam; i++) {
        if(!strcmp(transicoes[i].current_state, old_name)) {
            strcpy(transicoes[i].current_state, new_name);
        }
        if(!strcmp(transicoes[i].dest_state, old_name)) {
            strcpy(transicoes[i].dest_state, new_name);
        }
    }
}

void novaTransicao(struct transition** transicoes, int *tam, const char *current_state, char symbol_read, char symbol_write, char movement, const char* dest_state) {
    // verificando se a transição já não existe
    if(procurarTransicao(*transicoes, tam, current_state, symbol_read, symbol_write, movement, dest_state)) return;
    
    (*tam)++;
    *transicoes = realloc(*transicoes, *tam * sizeof(struct transition));
    if(!*transicoes) {perror("falha ao criar nova transicao"); return;}
    
    strcpy((*transicoes)[*tam - 1].current_state, current_state);
    (*transicoes)[*tam - 1].symbol_read = symbol_read;
    (*transicoes)[*tam - 1].symbol_write = symbol_write;
    (*transicoes)[*tam - 1].movement = movement;
    strcpy((*transicoes)[*tam - 1].dest_state, dest_state);
    
    return;
}

void sipserToStandard_setup(struct transition** transicoes, int *tam) {
    // renomeando o estado inicial
    renomearEstado(*transicoes, tam, "0", "0_old");

    // novo estado inicial, marcação no início da fita
    novaTransicao(transicoes, tam, "0", '*', '*', 'l', "1_aux");
    novaTransicao(transicoes, tam, "1_aux", '_', '#', 'r', "0_old");

    return;
}

void delimitador_esquerdo(struct transition** transicoes, int *tam) {
    // procurando transições que movem o cabeçote para a esquerda
    for(int i = 0; i < *tam; i++) {
        if((*transicoes)[i].movement == 'l' && strcmp((*transicoes)->current_state, "0")) {
            // criando uma transição do estado destino para ele mesmo que verifica a marcação inicial
            novaTransicao(transicoes, tam, (*transicoes)[i].dest_state, '#', '#', 'r', (*transicoes)[i].dest_state);
        }
    }
    
    return;
}

void sipserToStandard(struct transition** transicoes, int *tam) {
    sipserToStandard_setup(transicoes, tam);
    delimitador_esquerdo(transicoes, tam);

    return;
}

void standardToSipser_setup(struct transition** transicoes, int *tam) {
    // renomeando o estado inicial
    renomearEstado(*transicoes, tam, "0", "0_old");

    // novo conjunto de estados/transições que antecedem o estado inicial antigo
    novaTransicao(transicoes, tam, "0", '0', '#', 'r', "1_aux");
    novaTransicao(transicoes, tam, "0", '1', '#', 'r', "2_aux");    

    novaTransicao(transicoes, tam, "1_aux", '0', '0', 'r', "1_aux");
    novaTransicao(transicoes, tam, "1_aux", '_', '0', 'r', "3_aux");
    novaTransicao(transicoes, tam, "1_aux", '1', '0', 'r', "2_aux");

    novaTransicao(transicoes, tam, "2_aux", '0', '1', 'r', "1_aux");
    novaTransicao(transicoes, tam, "2_aux", '1', '1', 'r', "2_aux");
    novaTransicao(transicoes, tam, "2_aux", '_', '1', 'r', "3_aux");
    
    novaTransicao(transicoes, tam, "3_aux", '_', '&', 'l', "4_aux");

    novaTransicao(transicoes, tam, "4_aux", '*', '*', 'l', "4_aux");
    novaTransicao(transicoes, tam, "4_aux", '#', '#', 'r', "0_old");

    return;        
}

int estados_sem_desvio(const char* current_state, const char* dest_state) {
    // verifica se o estado atual ou o estado destino da transição em questão não é um estado de desvio ou pertence ao setup
    if (!strstr(current_state, "delim_dir") && !strstr(dest_state, "delim_dir")
        && !strstr(current_state, "shift") && !strstr(dest_state, "shift")
        && !strstr(current_state, "aux") && !strstr(dest_state, "aux")
        && !strstr(current_state, "halt") && !strstr(dest_state, "halt")) return 1;

    return 0;
}

int existeDesvio(struct transition* transicoes, int *tam, const char* estado_inicial, const char* tipo_desvio) {
    // verifica se para o estado dado, não existe algum desvio do tipo especificado (delim_dir ou shift)
    for(int i = 0; i < *tam; i++) {
        if(!strcmp(transicoes[i].current_state, estado_inicial) && strstr(transicoes[i].dest_state, tipo_desvio)) return 1;
    }
    
    return 0;
}

void delimitador_esquerdo_standard(struct transition** transicoes, int *tam) {
    // para cada transição que vai para a esquerda, verificar, no estado destino, 
    // se não encontrou delimitador esquerdo
    for(int i = 0; i < *tam; i++) {
        if((*transicoes)[i].movement == 'l' 
            && estados_sem_desvio((*transicoes)[i].current_state, (*transicoes)[i].dest_state)
            && !existeDesvio(*transicoes, tam, (*transicoes)[i].dest_state, "shift")) {

            // criando o nome do conjunto de estados que fazem o deslocamento para a direita
            // (cada estado requer uma sub-rotina própria ou ocorre não-determinismo no retorno)        
            char num[4];
            snprintf(num, 4,"%d", i);
            
            char nome_shift_right[tam_nome_estado] = "shift_right_";
            strcat(nome_shift_right, num);
            char nome_shift_1[tam_nome_estado] = "shift_1_";
            strcat(nome_shift_1, num);
            char nome_shift_2[tam_nome_estado] = "shift_2_";
            strcat(nome_shift_2, num);
            char nome_shift_3[tam_nome_estado] = "shift_3_";
            strcat(nome_shift_3, num);
            char nome_shift_4[tam_nome_estado] = "shift_4_";
            strcat(nome_shift_4, num);
            char nome_shift_final[tam_nome_estado] = "shift_final_";
            strcat(nome_shift_final, num);

            // transição que leva pra rotina do deslocamento
            novaTransicao(transicoes, tam, (*transicoes)[i].dest_state, '#', '#', 'r', nome_shift_right);
            
            novaTransicao(transicoes, tam, nome_shift_right, '_', '_', 'r', nome_shift_1);
            novaTransicao(transicoes, tam, nome_shift_right, '0', '_', 'r', nome_shift_2);
            novaTransicao(transicoes, tam, nome_shift_right, '1', '_', 'r', nome_shift_3);

            novaTransicao(transicoes, tam, nome_shift_1, '_', '_', 'r', nome_shift_1);
            novaTransicao(transicoes, tam, nome_shift_1, '0', '_', 'r', nome_shift_2);
            novaTransicao(transicoes, tam, nome_shift_1, '1', '_', 'r', nome_shift_3);
            novaTransicao(transicoes, tam, nome_shift_1, '&', '_', 'r', nome_shift_4);

            novaTransicao(transicoes, tam, nome_shift_2, '_', '0', 'r', nome_shift_1);
            novaTransicao(transicoes, tam, nome_shift_2, '0', '0', 'r', nome_shift_2);
            novaTransicao(transicoes, tam, nome_shift_2, '1', '0', 'r', nome_shift_3);
            novaTransicao(transicoes, tam, nome_shift_2, '&', '0', 'r', nome_shift_4);

            novaTransicao(transicoes, tam, nome_shift_3, '_', '1', 'r', nome_shift_1);
            novaTransicao(transicoes, tam, nome_shift_3, '0', '1', 'r', nome_shift_2);
            novaTransicao(transicoes, tam, nome_shift_3, '1', '1', 'r', nome_shift_3);
            novaTransicao(transicoes, tam, nome_shift_3, '&', '1', 'r', nome_shift_4);
            
            novaTransicao(transicoes, tam, nome_shift_4, '_', '&', 'l', nome_shift_final);

            novaTransicao(transicoes, tam, nome_shift_final, '*', '*', 'l', nome_shift_final);

            // transição que traz de volta pro estado original
            novaTransicao(transicoes, tam, nome_shift_final, '#', '#', 'r', (*transicoes)[i].dest_state);
        }
    }

    
    
    return;
}

void delimitador_direito_standard(struct transition** transicoes, int *tam) {
    for(int i = 0; i < *tam; i++) {
        if((*transicoes)[i].movement == 'r' 
            && estados_sem_desvio((*transicoes)[i].current_state, (*transicoes)[i].dest_state)
            && !existeDesvio(*transicoes, tam, (*transicoes)[i].dest_state, "delim_dir")) {

            char num[4];
            snprintf(num, 4,"%d", i);

            char nome_delim_dir[tam_nome_estado] = "delim_dir_";
            strcat(nome_delim_dir, num);        
            
            novaTransicao(transicoes, tam, (*transicoes)[i].dest_state, '&', '_', 'r', nome_delim_dir);
            novaTransicao(transicoes, tam, nome_delim_dir, '_', '&', 'l', (*transicoes)[i].dest_state);
        }
    }
    
    return;
}

void standardToSipser(struct transition** transicoes, int *tam) {
    standardToSipser_setup(transicoes, tam);
    delimitador_esquerdo_standard(transicoes, tam);
    delimitador_direito_standard(transicoes, tam);

    return;
}