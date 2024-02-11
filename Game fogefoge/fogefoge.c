// foge foge jogo derivado do PAC-MAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CIMA 'w'
#define BAIXO 's'
#define ESQUERDA 'a'
#define DIREITA 'd'
#define HEROI '@'
#define FANTASMA 'F'
#define VAZIO '.'
#define PAREDE_VERTICAL '|'
#define PAREDE_HORIZONTAL '-'
#define PILULA 'P'
#define BOMBA 'b'

// transformamos os caracteres w,s,a,d em constantes para poder lidar melhor com o codigo
// tambem os demais caracteres de heroi @, parede e vazio como constantes

//  estrutura com a variaveis globais para facilitar a escrita e leitura dos dados

struct mapa
{
    char **matriz; // matriz = ponteiro de ponteiro
    int linhas;
    int colunas;
} m;
typedef struct mapa MAPA;

struct posicao
{
    int x;
    int y;
} heroi;
typedef struct posicao POSICAO;

int tempilula = 0;

// funçoes declaradas
void lemapa();
void alocamapa(MAPA *m);
void liberamapa(MAPA *m);
void copiamapa(MAPA *destino, MAPA *origem);
int ehdirecao(char direcao);
int podeandar(MAPA *m, char personagem, int x, int y);
int ehvalida(MAPA *m, int x, int y);
int ehvazia(MAPA *m, int x, int y);
int ehparede(MAPA *m, int x, int y);
int ehpersonagem(MAPA *m, char personagem, int x, int y);
void move(char direcao);
void andanomapa(MAPA *m, int xorigem, int yorigem, int xdestino, int ydestino);
void imprimeparte(char desenho[4][7], int parte);
void imprimemapa(MAPA *m);
int acabou();
int encontramapa(MAPA *m, POSICAO *p, char c);
int praondefantasmavai(int xatual, int yatual, int *xdestino, int *ydestino);
void fantasmas();
void explodepilula2(int x, int y, int somax, int somay, int qtd);
void explodepilula();

// funçoes inicializadas
void liberamapa(MAPA *m)
{
    /*com essa funcao liberamos a memoria solicitada ao sistema*/
    for (int i = 0; i < m->linhas; i++)
    {
        free(m->matriz[i]);
    }
    free(m->matriz);
}
void alocamapa(MAPA *m)
{
    // alocaçao dinamica de memoria
    m->matriz = malloc(sizeof(char *) * m->linhas);
    for (int i = 0; i < m->linhas; i++)
    {
        m->matriz[i] = malloc(sizeof(char) * (m->colunas + 1));
    }
}
void lemapa()
{
    /*nessa funcao abrimos o arquivo que contem o mapa do jogo, alocamos memoria
    e depois fechamos o arquivo*/
    FILE *f;
    f = fopen("mapa.txt", "r");
    if (f == 0)
    {
        printf("Erro na leitura do mapa\n");
        exit(1);
    }
    fscanf(f, "%d %d", &(m.linhas), &(m.colunas));

    alocamapa(&m);

    for (int i = 0; i < m.linhas; i++)
    {
        fscanf(f, "%s", m.matriz[i]);
    }

    fclose(f);
}

int acabou()
{
    POSICAO pos;
    int encontrou = encontramapa(&m, &pos, HEROI);
    return !encontrou;
}
void copiamapa(MAPA *destino, MAPA *origem)
{
    /*com essa funcao podemos criar uma copia do mapa e poder mudar as posicoes dos fantasmas em outra funcao
    aqui tambem alocamos memoria para o novo mapa*/
    destino->linhas = origem->linhas;
    destino->colunas = origem->colunas;
    alocamapa(destino);
    for (int i = 0; i < origem->linhas; i++)
    {
        strcpy(destino->matriz[i], origem->matriz[i]);
    }
}
int encontramapa(MAPA *m, POSICAO *p, char c)
{
    // essa funçao vai buscar onde o caracter do heroi se encontra dentro do mapa
    // ou seja, se ha algo ou nao no mapa
    for (int i = 0; i < m->linhas; i++)
    {
        for (int j = 0; j < m->colunas; j++)
        {
            if (m->matriz[i][j] == c)
            {
                p->x = i;
                p->y = j;
                return 1;
            }
        }
    }
    return 0;
}

int ehdirecao(char direcao)
{
    // essa funcao devolve a direcao se essa corresponde ao caracter correto
    return direcao == ESQUERDA || direcao == CIMA || direcao == BAIXO || direcao == DIREITA;
}

int podeandar(MAPA *m, char personagem, int x, int y)
{
    return ehvalida(m, x, y) &&
           !ehparede(m, x, y) &&
           !ehpersonagem(m, personagem, x, y);
}

int ehparede(MAPA *m, int x, int y)
{
    return m->matriz[x][y] == PAREDE_VERTICAL ||
           m->matriz[x][y] == PAREDE_HORIZONTAL;
}

int ehpersonagem(MAPA *m, char personagem, int x, int y)
{
    return m->matriz[x][y] == personagem;
}

int ehvalida(MAPA *m, int x, int y)
{
    // bloqueia o heroi de seguir se nao ha espaço para mover-se (paredes)
    if (x >= m->linhas)
        return 0;
    if (y >= m->colunas)
        return 0;
    return 1;
}

int ehvazia(MAPA *m, int x, int y)
{
    // devolve a posicao vazia depois de introduzida a direcao
    return m->matriz[x][y] == VAZIO;
}

void andanomapa(MAPA *m, int xorigem, int yorigem, int xdestino, int ydestino)
{
    // muda o personagem de posicao de um ponto ao otro no mapa e o substitui na posicao anterior pelo vazio '.'
    char personagem = m->matriz[xorigem][yorigem];
    m->matriz[xdestino][ydestino] = personagem;
    m->matriz[xorigem][yorigem] = VAZIO;
}
void imprimeparte(char desenho[4][7], int parte)
{
    printf("%s", desenho[parte]);
}
void imprimemapa(MAPA *m)
{
    /*aqui definimos as partes da interface do jogo*/
    char desenhofantasma[4][7] = {
        {" .-.  "},
        {"| OO| "},
        {"|   | "},
        {"'^^^' "}};

    char desenhoparede[4][7] = {
        {"......"},
        {"......"},
        {"......"},
        {"......"}};

    char desenhoheroi[4][7] = {
        {" .--. "},
        {"/ _.-'"},
        {"\\  '-."},
        {" '--' "}};

    char desenhopilula[4][7] = {
        {"      "},
        {" .-.  "},
        {" '-'  "},
        {"      "}};

    char desenhovazio[4][7] = {
        {"      "},
        {"      "},
        {"      "},
        {"      "}};

    for (int i = 0; i < m->linhas; i++)
    {
        /*aqui desenhamos todas as partes da interface do jogo*/
        for (int parte = 0; parte < 4; parte++)
        {
            for (int j = 0; j < m->colunas; j++)
            {

                switch (m->matriz[i][j])
                {
                case FANTASMA:
                    imprimeparte(desenhofantasma, parte);
                    break;
                case HEROI:
                    imprimeparte(desenhoheroi, parte);
                    break;
                case PILULA:
                    imprimeparte(desenhopilula, parte);
                    break;
                case PAREDE_VERTICAL:
                case PAREDE_HORIZONTAL:
                    imprimeparte(desenhoparede, parte);
                    break;
                case VAZIO:
                    imprimeparte(desenhovazio, parte);
                    break;
                }
            }
            printf("\n");
        }
    }
}
void move(char direcao)
{
    // se a direcao introduzida na funcao ehdirecao devolve um caracter nao armazenado, a funcao para e devolve nada.
    if (!ehdirecao(direcao))
        return;

    // essa matriz dira onde o caracter do heroi se encontra dentro do mapa e mudamos na matriz a posiçao do heroi
    int proximox = heroi.x;
    int proximoy = heroi.y;

    switch (direcao)
    {
        // em caso a andamos para a esquerda, ou seja, uma coluna a menos e mudamos na matriz a posiçao do heroi
    case ESQUERDA:
        proximoy--;
        break;
        // em caso w andamos para cima, ou seja, uma linha a menos e mudamos na matriz a posiçao do heroi
    case CIMA:
        proximox--;
        break;
        // em caso s andamos para baixo, ou seja, uma linha a mais e mudamos na matriz a posiçao do heroi
    case BAIXO:
        proximox++;
        break;
        // em caso d andamos para a direita, ou seja, uma coluna a mais e mudamos na matriz a posiçao do heroi
    case DIREITA:
        proximoy++;
        break;
    }
    if (!podeandar(&m, HEROI, proximox, proximoy))
        return;

    if (ehpersonagem(&m, PILULA, proximox, proximoy))
    {
        tempilula = 1;
    }

    // aqui chamamos a funcao anda para alterar a posicao do heroi no jogo
    andanomapa(&m, heroi.x, heroi.y, proximox, proximoy);

    // aqui passamos a posicao nova para a anterior, podendo assim seguir movendo o heroi no mapa
    heroi.x = proximox;
    heroi.y = proximoy;
}
int praondefantasmavai(int xatual, int yatual, int *xdestino, int *ydestino)
{
    /* com essa funcao definimos as posicoes que os fantasmas podem ocupar no mapa
     de forma aleatoria */
    int opcoes[4][2] = {
        {xatual, yatual + 1},
        {xatual + 1, yatual},
        {xatual, yatual - 1},
        {xatual - 1, yatual}};

    srand(time(0));
    for (int i = 0; i < 10; i++)
    {
        int posicao = rand() % 4;

        if (podeandar(&m, FANTASMA, opcoes[posicao][0], opcoes[posicao][1]))
        {
            *xdestino = opcoes[posicao][0];
            *ydestino = opcoes[posicao][1];
            return 1;
        }
    }
    return 0;
}
void fantasmas()
{
    /*fazendo uso de uma estrutura auxiliar copiamos o mapa para que os fantasmas possam
    se mover apenas uma vez para a direita, cada vez que se move o heroi
    e ao final liberamos a memoria*/
    MAPA copia;

    copiamapa(&copia, &m);

    for (int i = 0; i < copia.linhas; i++)
    {
        for (int j = 0; j < copia.colunas; j++)
        {
            if (copia.matriz[i][j] == FANTASMA)
            {
                int xdestino;
                int ydestino;
                int encontrou = praondefantasmavai(i, j, &xdestino, &ydestino);
                if (encontrou)
                {
                    andanomapa(&m, i, j, xdestino, ydestino);
                }
            }
        }
    }

    liberamapa(&copia);
}

void explodepilula()
{
    if (!tempilula)
        return;
    explodepilula2(heroi.x, heroi.y, 0, 1, 3);
    explodepilula2(heroi.x, heroi.y, 0, -1, 3);
    explodepilula2(heroi.x, heroi.y, 1, 0, 3);
    explodepilula2(heroi.x, heroi.y, -1, 0, 3);
    tempilula = 0;
}

void explodepilula2(int x, int y, int somax, int somay, int qtd)
{
    // se acabou o numero de vezes,
    // entao acaba a função
    if (qtd == 0)
        return;
    int novox = x + somax;
    int novoy = y + somay;

    if (!ehvalida(&m, novox, novoy + 1))
        return;
    if (!ehparede(&m, novox, novoy + 1))
        return;

    m.matriz[x][y + 1] = VAZIO;

    // dessa vez, passamos qtd-1, pois
    // já rodamos uma vez a função
    explodepilula2(novox, novoy, somax, somay, qtd - 1);
}
int main()
{

    lemapa();
    encontramapa(&m, &heroi, HEROI);

    do
    {
        printf("Tem pilula: %s\n", (tempilula ? "SIM" : "NAO"));
        imprimemapa(&m);
        char comando;
        scanf(" %c", &comando);
        move(comando);
        if (comando == BOMBA)
            explodepilula();
        fantasmas();

    } while (!acabou());

    printf("FIM DE JOGO! UM FANTASMA TE MATOU\n");

    liberamapa(&m);
}