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
    QuadNode *raiz = newNode(x, y, width, height);

    RGB(*pixels)
    [pic->width] = (RGB(*)[pic->width])pic->img;

    int pixelRMedio = 0;
    int pixelGMedio = 0;
    int pixelBMedio = 0;

    //PERCORRE A A REGIÃO DA IMAGEM E ENCONTRA A VARIAÇÃO MÉDIA DE R,G e B
    for (int linha = y; linha < (y + height); linha++)
    {
        for (int coluna = x; coluna < (x + width); coluna++)
        {
            pixelRMedio += pixels[linha][coluna].r;
            pixelGMedio += pixels[linha][coluna].g;
            pixelBMedio += pixels[linha][coluna].b;
        }
    }

    if ((width * height) != 0)
    {
        pixelRMedio = pixelRMedio / (width * height);
        pixelGMedio = pixelGMedio / (width * height);
        pixelBMedio = pixelBMedio / (width * height);
    }

    int diferencaMedia = 0;

    //PERCORRE A IMAGEM E ENCONTRA A DIFERENÇA TOTAL DE TODOS OS PIXELS
    for (int linha = y; linha < (y + height); linha++)
    {
        for (int coluna = x; coluna < (x + width); coluna++)
        {
            diferencaMedia += sqrt(pow((pixels[linha][coluna].r - pixelRMedio), 2) + pow((pixels[linha][coluna].g - pixelGMedio), 2) + pow((pixels[linha][coluna].b - pixelBMedio), 2));
        }
    }

    if ((width * height) != 0)
        diferencaMedia = diferencaMedia / (width * height);

    raiz->color[0] = pixelRMedio;
    raiz->color[1] = pixelGMedio;
    raiz->color[2] = pixelBMedio;

    if (diferencaMedia > minDetail)
    {
        raiz->status = PARCIAL;

        raiz->NE = geraNodo(pic, x, y, width / 2, height / 2, minDetail);
        raiz->NW = geraNodo(pic, x + (width / 2), y, width / 2, height / 2, minDetail);
        raiz->SE = geraNodo(pic, x, y + (height / 2), width / 2, height / 2, minDetail);
        raiz->SW = geraNodo(pic, x + (width / 2), y + (height / 2), width / 2, height / 2, minDetail);
    }
    else
        raiz->status = CHEIO;

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
