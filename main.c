#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h> 
#include "expressao.h" 

#define MAX_INPUT_LEN 512

int main() {
    setlocale(LC_NUMERIC, "C");

    char expressao_input[MAX_INPUT_LEN];
    int tipo_entrada;

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    while (1) { 
        printf("CALCULADORA\n");
        printf("Escolha o tipo de expressao de entrada:\n");
        printf("1. Expressao Infixa\n");
        printf("2. Expressao Pos-fixa\n");
        printf("---------------------------------------\n");
        printf("Digite sua escolha (1 ou 2, ou 'sair'/'exit' para fechar): ");
        
        char escolha_str[10];
        if (!fgets(escolha_str, sizeof(escolha_str), stdin)) {
            fprintf(stderr, "Erro ao ler a escolha.\n");
            continue; 
        }
    
        escolha_str[strcspn(escolha_str, "\n")] = 0;

        if (strcmp(escolha_str, "sair") == 0 || strcmp(escolha_str, "exit") == 0) {
            printf("Saindo da calculadora. Ate mais!\n");
            break; 
        }

        tipo_entrada = atoi(escolha_str); 

        if (tipo_entrada != 1 && tipo_entrada != 2) {
            printf("Escolha invalida. Por favor, digite 1 ou 2, ou 'sair'/'exit'.\n");
            continue; 
        }

        printf("---------------------------------------\n");
        printf("Digite a expressao (use '.' para decimais, ou ',' que sera convertido):\n> ");
        if (!fgets(expressao_input, sizeof(expressao_input), stdin)) {
            fprintf(stderr, "Erro ao ler a expressao.\n");
            continue; 
        }
        expressao_input[strcspn(expressao_input, "\n")] = 0;

        printf("\n--- Processando Expressao ---\n");
        printf("Expressao digitada: **%s**\n", expressao_input);

        char *forma_posfixa = NULL;
        char *forma_infixa = NULL;
        float resultado = 0.0f; 
        int erro_processamento = 0;

        char *expressao_input_copy = strdup(expressao_input);
        if (!expressao_input_copy) {
            fprintf(stderr, "Erro de memoria ao duplicar a expressao.\n");
            erro_processamento = 1;
        } else {
            if (tipo_entrada == 1) { 
                printf("Tipo de entrada: Infixa\n");
                printf("Expressao Infixa (entrada): %s\n", expressao_input_copy);
                
                forma_posfixa = getFormaPosFixa(expressao_input_copy);
                
                if (forma_posfixa && forma_posfixa[0] != '\0') {
                    printf("Convertida para Pos-fixa: %s\n", forma_posfixa);
                    resultado = getValorPosFixa(forma_posfixa);
                } else {
                    fprintf(stderr, "Erro: Nao foi possivel converter a expressao infixa para pos-fixa ou a expressao resultante esta vazia.\n");
                    erro_processamento = 1;
                }

            } else { 
                printf("Tipo de entrada: Pos-fixa\n");
                printf("Expressao Pos-fixa (entrada): %s\n", expressao_input_copy);
            
                forma_infixa = getFormaInFixa(expressao_input_copy); 
                
                if (forma_infixa && forma_infixa[0] != '\0') {
                    printf("Convertida para Infixa: %s\n", forma_infixa);
                    resultado = getValorPosFixa(expressao_input_copy); 
                } else {
                    fprintf(stderr, "Erro: Nao foi possivel converter a expressao pos-fixa para infixa ou a expressao resultante esta vazia.\n");
                    erro_processamento = 1;
                }
            }
        }
        
        if (expressao_input_copy) {
            free(expressao_input_copy);
            expressao_input_copy = NULL;
        }

        if (!erro_processamento) {
            printf("Resultado da expressao: %.4f\n", resultado);
        }
        printf("=======================================\n\n"); 
    } 

    return 0; 
}