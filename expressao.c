#define _POSIX_C_SOURCE 200809L 

#include "expressao.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <locale.h>

#define MAX_EXPRESSAO 512
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define GRAUS_PARA_RAD(g) ((g) * M_PI / 180.0f)


typedef struct { float itens[MAX_EXPRESSAO]; int topo; } PilhaFloat;
typedef struct { char *itens[MAX_EXPRESSAO]; int topo; } PilhaStr;
typedef struct { int itens[MAX_EXPRESSAO]; int topo; } PilhaInt;

enum {
    OP_SOMA, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_POW, OP_LOG, OP_SQRT, OP_SEN, OP_COS, OP_TAN, OP_RAIZ, OP_INVALIDO
};

void pushFloat(PilhaFloat *p, float v) { p->itens[++p->topo] = v; }
float popFloat(PilhaFloat *p) { return p->itens[p->topo--]; }
void pushStr(PilhaStr *p, char *str) { p->itens[++p->topo] = strdup(str); }
char *popStr(PilhaStr *p) { return (p->topo < 0) ? NULL : p->itens[p->topo--]; }
void clearPilhaStr(PilhaStr *p) { while (p->topo >= 0) free(popStr(p)); }
void pushInt(PilhaInt *p, int v) { p->itens[++p->topo] = v; }
int popInt(PilhaInt *p) { return p->itens[p->topo--]; }
void clearPilhaInt(PilhaInt *p) { p->topo = -1; }

int getOperadorId(const char *token) {
    if (!token) return OP_INVALIDO;
    if (strcmp(token, "+") == 0) return OP_SOMA;
    if (strcmp(token, "-") == 0) return OP_SUB;
    if (strcmp(token, "*") == 0) return OP_MUL;
    if (strcmp(token, "/") == 0) return OP_DIV;
    if (strcmp(token, "%") == 0) return OP_MOD;
    if (strcmp(token, "^") == 0) return OP_POW;
    if (strcmp(token, "log") == 0) return OP_LOG;
    if (strcmp(token, "sqrt") == 0) return OP_SQRT;
    if (strcmp(token, "raiz") == 0) return OP_RAIZ;
    if (strcmp(token, "sen") == 0) return OP_SEN;
    if (strcmp(token, "cos") == 0) return OP_COS;
    if (strcmp(token, "tan") == 0 || strcmp(token, "tg") == 0) return OP_TAN;
    return OP_INVALIDO;
}

int getPrecedence(const char *token) {
    if (!token || *token == '\0') return 0;
    if (strlen(token) == 1) {
        char op_char = token[0];
        if (op_char == '+' || op_char == '-') return 1;
        if (op_char == '*' || op_char == '/' || op_char == '%') return 2;
        if (op_char == '^') return 3;
    }
    if (getOperadorId(token) >= OP_LOG && getOperadorId(token) <= OP_RAIZ) return 4;
    return 0; 
}

int isNumber(const char *s) {
    if (!s || *s == '\0') return 0;
    char *endptr;

    strtof(s, &endptr);
    return *endptr == '\0' && s != endptr;
}

float potenciaInteira(float base, int expoente) {
    float resultado = 1.0f;
    if (expoente < 0) { base = 1.0f / base; expoente = -expoente; }
    for (int i = 0; i < expoente; i++) resultado *= base;
    return resultado;
}

void replaceCommaWithDot(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ',') {
            str[i] = '.';
        }
    }
}

char *getFormaPosFixa(char *Str) {
    static char resultado[MAX_EXPRESSAO * 2];
    resultado[0] = '\0';

    char *str_copy = strdup(Str);
    if (!str_copy) {
        fprintf(stderr, "Erro de memoria ao copiar string para posfixa.\n");
        return NULL;
    }
    replaceCommaWithDot(str_copy);

    PilhaStr operadores_pilha = {.topo = -1};

    char *current_pos = str_copy;
    char temp_token_buf[MAX_EXPRESSAO];
    int token_len = 0;

    while (*current_pos != '\0') {
        while (isspace((unsigned char)*current_pos)) {
            current_pos++;
        }
        if (*current_pos == '\0') break;

        token_len = 0;

        if (isdigit((unsigned char)*current_pos) || (*current_pos == '-' && isdigit((unsigned char)*(current_pos + 1))) || (*current_pos == '.' && isdigit((unsigned char)*(current_pos + 1)))) {
            if (*current_pos == '-' && isdigit((unsigned char)*(current_pos + 1))) {
                temp_token_buf[token_len++] = *current_pos++;
            }
            while (isdigit((unsigned char)*current_pos) || *current_pos == '.') {
                temp_token_buf[token_len++] = *current_pos++;
            }
            temp_token_buf[token_len] = '\0';
            strcat(resultado, temp_token_buf);
            strcat(resultado, " ");
        }

        else if (isalpha((unsigned char)*current_pos)) {
            const char *func_names[] = {"log", "sqrt", "raiz", "sen", "cos", "tan", "tg", NULL};
            int found_func = 0;

            for (int i = 0; func_names[i] != NULL; i++) {
                int len = strlen(func_names[i]);
                if (strncmp(current_pos, func_names[i], len) == 0) {

                    if (!isalnum((unsigned char)current_pos[len]) || isdigit((unsigned char)current_pos[len])) {
                        strncpy(temp_token_buf, func_names[i], len);
                        temp_token_buf[len] = '\0';
                        current_pos += len;
                        found_func = 1;
                        break;
                    }
                }
            }

            if (found_func) {
                pushStr(&operadores_pilha, temp_token_buf);
            } else {
                fprintf(stderr, "Erro: Token alfabetico desconhecido: '%.*s'\n", (int)strcspn(current_pos, " +-/%^*()"), current_pos);
                clearPilhaStr(&operadores_pilha);
                free(str_copy);
                return NULL;
            }
        }
 
        else if (*current_pos == '(') {
            pushStr(&operadores_pilha, "(");
            current_pos++;
        }

        else if (*current_pos == ')') {
            while (operadores_pilha.topo >= 0 && strcmp(operadores_pilha.itens[operadores_pilha.topo], "(") != 0) {
                char *op = popStr(&operadores_pilha);
                strcat(resultado, op);
                strcat(resultado, " ");
                free(op);
            }
            if (operadores_pilha.topo < 0) {
                fprintf(stderr, "Erro: Parentese de fechamento sem abertura correspondente.\n");
                clearPilhaStr(&operadores_pilha); free(str_copy); return NULL;
            }
            free(popStr(&operadores_pilha)); 

         
            if (operadores_pilha.topo >= 0) {
                int op_id = getOperadorId(operadores_pilha.itens[operadores_pilha.topo]);
                if (op_id >= OP_LOG && op_id <= OP_RAIZ) { 
                    char *func_op = popStr(&operadores_pilha);
                    strcat(resultado, func_op);
                    strcat(resultado, " ");
                    free(func_op);
                }
            }
            current_pos++;
        }

        else if (strchr("+-*/%^", *current_pos) != NULL) {
            char op_char[2] = {*current_pos, '\0'};
            int current_op_prec = getPrecedence(op_char);

            while (operadores_pilha.topo >= 0 && strcmp(operadores_pilha.itens[operadores_pilha.topo], "(") != 0 &&
                   (getPrecedence(operadores_pilha.itens[operadores_pilha.topo]) > current_op_prec ||
                   (getPrecedence(operadores_pilha.itens[operadores_pilha.topo]) == current_op_prec && op_char[0] != '^')) ) {
                char *op = popStr(&operadores_pilha);
                strcat(resultado, op);
                strcat(resultado, " ");
                free(op);
            }
            pushStr(&operadores_pilha, op_char);
            current_pos++;
        }
        else {
            fprintf(stderr, "Erro: Caractere invalido na expressao: '%c'\n", *current_pos);
            clearPilhaStr(&operadores_pilha); free(str_copy); return NULL;
        }
    }

    while (operadores_pilha.topo >= 0) {
        char *op = popStr(&operadores_pilha);
        if (strcmp(op, "(") == 0 || strcmp(op, ")") == 0) {
            fprintf(stderr, "Erro: Parentese desbalanceado (sobraram na pilha)!\n");
            free(op); clearPilhaStr(&operadores_pilha); free(str_copy); return NULL;
        }
        strcat(resultado, op); strcat(resultado, " "); free(op);
    }
    if (strlen(resultado) > 0 && resultado[strlen(resultado) - 1] == ' ') {
        resultado[strlen(resultado) - 1] = '\0';
    }

    free(str_copy);
    return resultado;
}

char *getFormaInFixa(char *Str) {
    static char resultado_infixa[MAX_EXPRESSAO * 2];
    PilhaStr pilha_str = {.topo = -1};
    PilhaInt pilha_prec = {.topo = -1};
    char *str_copy = strdup(Str);
    char *token;
    char *save_ptr = NULL;

    if (!str_copy) { fprintf(stderr, "Erro de memoria!\n"); return NULL; }
    replaceCommaWithDot(str_copy); 
    token = strtok_r(str_copy, " ", &save_ptr);
    while (token != NULL) {
        if (isNumber(token)) {
            pushStr(&pilha_str, token);
            pushInt(&pilha_prec, getPrecedence(token));
        } else {
            int op_id = getOperadorId(token);
            if (op_id == OP_INVALIDO) { fprintf(stderr, "Erro: Operador/Token invalido: '%s'!\n", token); clearPilhaStr(&pilha_str); clearPilhaInt(&pilha_prec); free(str_copy); return NULL; }
            
            if (op_id >= OP_LOG && op_id <= OP_RAIZ) { 
                if (pilha_str.topo < 0) { fprintf(stderr, "Erro: Operando ausente para funcao '%s'!\n", token); clearPilhaStr(&pilha_str); clearPilhaInt(&pilha_prec); free(str_copy); return NULL; }
                char *operand_a_str = popStr(&pilha_str); 
                popInt(&pilha_prec); 
                char temp_buf[MAX_EXPRESSAO * 2];
            
                snprintf(temp_buf, sizeof(temp_buf), "%s(%s)", token, operand_a_str);
                
                pushStr(&pilha_str, temp_buf);
                pushInt(&pilha_prec, getPrecedence(token));
                free(operand_a_str);
            } else { 
                if (pilha_str.topo < 1) { fprintf(stderr, "Erro: Operando(s) ausente(s) para '%s'!\n", token); clearPilhaStr(&pilha_str); clearPilhaInt(&pilha_prec); free(str_copy); return NULL; }
                
          
                char *operand_b_str = popStr(&pilha_str); 
                int prec_b = popInt(&pilha_prec);
                
             
                char *operand_a_str = popStr(&pilha_str); 
                int prec_a = popInt(&pilha_prec);

                char temp_formatted_a[MAX_EXPRESSAO * 2], temp_formatted_b[MAX_EXPRESSAO * 2], result_combined_buf[MAX_EXPRESSAO * 2];
                char op_char = token[0];
                int current_op_prec = getPrecedence(token);

                char *formatted_a_ptr = operand_a_str;
                char *formatted_b_ptr = operand_b_str;
                
              
                if (prec_a < current_op_prec || (prec_a == current_op_prec && op_char == '^')) {
                    snprintf(temp_formatted_a, sizeof(temp_formatted_a), "(%s)", operand_a_str);
                    formatted_a_ptr = temp_formatted_a;
                }
                if (prec_b < current_op_prec || (prec_b == current_op_prec && op_char != '^')) {
                    snprintf(temp_formatted_b, sizeof(temp_formatted_b), "(%s)", operand_b_str);
                    formatted_b_ptr = temp_formatted_b;
                }
                
                snprintf(result_combined_buf, sizeof(result_combined_buf), "%s %s %s", formatted_a_ptr, token, formatted_b_ptr);
                pushStr(&pilha_str, result_combined_buf);
                pushInt(&pilha_prec, current_op_prec);
                
                free(operand_a_str); free(operand_b_str);
            }
        }
        token = strtok_r(NULL, " ", &save_ptr); 
    }

    if (pilha_str.topo != 0 || pilha_prec.topo != 0) { 
        fprintf(stderr, "Erro: Expressao malformada (sobraram itens na pilha)!\n"); 
        clearPilhaStr(&pilha_str); 
        clearPilhaInt(&pilha_prec); 
        free(str_copy); 
        return NULL; 
    }
    char *res = popStr(&pilha_str);
    strncpy(resultado_infixa, res, sizeof(resultado_infixa) - 1); 
    resultado_infixa[sizeof(resultado_infixa) - 1] = '\0'; 
    free(res); 
    clearPilhaInt(&pilha_prec); 
    free(str_copy);
    return resultado_infixa;
}

float getValorPosFixa(char *StrPosFixa) {
    PilhaFloat pilha = {.topo = -1};
    char *str_copy = strdup(StrPosFixa);
    char *save_ptr = NULL;
    float final_result;

    if (!str_copy) { fprintf(stderr, "Erro de memoria!\n"); exit(1); }

  
    char *token = strtok_r(str_copy, " ", &save_ptr);
    while (token != NULL) {
        int op_id = getOperadorId(token);
        if (isNumber(token)) {
            pushFloat(&pilha, atof(token));
        } else {
            float a, b;
           
            if (op_id >= OP_LOG && op_id <= OP_RAIZ) {
                if (pilha.topo < 0) { fprintf(stderr, "Erro: Operando ausente para '%s'!\n", token); free(str_copy); exit(1); }
                a = popFloat(&pilha);
            } else { 
                if (pilha.topo < 1) { fprintf(stderr, "Erro: Operando(s) ausente(s) para '%s'!\n", token); free(str_copy); exit(1); }
                b = popFloat(&pilha); a = popFloat(&pilha); 
            }

            switch (op_id) {
                case OP_SOMA: pushFloat(&pilha, a + b); break;
                case OP_SUB:  pushFloat(&pilha, a - b); break;
                case OP_MUL:  pushFloat(&pilha, a * b); break;
                case OP_DIV:  if (b == 0.0f) { fprintf(stderr, "Erro: Divisao por zero!\n"); free(str_copy); exit(1); } pushFloat(&pilha, a / b); break;
                case OP_MOD:  pushFloat(&pilha, fmodf(a, b)); break;
                case OP_POW:  pushFloat(&pilha, (b == floorf(b) && b >= 0) ? potenciaInteira(a, (int)b) : powf(a, b)); break;
                case OP_LOG:  if (a <= 0.0f) { fprintf(stderr, "Erro: Logaritmo de numero nao positivo (%.2f)!\n", a); free(str_copy); exit(1); } pushFloat(&pilha, log10f(a)); break;
                case OP_SQRT: case OP_RAIZ: if (a < 0.0f) { fprintf(stderr, "Erro: Raiz quadrada de numero negativo (%.2f)!\n", a); free(str_copy); exit(1); } pushFloat(&pilha, sqrtf(a)); break;
                case OP_SEN:  pushFloat(&pilha, sinf(GRAUS_PARA_RAD(a))); break;
                case OP_COS:  pushFloat(&pilha, cosf(GRAUS_PARA_RAD(a))); break;
                case OP_TAN:  if (fabsf(fmodf(a, 180.0f) - 90.0f) < 0.0001f || fabsf(fmodf(a, 180.0f) + 90.0f) < 0.0001f) { fprintf(stderr, "Erro: Tangente de angulo invalido (%.2f graus)!\n", a); free(str_copy); exit(1); } pushFloat(&pilha, tanf(GRAUS_PARA_RAD(a))); break;
                default: fprintf(stderr, "Erro: Operador desconhecido '%s'!\n", token); free(str_copy); exit(1);
            }
        }
        token = strtok_r(NULL, " ", &save_ptr); 
    }

    if (pilha.topo != 0) { fprintf(stderr, "Erro: Expressao malformada (itens restantes na pilha)!\n"); free(str_copy); exit(1); }
    final_result = popFloat(&pilha);
    free(str_copy);
    return final_result;
}

float getValorInFixa(char *StrInFixa) {
    char *input_copy = strdup(StrInFixa);
    if (!input_copy) { fprintf(stderr, "Erro de memoria!\n"); exit(1); }
    
    replaceCommaWithDot(input_copy);

    char *posfixa_str = getFormaPosFixa(input_copy);
    if (!posfixa_str || posfixa_str[0] == '\0') { fprintf(stderr, "Erro: Falha na conversao para posfixa.\n"); free(input_copy); exit(1); }
    
    float valor = getValorPosFixa(posfixa_str);
    free(input_copy);
    return valor;
}