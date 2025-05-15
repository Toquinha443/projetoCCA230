// *******************************************
// DECLARAÇÕES E INCLUDES
// *******************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define MAX_HEAP 20  // capacidade máxima da estrutura de Heap (fila prioritária)

// *******************************************
// DEFINIÇÕES DE ESTRUTURAS
// *******************************************

// Estrutura para representar uma data (dia, mês e ano)
typedef struct {
    int dia;
    int mes;
    int ano;
} Data;

// Estrutura para armazenar os dados de um paciente
typedef struct {
    char nome[100];
    int idade;
    char rg[20];
    Data *entrada;  // data de entrada do paciente (ponteiro para estrutura Data)
} Registro;

// Elemento da lista encadeada de pacientes (célula contendo um Registro)
typedef struct ELista {
    Registro *dados;
    struct ELista *proximo;
} ELista;

// Estrutura da lista encadeada de pacientes cadastrados
typedef struct {
    ELista *inicio;
    int qtde;
} Lista;

// Elemento da fila de atendimento (célula duplamente encadeada contendo um Registro)
typedef struct EFila {
    struct EFila *proximo;
    struct EFila *anterior;
    Registro *dados;
} EFila;

// Estrutura da fila de atendimento comum
typedef struct {
    EFila *head;  // início da fila (primeiro elemento)
    EFila *tail;  // fim da fila (último elemento)
    int qtde;
} Fila;

// Estrutura de Heap (fila de prioridade) para atendimento prioritário
typedef struct {
    Registro *dados[MAX_HEAP];  // array de ponteiros para registros (pacientes)
    int qtde;
} Heap;

// Nó da árvore binária de busca (ABB) para pesquisa de pacientes
typedef struct EABB {
    Registro *dados;
    struct EABB *filhoEsq;
    struct EABB *filhoDir;
} EABB;

// Estrutura da árvore binária de busca de pacientes
typedef struct {
    EABB *raiz;
    int qtde;
} ABB;

// Elemento da pilha de operações (usado para funcionalidade de desfazer operações)
typedef struct Cell {
    struct Cell *anterior;
    struct Cell *proximo;
    char operacao;       // código da operação ('E' = Enfileirar, 'D' = Desenfileirar)
    Registro *paciente;  // ponteiro para o paciente envolvido na operação
} Cell;

// Estrutura da pilha para registrar operações (desfazer enfileiramento/desenfileiramento)
typedef struct {
    Cell *top;
    int qtde;
} Stack;

// *******************************************
// FUNÇÕES PRINCIPAIS POR MÓDULO (CADASTRO, ATENDIMENTO, ETC.)
// *******************************************

// ** Módulo Cadastro de Pacientes ** 

// Inicializa a lista encadeada de pacientes (aloca Lista e define valores iniciais)
Lista* inicializa_lista() {
    Lista *novaLista = malloc(sizeof(Lista));
    if (novaLista != NULL) {
        novaLista->inicio = NULL;
        novaLista->qtde = 0;
    }
    return novaLista;
}

// Cria uma nova estrutura de Data com dia, mês e ano informados
Data* cria_data(int dia, int mes, int ano) {
    Data *novaData = malloc(sizeof(Data));
    if (novaData != NULL) {
        novaData->dia = dia;
        novaData->mes = mes;
        novaData->ano = ano;
    }
    return novaData;
}

// Insere um novo paciente no início da lista de pacientes cadastrados
void cadastrar_paciente(Lista *lista, Registro paciente) {
    // Aloca um novo nó para a lista e copia os dados do paciente para ele
    ELista *novoNo = malloc(sizeof(ELista));
    novoNo->dados = malloc(sizeof(Registro));
    *novoNo->dados = paciente;
    // Insere o novo nó no início (cabeça) da lista encadeada
    novoNo->proximo = lista->inicio;
    lista->inicio = novoNo;
    lista->qtde++;
}

// Imprime todos os pacientes presentes na lista de cadastrados
void imprimir_lista(const Lista *lista) {
    if (lista->inicio == NULL) {
        limpar_console();
        printf("\nNenhum paciente cadastrado.\nAdicione algum e tente novamente!\n");
        return;
    }
    limpar_console();
    printf("\n%d Pacientes cadastrados:\n\n",lista->qtde);
    // Percorre a lista encadeada e imprime os dados de cada paciente
    for (ELista *noAtual = lista->inicio; noAtual != NULL; noAtual = noAtual->proximo) {
        printf("Nome: %s; Idade: %d; RG: %s; Entrada: %02d/%02d/%04d\n",
               noAtual->dados->nome, noAtual->dados->idade, noAtual->dados->rg,
               noAtual->dados->entrada->dia, noAtual->dados->entrada->mes, noAtual->dados->entrada->ano);
    }
}

// Busca na lista um paciente pelo nome. Retorna o ponteiro para o nó do paciente ou NULL se não encontrado.
ELista* consultar_paciente_nome(const Lista *lista, const char *nome) {
    ELista *noAtual = lista->inicio;
    // Percorre a lista comparando os nomes
    while (noAtual != NULL) {
        if (strcmp(noAtual->dados->nome, nome) == 0) {
            return noAtual;  // paciente encontrado
        }
        noAtual = noAtual->proximo;
    }
    return NULL;  // paciente não encontrado
}
ELista* consultar_paciente_rg(const Lista *lista, const char *rg) {
    ELista *noAtual = lista->inicio;

    char rg_busca[20];  // RG tratado do parâmetro
    extrair_numeros_rg(rg, rg_busca);

    while (noAtual != NULL) {
        char rg_paciente[20];  // RG tratado do paciente na lista
        extrair_numeros_rg(noAtual->dados->rg, rg_paciente);

        if (strcmp(rg_paciente, rg_busca) == 0) {
            return noAtual;  // paciente encontrado
        }
        noAtual = noAtual->proximo;
    }

    return NULL;  // paciente não encontrado
}

// Atualiza os dados de um paciente existente na lista de cadastrados
void atualizar_paciente(Lista *lista) {
    char rgPaciente[100];
    printf("\nDigite o RG do paciente: ");
    fgets(rgPaciente, sizeof(rgPaciente), stdin);
    rgPaciente[strcspn(rgPaciente, "\n")] = '\0';  // remove o newline do final da string

    ELista *noEncontrado = consultar_paciente_rg(lista, rgPaciente);
    if (noEncontrado == NULL) {
        limpar_console();
        printf("\nERRO!\nNão existe paciente com esse RG cadastrado.\n");
        limpar_console_dinamico();
        return;
    }
    // Menu de opções de atualização
    int opcaoAtualizacao;
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║     QUAL INFORMAÇÃO DESEJA ATUALIZAR?      ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║ 1 - Nome                                   ║\n");
    printf("║ 2 - Idade                                  ║\n");
    printf("║ 3 - RG                                     ║\n");
    printf("║ 4 - Data de entrada                        ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\nSelecione uma opção: ");
    scanf("%d", &opcaoAtualizacao);
    getchar();  // consome o '\n' deixado pelo scanf

    switch (opcaoAtualizacao) {
        case 1:
            printf("Digite o novo NOME: ");
            fgets(noEncontrado->dados->nome, sizeof(noEncontrado->dados->nome), stdin);
            noEncontrado->dados->nome[strcspn(noEncontrado->dados->nome, "\n")] = '\0';
            break;
        case 2:
            printf("Digite a nova IDADE: ");
            scanf("%d", &noEncontrado->dados->idade);
            getchar();
            break;
        case 3:
            printf("Digite o novo RG: ");
            fgets(noEncontrado->dados->rg, sizeof(noEncontrado->dados->rg), stdin);
            noEncontrado->dados->rg[strcspn(noEncontrado->dados->rg, "\n")] = '\0';
            break;
        case 4: {
            int dia, mes, ano;
            printf("Digite a nova data de ENTRADA (dd mm aaaa): ");
            scanf("%d %d %d", &dia, &mes, &ano);
            getchar();
            // Atualiza os campos da data de entrada do paciente encontrado
            noEncontrado->dados->entrada->dia = dia;
            noEncontrado->dados->entrada->mes = mes;
            noEncontrado->dados->entrada->ano = ano;
            break;
        }
        default:
            limpar_console();
            printf("\nERRO!\nOpção inválida, favor escolher outra\n");
            limpar_console_dinamico();
            return;
    }
    limpar_console();
    printf("\nSUCESSO! Dados do paciente atualizados!\n");
    limpar_console_dinamico();
}

// Remove um paciente da lista de cadastrados pelo nome
void remover_paciente(Lista *lista, const char *nome) {
    ELista *noAtual = lista->inicio;
    ELista *noAnterior = NULL;
    // Percorre a lista até encontrar o paciente ou chegar ao final
    while (noAtual != NULL) {
        if (strcmp(noAtual->dados->nome, nome) == 0) {
            // Se encontrado, retira o nó da lista encadeada
            if (noAnterior == NULL) {
                // Removendo o primeiro nó da lista
                lista->inicio = noAtual->proximo;
            } else {
                // Removendo um nó do meio ou fim da lista
                noAnterior->proximo = noAtual->proximo;
            }
            // Libera a memória alocada para a data e para o nó removido
            free(noAtual->dados->entrada);
            free(noAtual);
            lista->qtde--;
            limpar_console();
            printf("\nSUCESSO!\nExclusão de %s realizada.\n", nome);
            limpar_console_dinamico();
            return;
        }
        // Avança para o próximo nó
        noAnterior = noAtual;
        noAtual = noAtual->proximo;
    }
    limpar_console();
    printf("ERRO!\nNão existe paciente com esse NOME cadastrado.\n");
    limpar_console_dinamico();
}

// ** Módulo Desfazer Operações (Pilha) ** 

// Cria uma nova célula da pilha de operações com o código da operação fornecido
Cell* start_cell(char operacao) {
    Cell *novaCelula = malloc(sizeof(Cell));
    if (novaCelula != NULL) {
        novaCelula->anterior = NULL;
        novaCelula->proximo = NULL;
        novaCelula->operacao = operacao;
        novaCelula->paciente = NULL;
    }
    return novaCelula;
}

// Inicializa a pilha de operações (desfazimento) vazia
Stack* start_stack() {
    Stack *novaPilha = malloc(sizeof(Stack));
    if (novaPilha != NULL) {
        novaPilha->top = NULL;
        novaPilha->qtde = 0;
    }
    return novaPilha;
}

// Empilha uma nova operação na pilha (registrando enfileiramento ou desenfileiramento)
void push(Stack *pilha, char operacao, Registro *paciente) {
    Cell *novaCelula = start_cell(operacao);
    novaCelula->paciente = paciente;
    // Coloca a nova célula no topo da pilha
    novaCelula->proximo = pilha->top;
    if (pilha->top != NULL) {
        pilha->top->anterior = novaCelula;
    }
    pilha->top = novaCelula;
    pilha->qtde++;
}

// Desempilha a última operação da pilha e retorna o ponteiro para a célula removida (ou NULL se vazia)
Cell* pop(Stack *pilha) {
    if (pilha->qtde == 0) {
        return NULL;
    }
    Cell *removida = pilha->top;
    pilha->top = removida->proximo;
    if (pilha->top != NULL) {
        pilha->top->anterior = NULL;
    }
    pilha->qtde--;
    return removida;
}

// Imprime o histórico de operações armazenado na pilha (do topo para a base)
void imprimir_stack(const Stack *pilha) {
    printf("\nHistórico de operações:\n");
    if (pilha->top == NULL) {
        printf("(Nenhuma operação registrada.)\n");
        return;
    }
    // Percorre a pilha imprimindo cada operação
    for (Cell *celulaAtual = pilha->top; celulaAtual != NULL; celulaAtual = celulaAtual->proximo) {
        switch (celulaAtual->operacao) {
            case 'E':
                printf("Enfileiramento de %s\n", celulaAtual->paciente ? celulaAtual->paciente->nome : "(desconhecido)");
                break;
            case 'D':
                printf("Desenfileiramento de %s\n", celulaAtual->paciente ? celulaAtual->paciente->nome : "(desconhecido)");
                break;
            default:
                printf("Operação desconhecida\n");
                break;
        }
    }
    printf("\n");
}

// Desfaz a última operação realizada na fila de atendimento comum, se possível
void desfazer_ultima_operacao(Stack *pilha, Fila *fila) {
    Cell *ultimaOperacao = pop(pilha);
    if (ultimaOperacao == NULL) {
        limpar_console();
        printf("\nERRO!\nNão temos operações para reverter.\n");
        limpar_console_dinamico();
        return;
    }
    // Verifica qual operação foi registrada e desfaz de acordo
    switch (ultimaOperacao->operacao) {
        case 'E': {  // Desfazer enfileiramento (remover último da fila)
            EFila *ultimoNo = fila->tail;
            if (ultimoNo != NULL) {
                // Remove o último paciente da fila
                fila->tail = ultimoNo->anterior;
                if (fila->tail != NULL) {
                    fila->tail->proximo = NULL;
                } else {
                    fila->head = NULL;
                }
                free(ultimoNo);
                fila->qtde--;
                limpar_console();
                printf("\nSUCESSO!\nÚltimo paciente adicionado a fila foi retirado.\n");
                limpar_console_dinamico();
            }
            break;
        }
        case 'D': {  // Desfazer desenfileiramento (recolocar paciente na frente da fila)
            EFila *novoNo = malloc(sizeof(EFila));
            novoNo->dados = ultimaOperacao->paciente;
            novoNo->anterior = NULL;
            novoNo->proximo = fila->head;
            if (fila->head != NULL) {
                fila->head->anterior = novoNo;
            } else {
                fila->tail = novoNo;
            }
            fila->head = novoNo;
            fila->qtde++;
            limpar_console();
            printf("\nSUCESSO!\nÚltimo paciente removido da fila foi realocado nela.\n");
            limpar_console_dinamico();
            break;
        }
        default:
            limpar_console();
            printf("\nERRO DESCONHECIDO!\n");
            limpar_console_dinamico();
            break;
    }
    // Libera a memória da célula de operação removida da pilha
    free(ultimaOperacao);
}

// ** Módulo Atendimento (Fila Comum) ** 

// Inicializa a estrutura de fila de atendimento vazia
Fila* inicializa_fila() {
    Fila *novaFila = malloc(sizeof(Fila));
    if (novaFila != NULL) {
        novaFila->head = NULL;
        novaFila->tail = NULL;
        novaFila->qtde = 0;
    }
    return novaFila;
}

// Insere (enfileira) um paciente da lista de cadastrados na fila de atendimento comum
void enfileirar_paciente(Lista *lista, Fila *fila, Stack *pilhaOperacoes) {
    char nomeBusca[100];
    printf("\nDigite o NOME do paciente que deseja adicionar a fila: ");
    fgets(nomeBusca, sizeof(nomeBusca), stdin);
    nomeBusca[strcspn(nomeBusca, "\n")] = '\0';
    // Verifica se o paciente existe na lista de cadastrados
    ELista *pacienteEncontrado = consultar_paciente_nome(lista, nomeBusca);
    if (pacienteEncontrado == NULL) {
        limpar_console();
        printf("\nERRO!\nNão existe paciente com esse NOME cadastrado.\n");
        limpar_console_dinamico();
        return;
    }
    // Duplica os dados do paciente encontrado para não alterar o cadastro original
    Registro *copiaRegistro = malloc(sizeof(Registro));
    *copiaRegistro = *(pacienteEncontrado->dados);
    // Cria um novo nó de fila para o paciente e insere no final da fila
    EFila *novoNoFila = malloc(sizeof(EFila));
    novoNoFila->dados = copiaRegistro;
    novoNoFila->proximo = NULL;
    novoNoFila->anterior = fila->tail;
    if (fila->qtde == 0) {
        // Se a fila estava vazia, novo nó é tanto head quanto tail
        fila->head = novoNoFila;
        fila->tail = novoNoFila;
    } else {
        // Liga o novo nó após o antigo último elemento
        fila->tail->proximo = novoNoFila;
        fila->tail = novoNoFila;
    }
    fila->qtde++;
    // Registra a operação de enfileiramento na pilha de operações para possibilidade de desfazer
    push(pilhaOperacoes, 'E', copiaRegistro);
    limpar_console();
    printf("\nSUCESSO!\nPaciente %s adicionado à fila de atendimento.\n", copiaRegistro->nome);
    limpar_console_dinamico();
}

// Remove (desenfileira) o primeiro paciente da fila de atendimento comum e o atende
void desenfileirar_paciente(Fila *fila, Stack *pilhaOperacoes) {
    if (fila->qtde == 0) {
        limpar_console();
        printf("\nERRO!\nNão há pacientes na fila de atendimento.\n");
        limpar_console_dinamico();
        return;
    }
    // Remove o nó do início da fila (head)
    EFila *removerNo = fila->head;
    fila->head = removerNo->proximo;
    if (fila->head != NULL) {
        fila->head->anterior = NULL;
    } else {
        // Se a fila ficou vazia, ajusta o ponteiro tail
        fila->tail = NULL;
    }
    // Registra a operação de desenfileiramento na pilha, guardando o paciente atendido
    push(pilhaOperacoes, 'D', removerNo->dados);
    limpar_console();
    printf("\nSUCESSO!\nPaciente %s atendido\n", removerNo->dados->nome);
    limpar_console_dinamico();
    // Libera o nó removido (mas não libera os dados do paciente, pois podem ser usados para desfazer)
    free(removerNo);
    fila->qtde--;
}

// Exibe todos os pacientes atualmente na fila de atendimento comum, na ordem de chegada
void mostrar_fila(const Fila *fila) {
    if (fila->qtde == 0) {
        limpar_console();
        printf("\nERRO!\nA fila de atendimento está vazia.\n");
        limpar_console_dinamico();
        return;
    }
    limpar_console();
    printf("Pacientes na fila de atendimento:\n");
    int posicao = 1;
    // Percorre a fila do head ao tail imprimindo os pacientes em sequência
    for (EFila *noAtual = fila->head; noAtual != NULL; noAtual = noAtual->proximo) {
        printf("%d. Nome: %s; Idade: %d; RG: %s; Entrada: %02d/%02d/%04d\n",
               posicao++, noAtual->dados->nome, noAtual->dados->idade, noAtual->dados->rg,
               noAtual->dados->entrada->dia, noAtual->dados->entrada->mes, noAtual->dados->entrada->ano);
    }
}

// ** Módulo Atendimento Prioritário (Heap) ** 

// Calcula o índice do filho esquerdo no heap, dado o índice do pai
int filho_esquerda(int indicePai) {
    return 2 * indicePai + 1;
}

// Calcula o índice do filho direito no heap, dado o índice do pai
int filho_direita(int indicePai) {
    return 2 * indicePai + 2;
}

// Calcula o índice do pai no heap, dado o índice de um filho
int pai(int indiceFilho) {
    return (indiceFilho - 1) / 2;
}

// Função auxiliar para manter a propriedade do heap (reorganiza a partir de um índice pai)
void peneirar(Heap *heap, int indicePai) {
    int maior = indicePai;
    int indiceEsq = filho_esquerda(indicePai);
    int indiceDir = filho_direita(indicePai);
    // Compara o pai com o filho esquerdo
    if (indiceEsq < heap->qtde && heap->dados[indiceEsq]->idade > heap->dados[maior]->idade) {
        maior = indiceEsq;
    }
    // Compara o pai (ou maior até agora) com o filho direito
    if (indiceDir < heap->qtde && heap->dados[indiceDir]->idade > heap->dados[maior]->idade) {
        maior = indiceDir;
    }
    // Se um dos filhos for maior que o pai, realiza a troca e continua o processo recursivamente
    if (maior != indicePai) {
        Registro *temp = heap->dados[indicePai];
        heap->dados[indicePai] = heap->dados[maior];
        heap->dados[maior] = temp;
        peneirar(heap, maior);
    }
}

// (Re)constrói o heap a partir dos dados atuais, garantindo a propriedade de max-heap
void construir(Heap *heap) {
    // Ajusta a partir dos nós internos (metade inicial do array)
    for (int i = (heap->qtde / 2) - 1; i >= 0; i--) {
        peneirar(heap, i);
    }
}

// Inicializa a estrutura de heap (fila prioritária) vazia
void inicializar_heap(Heap *heap) {
    heap->qtde = 0;
    for (int i = 0; i < MAX_HEAP; i++) {
        heap->dados[i] = NULL;
    }
}

// Insere um paciente na fila prioritária (heap), utilizando a idade como critério de prioridade (maior idade = maior prioridade)
void inserir_heap(Heap *heap, Registro *paciente) {
    if (heap->qtde >= MAX_HEAP) {
        limpar_console();
        printf("\nFila prioritária cheia (capacidade máxima atingida).\n");
        limpar_console_dinamico();
        return;
    }
    // Insere o novo paciente no final do array e então reorganiza o heap
    heap->dados[heap->qtde] = paciente;
    heap->qtde++;
    construir(heap);
    // (Nota: como usamos um max-heap de idade, o paciente de maior idade ficará na posição 0)
}

// Remove o paciente com maior prioridade (mais idoso) do heap e o considera atendido
void remover_heap(Heap *heap) {
    if (heap->qtde == 0) {
        limpar_console();
        printf("\nERRO!\nNão há pacientes na fila prioritária.\n");
        limpar_console_dinamico();
        return;
    }

    // O paciente mais idoso está no topo do heap
    Registro *atendido = heap->dados[0];
    limpar_console();
    printf("Paciente prioritário atendido: %s (Idade: %d)\n", atendido->nome, atendido->idade);
    limpar_console_dinamico();

    // Substitui a raiz pelo último elemento e reduz a quantidade
    heap->dados[0] = heap->dados[heap->qtde - 1];
    heap->qtde--;

    // Reorganiza o heap para manter o paciente mais velho no topo
    construir(heap);
}

// Mostra todos os pacientes presentes na fila de atendimento prioritário (heap)
void mostrar_heap(const Heap *heap) {
    if (heap->qtde == 0) {
        limpar_console();
        printf("\nERRO!\nFila Prioritária Vazia.\n");
        limpar_console_dinamico();
        return;
    }
    limpar_console();
    printf("\nPacientes na fila prioritária:\n");
    // Percorre o array do heap mostrando os pacientes em cada posição (não necessariamente em ordem de prioridade)
    for (int i = 0; i < heap->qtde; i++) {
        Registro *p = heap->dados[i];
        printf("%d. Nome: %s; Idade: %d; RG: %s; Entrada: %02d/%02d/%04d\n",
               i + 1, p->nome, p->idade, p->rg, p->entrada->dia, p->entrada->mes, p->entrada->ano);
    }
    limpar_console_dinamico();
}

// ** Módulo Pesquisa de Pacientes (ABB) ** 

// Cria uma nova árvore binária de busca vazia
ABB* cria_abb() {
    ABB *novaABB = malloc(sizeof(ABB));
    if (novaABB != NULL) {
        novaABB->raiz = NULL;
        novaABB->qtde = 0;
    }
    return novaABB;
}

// Cria um novo nó (vértice) da ABB a partir de um registro de paciente
EABB* cria_vertice(Registro paciente) {
    EABB *novoVertice = malloc(sizeof(EABB));
    if (novoVertice != NULL) {
        novoVertice->dados = malloc(sizeof(Registro));
        *(novoVertice->dados) = paciente;
        novoVertice->filhoEsq = NULL;
        novoVertice->filhoDir = NULL;
    }
    return novoVertice;
}

// Insere um paciente na ABB de acordo com um critério de comparação fornecido
void inserir_abb(ABB *arvore, Registro paciente, int (*criterio)(Registro, Registro)) {
    EABB *novoNo = cria_vertice(paciente);
    if (arvore->raiz == NULL) {
        // Insere na raiz se a árvore estiver vazia
        arvore->raiz = novoNo;
    } else {
        // Percorre a árvore até encontrar a posição de inserção adequada
        EABB *atual = arvore->raiz;
        EABB *anterior = NULL;
        while (atual != NULL) {
            anterior = atual;
            if (criterio(paciente, *(atual->dados)) <= 0) {
                atual = atual->filhoEsq;
            } else {
                atual = atual->filhoDir;
            }
        }
        // Insere o novo nó como filho esquerdo ou direito do último nó visitado
        if (criterio(paciente, *(anterior->dados)) <= 0) {
            anterior->filhoEsq = novoNo;
        } else {
            anterior->filhoDir = novoNo;
        }
    }
    arvore->qtde++;
}

// Realiza o percurso in-order (ordem simétrica) na ABB e imprime os pacientes em ordem crescente de acordo com o critério atual
void imprimir_in_ordem(EABB *raiz) {
    if (raiz != NULL) {
        imprimir_in_ordem(raiz->filhoEsq);
        printf("Nome: %s; Idade: %d; RG: %s; Entrada: %02d/%02d/%04d\n",
               raiz->dados->nome, raiz->dados->idade, raiz->dados->rg,
               raiz->dados->entrada->dia, raiz->dados->entrada->mes, raiz->dados->entrada->ano);
        imprimir_in_ordem(raiz->filhoDir);
    }
}

// Funções de comparação para dois registros de paciente, usadas na ordenação da ABB
int comparar_por_ano(Registro a, Registro b) {
    return a.entrada->ano - b.entrada->ano;
}
int comparar_por_mes(Registro a, Registro b) {
    return a.entrada->mes - b.entrada->mes;
}
int comparar_por_dia(Registro a, Registro b) {
    return a.entrada->dia - b.entrada->dia;
}
int comparar_por_idade(Registro a, Registro b) {
    return a.idade - b.idade;
}

// Reconstrói a ABB a partir da lista de pacientes usando um critério de comparação especificado
void reconstruir_abb(Lista *lista, ABB *arvore, int (*criterio)(Registro, Registro)) {
    // Remove nós anteriores (não implementado – poderia ser adicionado se necessário)
    // Percorre a lista encadeada e insere cada paciente na ABB de acordo com o critério
    for (ELista *noAtual = lista->inicio; noAtual != NULL; noAtual = noAtual->proximo) {
        inserir_abb(arvore, *(noAtual->dados), criterio);
    }
}

// ** Módulo Arquivos (Carregar/Salvar Dados) ** 

// Salva todos os pacientes da lista em um arquivo de texto (nome especificado)
void salvar_lista(Lista *lista, const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "w");
    if (arquivo == NULL) {
        limpar_console();
        printf("\nERRO!\nDesculpe, tivemos problemas para acessar a base de clientes\n");
        limpar_console_dinamico();
        return;
    }
    // Escreve cada paciente em uma linha do arquivo, com campos separados por delimitadores
    for (ELista *noAtual = lista->inicio; noAtual != NULL; noAtual = noAtual->proximo) {
        fprintf(arquivo, "Nome: %s; Idade: %d; RG: %s; Entrada: %02d/%02d/%04d\n",
                noAtual->dados->nome, noAtual->dados->idade, noAtual->dados->rg,
                noAtual->dados->entrada->dia, noAtual->dados->entrada->mes, noAtual->dados->entrada->ano);
    }
    fclose(arquivo);
    limpar_console();
    printf("\nSUCESSO!\nBase de pacientes atualizada!\n");
    limpar_console_dinamico();
}

// Carrega os pacientes de um arquivo de texto para a lista (substituindo os dados atuais da lista)
void carregar_lista(Lista *lista, const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        printf("\nERRO!\nDesculpe, tivemos problemas ao acessar a base de clientes\n");
        limpar_console_dinamico();
        return;
    }
    char linha[256];
    // Lê o arquivo linha por linha, criando um novo registro para cada linha
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        Registro novoRegistro;
        int dia, mes, ano;
        // Extrai os campos do paciente da linha formatada
        sscanf(linha, "Nome: %[^;]; Idade: %d; RG: %[^;]; Entrada: %d/%d/%d",
               novoRegistro.nome, &novoRegistro.idade, novoRegistro.rg, &dia, &mes, &ano);
        // Remove espaços e quebras de linha ao final do nome e do RG
        size_t len;
        len = strlen(novoRegistro.nome);
        while (len > 0 && (novoRegistro.nome[len-1] == ' ' || novoRegistro.nome[len-1] == '\n')) {
            novoRegistro.nome[len-1] = '\0';
            len--;
        }
        len = strlen(novoRegistro.rg);
        while (len > 0 && (novoRegistro.rg[len-1] == ' ' || novoRegistro.rg[len-1] == '\n')) {
            novoRegistro.rg[len-1] = '\0';
            len--;
        }
        // Cria a estrutura Data para a data de entrada e atribui ao novo registro
        novoRegistro.entrada = cria_data(dia, mes, ano);
        // Insere o novo registro na lista encadeada de pacientes
        cadastrar_paciente(lista, novoRegistro);
    }
    fclose(arquivo);
    limpar_console();
    printf("\nSUCESSO!\nDados importados!\n");
    limpar_console_dinamico();
}

// ** Módulo Sobre ** 

// Mostra as informações sobre o projeto e seus autores
void mostrar_sobre() {
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║               SOBRE O PROJETO              ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║ Nome: Rafael Simões                        ║\n");
    printf("║ RA: 11.122.284-0                           ║\n");
    printf("║ Ciclo: 7º SEM                              ║\n");
    printf("║ Curso: Engenharia de Robôs                 ║\n");
    printf("║ Disciplina: CCA230                         ║\n");
    printf("║ Data: 14/05/2025                           ║\n");
    printf("╠────────────────────────────────────────────╣\n");
    printf("║ Nome: Caroline Barroso                     ║\n");
    printf("║ RA: 11.122.398-8                           ║\n");
    printf("║ Ciclo: 7º SEM                              ║\n");
    printf("║ Curso: Engenharia de Robôs                 ║\n");
    printf("║ Disciplina: CCA230                         ║\n");
    printf("║ Data: 14/05/2025                           ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    limpar_console_dinamico();
}

// ** Módulo Limpeza ** 

// Limpa direto
void limpar_console() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Limpa dinâmico
void limpar_console_dinamico() {
    printf("\n");
    for (int i = 5; i >= 1; i--) {
        printf("Limpando em %d", i);
        fflush(stdout);  // força imprimir antes de esperar
        for (int j = 0; j < 3; j++) {
            usleep(300000);  // 300ms entre pontos
            printf(".");
            fflush(stdout);
        }
        usleep(300000);  // pausa final antes de apagar a linha
        printf("\r");     // volta o cursor para o início da linha
        printf("                     \r");  // apaga a linha (espaços + retorno)
    }

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ** Módulo Padronização ** 

void extrair_numeros_rg(const char *rg_original, char *rg_numerico) {
    int j = 0;
    for (int i = 0; rg_original[i] != '\0'; i++) {
        if (isdigit(rg_original[i])) {
            rg_numerico[j++] = rg_original[i];
        }
    }
    rg_numerico[j] = '\0';
}

// *******************************************
// FUNÇÕES AUXILIARES
// *******************************************
// (As funções auxiliares de apoio, como funções de comparação, ajustes de heap, etc., 
// foram implementadas junto com os módulos correspondentes acima para manter a coesão do código.)

// *******************************************
// FUNÇÃO PRINCIPAL (main)
// *******************************************
int main() {
    // Inicialização das estruturas principais
    Lista *listaPacientes = inicializa_lista();
    Fila *filaAtendimento = inicializa_fila();
    Heap *filaPrioritaria = malloc(sizeof(Heap));
    inicializar_heap(filaPrioritaria);
    Stack *pilhaOperacoes = start_stack();

    int opcaoMenuPrincipal;
    do {
        // Exibição do menu principal de opções
        limpar_console();
        printf("\n╔════════════════════════════════╗\n");
        printf("║        MENU PRINCIPAL          ║\n");
        printf("╠════════════════════════════════╣\n");
        printf("║ 1 - Cadastrar Pacientes        ║\n");
        printf("║ 2 - Atendimento (Fila Comum)   ║\n");
        printf("║ 3 - Atendimento Prioritário    ║\n");
        printf("║ 4 - Pesquisa de Pacientes      ║\n");
        printf("║ 5 - Desfazer Operação          ║\n");
        printf("║ 6 - Carregar/Salvar Dados      ║\n");
        printf("║ 7 - Sobre                      ║\n");
        printf("║ 0 - Sair                       ║\n");
        printf("╚════════════════════════════════╝\n");
        printf("\nSelecione uma opção: ");
        scanf("%d", &opcaoMenuPrincipal);
        getchar();  // limpa o buffer de entrada (newline)
        switch (opcaoMenuPrincipal) {
            case 1: {
                // Submenu de Cadastro de Pacientes
                int opcaoCadastro;
                do {
                    limpar_console();
                    printf("\n╔══════════════════════════════════════╗\n");
                    printf("║     OPÇÕES CADASTRO DO PACIENTE      ║\n");
                    printf("╠══════════════════════════════════════╣\n");
                    printf("║ 1 - Novo cadastro de paciente        ║\n");
                    printf("║ 2 - Consultar paciente por nome      ║\n");
                    printf("║ 3 - Atualizar cadastro de paciente   ║\n");
                    printf("║ 4 - Remover paciente                 ║\n");
                    printf("║ 5 - Listar todos os pacientes        ║\n");
                    printf("║ 0 - Voltar ao menu principal         ║\n");
                    printf("╚══════════════════════════════════════╝\n");
                    printf("\nSelecione uma opção: ");
                    scanf("%d", &opcaoCadastro);
                    getchar();
                    switch (opcaoCadastro) {
                        case 1: {
                            // Cadastrar um novo paciente
                            Registro novoPaciente;
                            int dia, mes, ano;
                            printf("\nNome: ");
                            fgets(novoPaciente.nome, sizeof(novoPaciente.nome), stdin);
                            novoPaciente.nome[strcspn(novoPaciente.nome, "\n")] = '\0';
                            printf("Idade: ");
                            scanf("%d", &novoPaciente.idade);
                            getchar();
                            printf("RG: ");
                            fgets(novoPaciente.rg, sizeof(novoPaciente.rg), stdin);
                            novoPaciente.rg[strcspn(novoPaciente.rg, "\n")] = '\0';
                            printf("Data de entrada (dd mm aaaa): ");
                            scanf("%d %d %d", &dia, &mes, &ano);
                            getchar();
                            novoPaciente.entrada = cria_data(dia, mes, ano);
                            cadastrar_paciente(listaPacientes, novoPaciente);
                            limpar_console();
                            printf("\nSUCESSO!\nPaciente cadastrado!\n");
                            limpar_console_dinamico();
                            break;
                        }
                        case 2: {
                            // Consultar um paciente pelo nome
                            char nomeBusca[100];
                            printf("\nNome do paciente para consulta: ");
                            fgets(nomeBusca, sizeof(nomeBusca), stdin);
                            nomeBusca[strcspn(nomeBusca, "\n")] = '\0';
                            ELista *resultado = consultar_paciente_nome(listaPacientes, nomeBusca);
                            if (resultado != NULL) {
                                printf("\nPaciente encontrado: %s | Idade: %d | RG: %s | Entrada: %02d/%02d/%04d\n",
                                       resultado->dados->nome, resultado->dados->idade, resultado->dados->rg,
                                       resultado->dados->entrada->dia, resultado->dados->entrada->mes, resultado->dados->entrada->ano);
                                limpar_console_dinamico();
                            } else {
                                limpar_console();
                                printf("\nERRO!\nPaciente não encontrado.\n");
                                limpar_console_dinamico();
                            }
                            break;
                        }
                        case 3:
                            // Atualizar cadastro de um paciente
                            atualizar_paciente(listaPacientes);
                            break;
                        case 4: {
                            // Remover um paciente do cadastro
                            char nomeRem[100];
                            printf("\nNome do paciente para remover: ");
                            fgets(nomeRem, sizeof(nomeRem), stdin);
                            nomeRem[strcspn(nomeRem, "\n")] = '\0';
                            remover_paciente(listaPacientes, nomeRem);
                            break;
                        }
                        case 5:
                            // Listar todos os pacientes cadastrados
                            imprimir_lista(listaPacientes);
                            limpar_console_dinamico();
                            break;
                        case 0:
                            printf("\nVoltando ao menu principal...\n");
                            break;
                        default:
                            printf("\nOpção inválida. Tente novamente.\n");
                    }
                } while (opcaoCadastro != 0);
            } break;
            case 2: {
                // Submenu de Atendimento (Fila Comum)
                int opcaoAtend;
                do {
                    limpar_console();
                    printf("\n╔════════════════════════════════════╗\n");
                    printf("║         MENU ATENDIMENTO           ║\n");
                    printf("╠════════════════════════════════════╣\n");
                    printf("║ 1 - Enfileirar paciente            ║\n");
                    printf("║ 2 - Atender paciente               ║\n");
                    printf("║ 3 - Mostrar fila de atendimento    ║\n");
                    printf("║ 0 - Voltar ao menu principal       ║\n");
                    printf("╚════════════════════════════════════╝\n");
                    printf("\nSelecione uma opção: ");
                    scanf("%d", &opcaoAtend);
                    getchar();
                    switch (opcaoAtend) {
                        case 1:
                            enfileirar_paciente(listaPacientes, filaAtendimento, pilhaOperacoes);
                            break;
                        case 2:
                            desenfileirar_paciente(filaAtendimento, pilhaOperacoes);
                            break;
                        case 3:
                            mostrar_fila(filaAtendimento);
                            limpar_console_dinamico();
                            break;
                        case 0:
                            printf("\nVoltando ao menu principal...\n");
                            break;
                        default:
                            printf("\nOpção inválida. Tente novamente.\n");
                            limpar_console_dinamico();
                    }
                } while (opcaoAtend != 0);
            } break;
            case 3: {
                // Submenu de Atendimento Prioritário (Fila com Heap)
                int opcaoPri;
                do {
                    limpar_console();
                    printf("\n╔════════════════════════════════════════════╗\n");
                    printf("║       MENU ATENDIMENTO PRIORITÁRIO         ║\n");
                    printf("╠════════════════════════════════════════════╣\n");
                    printf("║ 1 - Adicionar paciente à fila prioritária  ║\n");
                    printf("║ 2 - Atender paciente prioritário           ║\n");
                    printf("║ 3 - Mostrar fila prioritária               ║\n");
                    printf("║ 0 - Voltar ao menu principal               ║\n");
                    printf("╚════════════════════════════════════════════╝\n");
                    printf("\nSelecione uma opção: ");
                    scanf("%d", &opcaoPri);
                    getchar();
                    switch (opcaoPri) {
                        case 1: {
                            // Enfileirar um paciente na fila prioritária
                            char nomeBusca[100];
                            printf("\nNome do paciente para prioridade: ");
                            fgets(nomeBusca, sizeof(nomeBusca), stdin);
                            nomeBusca[strcspn(nomeBusca, "\n")] = '\0';
                            ELista *pacientePri = consultar_paciente_nome(listaPacientes, nomeBusca);
                            if (pacientePri == NULL) {
                                limpar_console();
                                printf("\nERRO!\nPaciente não encontrado no cadastro.\n");
                                limpar_console_dinamico();
                            } else {
                                inserir_heap(filaPrioritaria, pacientePri->dados);
                                limpar_console();
                                printf("\nPaciente %s inserido na fila prioritária.\n", pacientePri->dados->nome);
                                limpar_console_dinamico();
                            }
                            break;
                        }
                        case 2:
                            // Atender (remover) paciente prioritário da fila
                            remover_heap(filaPrioritaria);
                            break;
                        case 3:
                            // Mostrar fila de atendimento prioritário
                            mostrar_heap(filaPrioritaria);
                            break;
                        case 0:
                            printf("\nVoltando ao menu principal...\n");
                            break;
                        default:
                            printf("\nOpção inválida. Tente novamente.\n");
                            limpar_console_dinamico();
                    }
                } while (opcaoPri != 0);
            } break;
            case 4: {
                // Submenu de Pesquisa de Pacientes (ABB)
                int opcaoPesq;
                do {
                    limpar_console();
                    printf("\n╔════════════════════════════════════════════╗\n");
                    printf("║              MENU PESQUISA                 ║\n");
                    printf("╠════════════════════════════════════════════╣\n");
                    printf("║ 1 - Listar pacientes por ano de entrada    ║\n");
                    printf("║ 2 - Listar pacientes por mês de entrada    ║\n");
                    printf("║ 3 - Listar pacientes por dia de entrada    ║\n");
                    printf("║ 4 - Listar pacientes por idade             ║\n");
                    printf("║ 0 - Voltar ao menu principal               ║\n");
                    printf("╚════════════════════════════════════════════╝\n");
                    printf("\nSelecione uma opção: ");

                    scanf("%d", &opcaoPesq);
                    getchar();
                    if (opcaoPesq != 0) {
                        // Cria uma árvore binária de busca temporária para ordenar os pacientes
                        ABB *arvoreTemp = cria_abb();
                        switch (opcaoPesq) {
                            case 1:
                                limpar_console();
                                reconstruir_abb(listaPacientes, arvoreTemp, comparar_por_ano);
                                printf("\nPacientes ordenados por ano de entrada:\n\n");
                                imprimir_in_ordem(arvoreTemp->raiz);
                                limpar_console_dinamico();
                                break;
                            case 2:
                                limpar_console();
                                reconstruir_abb(listaPacientes, arvoreTemp, comparar_por_mes);
                                printf("\nPacientes ordenados por mês de entrada:\n\n");
                                imprimir_in_ordem(arvoreTemp->raiz);
                                limpar_console_dinamico();
                                break;
                            case 3:
                                limpar_console();
                                reconstruir_abb(listaPacientes, arvoreTemp, comparar_por_dia);
                                printf("\nPacientes ordenados por dia de entrada:\n\n");
                                imprimir_in_ordem(arvoreTemp->raiz);
                                limpar_console_dinamico();
                                break;
                            case 4:
                                limpar_console();
                                reconstruir_abb(listaPacientes, arvoreTemp, comparar_por_idade);
                                printf("\nPacientes ordenados por idade:\n\n");
                                imprimir_in_ordem(arvoreTemp->raiz);
                                limpar_console_dinamico();
                                break;
                            default:
                                printf("\nOpção inválida. Tente novamente.\n");
                                limpar_console_dinamico();
                        }
                        // Libera a estrutura da ABB temporária (os nós alocados não são liberados nesta implementação simplificada)
                        free(arvoreTemp);
                    } else {
                        printf("\nVoltando ao menu principal...\n");
                    }
                } while (opcaoPesq != 0);
            } break;
            case 5: {
                // Ação de Desfazer Operação
                imprimir_stack(pilhaOperacoes);
                printf("\nDeseja desfazer a última operação? (s/n): ");
                char resposta;
                scanf(" %c", &resposta);
                getchar();
                if (resposta == 's' || resposta == 'S') {
                    desfazer_ultima_operacao(pilhaOperacoes, filaAtendimento);
                } else {
                    printf("\nNenhuma operação foi desfeita.\n");
                    limpar_console_dinamico();
                }
            } break;
            case 6: {
                // Submenu de operações com arquivos
                int opcaoArq;
                do {
                    limpar_console();
                    printf("\n╔════════════════════════════════════════════╗\n");
                    printf("║               MENU ARQUIVOS                ║\n");
                    printf("╠════════════════════════════════════════════╣\n");
                    printf("║ 1 - Salvar lista de pacientes em arquivo   ║\n");
                    printf("║ 2 - Carregar lista de pacientes do arquivo ║\n");
                    printf("║ 0 - Voltar ao menu principal               ║\n");
                    printf("╚════════════════════════════════════════════╝\n");
                    printf("\nSelecione uma opção: ");
                    scanf("%d", &opcaoArq);
                    getchar();
                    switch (opcaoArq) {
                        case 1:
                            salvar_lista(listaPacientes, "dbPacientes.txt");
                            break;
                        case 2:
                            carregar_lista(listaPacientes, "dbPacientes.txt");
                            break;
                        case 0:
                            printf("\nERRO!\nVoltando ao menu principal...\n");
                            break;
                        default:
                            limpar_console();
                            printf("\nERRO!\nOpção inválida. Tente novamente.\n");
                            limpar_console_dinamico();
                    }
                } while (opcaoArq != 0);
            } break;
            case 7:
                // Exibe informações sobre o projeto
                mostrar_sobre();
                break;
            case 0:
                // Encerra o programa
                limpar_console();
                printf("\nEncerrando o programa...\n");
                limpar_console_dinamico();
                break;
            default:
                printf("\nOpção inválida. Tente novamente.\n");
                limpar_console_dinamico();
        }
    } while (opcaoMenuPrincipal != 0);

    return 0;
}