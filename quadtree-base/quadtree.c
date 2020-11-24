#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

unsigned int first = 1;
char desenhaBorda = 1;

QuadNode *geraNodo(Img *pic, int x, int y, int width, int height, float minDetail)
{
    QuadNode *raiz = newNode(0, 0, width, height);

    RGB(*pixels)
    [pic->width] = (RGB(*)[pic->width])pic->img;

    int pixelRMedio = 0;
    int pixelGMedio = 0;
    int pixelBMedio = 0;

    //PERCORRE A A REGIÃO IMAGEM E ENCONTRA A VARIAÇÃO MÉDIA DE R,G e B
    for (int linha = y; linha < height; linha++)
    {
        for (int coluna = x; coluna < width; coluna++)
        {
            pixelRMedio += pixels[linha][coluna].r;
            pixelGMedio += pixels[linha][coluna].g;
            pixelBMedio += pixels[linha][coluna].b;
        }
    }

    pixelRMedio = pixelRMedio / (width * height);
    pixelGMedio = pixelGMedio / (width * height);
    pixelBMedio = pixelBMedio / (width * height);

    int diferencaMedia = 0;

    //PERCORRE A IMAGEM E ENCONTRA A DIFERENÇA TOTAL DE TODOS OS PIXELS
    for (int linha = y; linha < height; linha++)
    {
        for (int coluna = x; coluna < width; coluna++)
        {
            diferencaMedia += sqrt(pow((pixels[linha][coluna].r - pixelRMedio), 2) + pow((pixels[linha][coluna].g - pixelGMedio), 2) + pow((pixels[linha][coluna].b - pixelBMedio), 2));
        }
    }

    diferencaMedia = diferencaMedia / (width * height);

    //SE A DIFERENCA MEDIA FOR MENOR QUE O NIVEL DE DETALHE, O STATUS DO NODO É PARCIAL
    if (diferencaMedia <= minDetail)
    {
        raiz->color[0] = 255;
        raiz->color[1] = 0;
        raiz->color[2] = 0;
        raiz->status = CHEIO;
    }
    else
    {
        raiz->color[0] = 0;
        raiz->color[1] = 0;
        raiz->color[2] = 255;
        raiz->status = PARCIAL;
    }

    if (raiz->status == PARCIAL)
    {
        QuadNode *ne = geraNodo(pic, 0, 0, width / 2, height / 2, minDetail);
        QuadNode *nw = geraNodo(pic, (width / 2), 0, (width / 2), height / 2, minDetail);
        QuadNode *se = geraNodo(pic, 0, height / 2, width / 2, height / 2, minDetail);
        QuadNode *sw = geraNodo(pic, width / 2, height / 2, width / 2, height / 2, minDetail);

        raiz->NE = ne;
        raiz->NW = nw;
        raiz->SE = se;
        raiz->SW = sw;
    }

    return raiz;
}

QuadNode *newNode(int x, int y, int width, int height)
{
    QuadNode *n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

QuadNode *geraQuadtree(Img *pic, float minDetail)
{
    printf("%d", pic->width);

    int width = pic->width;
    int height = pic->height;

    QuadNode *raiz = geraNodo(pic, 0, 0, width, height, minDetail);

    return raiz;

    // COMENTE a linha abaixo quando seu algoritmo ja estiver funcionando
    // Caso contrario, ele ira gerar uma arvore de teste com 3 nodos

    // #define DEMO
    // #ifdef DEMO

    //     /************************************************************/
    //     /* Teste: criando uma raiz e dois nodos a mais              */
    //     /************************************************************/

    //     QuadNode *raiz = newNode(0, 0, width, height);
    //     raiz->status = PARCIAL;
    //     raiz->color[0] = 0;
    //     raiz->color[1] = 0;
    //     raiz->color[2] = 255;

    //     QuadNode *nw = newNode(width / 2, 0, width / 2, height / 2);
    //     nw->status = PARCIAL;
    //     nw->color[0] = 0;
    //     nw->color[1] = 0;
    //     nw->color[2] = 255;

    //     // Aponta da raiz para o nodo nw
    //     raiz->NW = nw;

    //     QuadNode *nw2 = newNode(width / 2 + width / 4, 0, width / 4, height / 4);
    //     nw2->status = CHEIO;
    //     nw2->color[0] = 255;
    //     nw2->color[1] = 0;
    //     nw2->color[2] = 0;

    //     // Aponta do nodo nw para o nodo nw2
    //     nw->NW = nw2;

    // #endif
    //     // Finalmente, retorna a raiz da árvore
    //     return raiz;
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode *n)
{
    if (n == NULL)
        return;
    if (n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    //printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder()
{
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode *raiz)
{
    if (raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode *raiz)
{
    FILE *fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE *fp, QuadNode *n)
{
    if (n == NULL)
        return;

    if (n->NE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if (n->NW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if (n->SE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if (n->SW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode *n)
{
    if (n == NULL)
        return;

    glLineWidth(0.1);

    if (n->status == CHEIO)
    {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width - 1, n->y);
        glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
        glVertex2f(n->x, n->y + n->height - 1);
        glEnd();
    }

    else if (n->status == PARCIAL)
    {
        if (desenhaBorda)
        {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x + n->width - 1, n->y);
            glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
            glVertex2f(n->x, n->y + n->height - 1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}
