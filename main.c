#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

//  taille maximale des expressions et des piles
#define MAX_SIZE 100


//  1: FONCTIONS ET STRUCTURES DE PILE

//  1.1 Stack pour les Caractères (Opérateurs/Parenthèses)
typedef struct StackChar{
    char items[MAX_SIZE];
    int top;
} StackChar;

void StackChar_initialize(StackChar *s) {
     s->top = -1; }

int StackChar_isEmpty(StackChar *s) {
    return s->top == -1;
    }

void StackChar_push(StackChar *s, char item) {
    if (s->top >= MAX_SIZE - 1) {
        printf("Error: Stack Overflow\n");
        exit(EXIT_FAILURE);
    }
    s->items[++s->top] = item;
}
char StackChar_pop(StackChar *s) {
    if (StackChar_isEmpty(s)) {
        return '\0';
    }
    return s->items[s->top--];
}
char StackChar_peek(StackChar *s) {
    if (StackChar_isEmpty(s)) {
        return '\0';
    }
    return s->items[s->top];
}

// 1.2 Stack pour les Doubles (Valeurs/Calcul)
typedef struct StackDouble {
    double items[MAX_SIZE];
    int top;
} StackDouble;

void StackDouble_initialize(StackDouble *s) {
    s->top = -1;
    }

int StackDouble_isEmpty(StackDouble *s) {
    return s->top == -1;
    }

void StackDouble_push(StackDouble *s, double item) {
    if (s->top >= MAX_SIZE - 1) {
        printf("Error: Pile de valeurs pleine.\n");
        exit(EXIT_FAILURE);
    }
    s->items[++s->top] = item;
}

double StackDouble_pop(StackDouble *s) {
    if (StackDouble_isEmpty(s)) {
        fprintf(stderr, "Error: Stack is empty ( Postfixe syntax incorrect).\n");
        exit(EXIT_FAILURE);
    }
    return s->items[s->top--];
}

// 1.3 Fonctions Mathématiques

// Factoriel (pour entiers non négatifs)
double factorial(double n) {
    // Vérifie si n est un entier non négatif
    if (n < 0 || n != floor(n)) {
        printf("Error: Factorial not defined .\n");
        return NAN; // Retourne NAN en cas d'erreur
    }
    if (n == 0 || n == 1) return 1.0;

    // Calcul de la factorielle
    double result = 1.0;
    for (int i = 2; i <= (int)n; i++) {
        result *= i;
    }
    return result;
}

//1.4 Priorités des Opérateurs

// Le symbole '~' est utilisé pour la négation unaire (moins unaire)
// Le symbole 's' pour sin, 'c' pour cos.
int getPrecedence(char op) {
    switch (op) {
        case '!': return 5; // Factoriel est postfixe, mais haute priorité
        case '~': return 4; // Moins unaire (négation)
        case 's':
        case 'c': return 4; // Fonctions trigonométriques
        case '^': return 3;
        case '*':
        case '/': return 2;
        case '+':
        case '-': return 1;
        default:  return 0; // Parenthèse ou autre
    }
}

// Vérifie si un caractère est un opérateur binaire standard
int isBinaryOperator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^';
}


// SECTION 2: CONVERSION INFIXE -> POSTFIXE


int infixToPostfix(const char *infix, char *postfix) {
    StackChar stack;
    StackChar_initialize(&stack);
    int j = 0; // Index pour la chaîne postfixe

    // Pour distinguer le moins unaire du moins binaire, et pour les fonctions/parenthèses
    //expectOperand=1 donc on attend un nombre, parenthèse ouvrante, ou moins unaire
    int expectOperand = 1;

    for (int i = 0; infix[i] != '\0'; i++) {
        char token = infix[i];

        if (token == ' ') continue;

        //  Operand (Number)
        if (isdigit(token) || (token == '.' && (isdigit(infix[i+1])))) {
            postfix[j++] = token;
            while (isdigit(infix[i+1]) || infix[i+1] == '.') {
                i++;
                postfix[j++] = infix[i];
            }
            postfix[j++] = ' ';
            expectOperand = 0; // Vient de voir un opérande, prochain '-' est binaire
        }

        // Parenthèse Ouvrante/Fonction
        else if (token == '(') {
            StackChar_push(&stack, token);
            expectOperand = 1; // Après '(', on s'attend toujours à un opérande/unaire
        }
        // Détection des fonctions (sin, cos)
        else if (token == 's' && infix[i+1] == 'i' && infix[i+2] == 'n') {
            StackChar_push(&stack, 's');
            i += 2;
            expectOperand = 1;
        } else if (token == 'c' && infix[i+1] == 'o' && infix[i+2] == 's') {
            StackChar_push(&stack, 'c');
            i += 2;
            expectOperand = 1;
        }

        //  Opérateur
        else if (isBinaryOperator(token) || token == '!') {

            // 1. Détection du Moins Unaire (quand expectOperand=1)
            if (token == '-' && expectOperand) {
                token = '~'; // Remplacer par notre marqueur d'unaire
                // Moins unaire est suivi d'un opérande, donc expectOperand reste 1
            }

            // 2. Gestion de la Factorielle (Opérateur unaire postfixe)
            else if (token == '!') {
                // Le Factoriel s'applique toujours à un opérande qui le précède.
                // Il doit être empilé/dépilé immédiatement.
                // Il ne change pas l'état expectOperand (doit être 0 car il suit un nombre/parenthèse).
                if (expectOperand) {
                    printf("\nERREUR: Factorielle mal placée ou sans opérande précédent.\n");
                    return 1;
                }
                // Dépiler les opérateurs de priorité supérieure ou égale (pour factorielle, cela est trivial
                // car '!' est la plus haute priorité et l'associativité est de droite à gauche ou unaire).
            }

            // 3. Gestion des autres Opérateurs
            else { // Opérateur binaire standard
                expectOperand = 1; // Après un binaire, on attend un opérande
            }

            // Dépilement des opérateurs (y compris les unaires comme '~' et les fonctions)
            while (!StackChar_isEmpty(&stack) &&StackChar_peek(&stack) != '(' &&
                   (getPrecedence(StackChar_peek(&stack)) > getPrecedence(token) ||
                    (getPrecedence(StackChar_peek(&stack)) == getPrecedence(token) && token != '^' && token != '~' && token != '!' && StackChar_peek(&stack) != 's' && StackChar_peek(&stack) != 'c'))) {

                postfix[j++] = StackChar_pop(&stack);
                postfix[j++] = ' ';
            }
            StackChar_push(&stack, token);

        }

        // Parenthèse Fermante ')'
        else if (token == ')') {
            while (!StackChar_isEmpty(&stack) && StackChar_peek(&stack) != '(') {
                postfix[j++] = StackChar_pop(&stack);
                postfix[j++] = ' ';
            }
            if (StackChar_isEmpty(&stack)) {
                printf( "\nERREUR: Parenthèses déséquilibrées (manque '(').\n");
                return 1;
            }
            StackChar_pop(&stack); // Jeter le '('

            // Dépiler la fonction associée (s/c) si elle existe
            char next = StackChar_peek(&stack);
            if (!StackChar_isEmpty(&stack) && (next == 's' || next == 'c')) {
                postfix[j++] = StackChar_pop(&stack);
                postfix[j++] = ' ';
            }
            expectOperand = 0; // Après ')', on s'attend à un opérateur binaire ou à la fin
        }

        // --- Jeton non reconnu ---
        else {
            printf("\nERREUR: Jeton inattendu '%c'.\n", token);
            return 1;
        }
    }

    // --- Fin de l'expression : Dépiler le reste ---
    while (!StackChar_isEmpty(&stack)) {
        char topOp = StackChar_pop(&stack);
        if (topOp == '(') {
            printf("\nERREUR: Parenthèses déséquilibrées (manque ')').\n");
            return 1;
        }
        postfix[j++] = topOp;
        postfix[j++] = ' ';
    }

    if (j > 0 && postfix[j-1] == ' ') j--; // Retirer l'espace final
    postfix[j] = '\0';
    return 0; // Succès
}


//3: ÉVALUATION POSTFIXE


double evaluatePostfix(const char *postfix) {
    StackDouble stack;
    StackDouble_initialize(&stack);
    char *token;
    char temp_str[2 * MAX_SIZE]; // Taille double car la postfixe est plus longue
    strcpy(temp_str, postfix);

    token = strtok(temp_str, " ");

    while (token != NULL) {
        char op = token[0];

        // Si c'est un nombre (détecte les nombres négatifs tokenisés par atof)
        if (isdigit(op) || (op == '-' && isdigit(token[1])) || op == '.') {
            StackDouble_push(&stack, atof(token));
        }
        // Opérateurs unaires (Factoriel, Négation, Sinus, Cosinus)
        else if (op == '!' || op == '~' || op == 's' || op == 'c'){
            if (StackDouble_isEmpty(&stack)) {
                 printf("Erreur d'évaluation: Opérateur '%c' sans opérande.\n", op);
                 return NAN;
            }
            double val = StackDouble_pop(&stack);
            double result = NAN;

            if (op == '!') {
                result = factorial(val);
            } else if (op == '~') {
                result = -val;
            } else if (op == 's') {
                result = sin(val);
            } else if (op == 'c'){
                result = cos(val);
            }

            if (isnan(result)) return NAN; // Propagation de l'erreur
            StackDouble_push(&stack, result);
        }
        // Opérateurs binaires
        else if (isBinaryOperator(op)) {
            if (StackDouble_isEmpty(&stack) || StackDouble_isEmpty(&stack)) {
                 printf( "Erreur d'évaluation: Opérateur binaire sans deux opérandes.\n");
                 return NAN;
            }
            double val2 = StackDouble_pop(&stack);
            double val1 = StackDouble_pop(&stack);
            double result = 0.0;

            switch (op) {
                case '+': result = val1 + val2; break;
                case '-': result = val1 - val2; break;
                case '*': result = val1 * val2; break;
                case '/':
                    if (val2 == 0.0) {
                        printf( "\nERREUR CRITIQUE: Division par zéro.\n");
                        return NAN;
                    }
                    result = val1 / val2;
                    break;
                case '^': result = pow(val1, val2); break;
                default:
                    printf("Opérateur inconnu dans l'évaluation.\n");
                    return NAN;
            }
            StackDouble_push(&stack, result);
        }

        token = strtok(NULL, " ");
    }

    if (StackDouble_isEmpty(&stack)) {
        // Cela devrait arriver si l'expression était vide
        return 0.0;
    }

    // Le résultat final est le seul élément restant
    double final_result = StackDouble_pop(&stack);

    // Si la pile n'est pas vide, c'est une expression malformée (trop d'opérandes)
    if (!StackDouble_isEmpty(&stack)) {
         printf("Erreur: Expression postfixe malformée (trop d'opérandes).\n");
         return NAN;
    }

    return final_result;
}



//  4: ARBRE D'EXPRESSION BINAIRE

typedef enum{
    OPERAND,
    UNAIRY_OPERATION,
    BINAIRY_OPERATION,
} TypeNode;

typedef struct Node {
    TypeNode type;
    union {
        double valeur;
        char symbole[10];
    } data;
    struct Node *LC;
    struct Node *RC;
} Node;

typedef struct StackElement {
    Node *node;
    struct StackElement *next;
} StackElement;

typedef struct StackT{
    StackElement *top;
} StackT;

void StackT_initialize(StackT *S) { S->top = NULL; }
int StackT_isEmpty(StackT *S) { return S->top == NULL; }

void StackT_push(StackT *S, Node *n) {
    StackElement *newNode = (StackElement*)malloc(sizeof(StackElement));
    if (!newNode) {
       printf("Erreur d'allocation memoire");
       exit(EXIT_FAILURE);
    }
    newNode->node = n;
    newNode->next = S->top;
    S->top = newNode;
}

Node* StackT_pop(StackT *S) {
    if (StackT_isEmpty(S)) {
        printf("Erreur, la pile de nœuds est vide.\n");
        return NULL;
    }
    StackElement *temp = S->top;
    Node *node = temp->node;
    S->top = temp->next;
    free(temp);
    return node;
}

Node *createNode(TypeNode type, const char *symbole, double val) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) {
        printf("Erreur d'allocation memoire pour le nœud");
        exit(EXIT_FAILURE);
    }
    n->type = type;
    n->LC = NULL;
    n->RC = NULL;

    if (type == OPERAND) {
        n->data.valeur = val;
    } else {
        strncpy(n->data.symbole, symbole, 9);
        n->data.symbole[9] = '\0';
    }
    return n;
}

void freeTree(Node *root) {
    if (root != NULL) {
        freeTree(root->LC);
        freeTree(root->RC);
        free(root);
    }
}

int isUnaryToken(const char *token) {
    return (strcmp(token, "!") == 0 || strcmp(token, "~") == 0 ||
            strcmp(token, "s") == 0 || strcmp(token, "c") == 0);
}

int isBinaryToken(const char *token) {
    return (strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
            strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
            strcmp(token, "^") == 0);
}

int isOperandToken(const char *token) {
    char *endptr;
    // Tente de lire un double
    strtod(token, &endptr);
    // Valide si l'intégralité de la chaîne a été lue et n'était pas vide
    return (*endptr == '\0' && token[0] != '\0');
}

Node *constructTree(const char *PostExp) {
    char copiedExp[2 * MAX_SIZE]; // Taille augmentée
    strncpy(copiedExp, PostExp, 2 * MAX_SIZE - 1);
    copiedExp[2 * MAX_SIZE - 1] = '\0';

    StackT S;
    StackT_initialize(&S);

    char *token = strtok(copiedExp, " ");

    while (token != NULL) {
        Node *newNode = NULL;

        if (isOperandToken(token)) {
            double val = strtod(token, NULL);
            newNode = createNode(OPERAND, NULL, val);
            StackT_push(&S, newNode);
        } else if (isBinaryToken(token)) {
            if (StackT_isEmpty(&S) || StackT_isEmpty(S.top->next)) {
                printf("Erreur de construction de l'AEB: Opérateur binaire sans deux opérandes.\n");

                return NULL;
            }
            newNode = createNode(BINAIRY_OPERATION, token, 0);
            newNode->RC = StackT_pop(&S);
            newNode->LC = StackT_pop(&S);
            StackT_push(&S, newNode);
        } else if (isUnaryToken(token)) {
            if (StackT_isEmpty(&S)) {
                printf("Erreur de construction de l'AEB: Opérateur unaire sans opérande.\n");
                return NULL;
            }
            newNode = createNode(UNAIRY_OPERATION, token, 0);
            newNode->RC = StackT_pop(&S); // L'opérande unique est l'enfant Droit
            StackT_push(&S, newNode);
        } else {
            printf("Erreur: Token non reconnu dans l'expression postfixe pour l'AEB: %s\n", token);
            return NULL;
        }

        token = strtok(NULL, " ");
    }

    Node *root = StackT_pop(&S);
    if (!StackT_isEmpty(&S)) {
        printf("Erreur: Expression postfixe malformée (trop d'opérandes laissés dans la pile).\n");
        freeTree(root);
        return NULL;
    }

    return root;
}


double evaluateTree(Node *root) {
    if (root == NULL) {
        return NAN; // Utiliser NAN pour indiquer une expression vide ou un sous-arbre invalide
    }

    if (root->type == OPERAND) {
        return root->data.valeur;
    }

    // Évaluation récursive des enfants (Post-ordre)
    double RIGHTval = evaluateTree(root->RC);
    // Vérification précoce de l'erreur
    if (isnan(RIGHTval)) return NAN;

    // Seul l'opérateur binaire a besoin de l'enfant gauche
    double LEFTval = 0.0;
    if (root->type == BINAIRY_OPERATION) {
        LEFTval = evaluateTree(root->LC);
        if (isnan(LEFTval)) return NAN;
    }

    // Cas 2 : Opérateurs Binaires
    if (root->type == BINAIRY_OPERATION) {
        char op = root->data.symbole[0];
        switch (op) {
            case '+': return LEFTval + RIGHTval;
            case '-': return LEFTval - RIGHTval;
            case '*': return LEFTval * RIGHTval;
            case '/':
                if (RIGHTval == 0.0) {
                    printf("Erreur arithmetique: Division par zero detectee.\n");
                    return NAN;
                }
                return LEFTval / RIGHTval;
            case '^': return pow(LEFTval, RIGHTval);
            default:
                printf("Erreur: Operateur binaire inconnu: %s\n", root->data.symbole);
                return NAN;
        }
    }

    // Cas 3 : Opérateurs Unaires (Fonctions)
    if (root->type == UNAIRY_OPERATION) {
        if (strcmp(root->data.symbole, "~") == 0) {
            return -RIGHTval;
        } else if (strcmp(root->data.symbole, "!") == 0) {
            return factorial(RIGHTval);
        } else if (strcmp(root->data.symbole, "s") == 0) {
            return sin(RIGHTval);
        } else if (strcmp(root->data.symbole, "c") == 0) {
            return cos(RIGHTval);
        } else {
            printf("Erreur: Operateur unaire inconnu: %s\n", root->data.symbole);
            return NAN;
        }
    }
    return NAN;
}



// 5: MAIN


int main() {
    char infix_input[MAX_SIZE];
    char postfix_result[2 * MAX_SIZE];

    printf("Entrez l'expression infixe (max %d caractères) : \n", MAX_SIZE - 1);

    if (fgets(infix_input, MAX_SIZE, stdin) == NULL) {
        printf("Erreur de lecture de l'entrée.\n");
        return 1;
    }
    infix_input[strcspn(infix_input, "\n")] = 0; // Retire le '\n'

    printf("Expression infixe : %s\n", infix_input);

    // --- CONVERSION INFIXE -> POSTFIXE ---
    if (infixToPostfix(infix_input, postfix_result) == 0) {
        printf("\nRésultat Postfixe : %s\n", postfix_result);

        // --- ÉVALUATION POSTFIXE ---
        double final_result_eval = evaluatePostfix(postfix_result);
        if (!isnan(final_result_eval)) {
            printf("Résultat de l'évaluation (directe) : %.4f\n", final_result_eval);
        } else {
            printf("Évaluation directe échouée (voir les messages d'erreur).\n");
        }

        printf("\n------------------------------------------------------\n");

        // --- CONSTRUCTION ET ÉVALUATION AEB ---
        Node *root = constructTree(postfix_result);

        if (root != NULL) {
            printf("Construction de l'arbre binaire réussie.\n");

            double resultat_tree = evaluateTree(root);

            if (!isnan(resultat_tree)) {
                printf("Résultat de l'évaluation (par AEB) : %.4f\n", resultat_tree);
            } else {
                printf("Évaluation par arbre échouée (voir les messages d'erreur).\n");
            }
            freeTree(root); // Libérer la mémoire
        } else {
            printf("Échec de la construction de l'arbre.\n");
        }
    } else {
        printf("Échec de la conversion Infixe -> Postfixe.\n");
    }

    return 0;
}
