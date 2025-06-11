#! /usr/bin/env python3 #
# -*- coding: utf-8 -*- #


## Módulos - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
import sys
import networkx as nx
import matplotlib.pyplot as plt
from os import EX_OK
from os import EX_USAGE
from Utils.BuildGrid import BuildGridHk


## Implementações de Funções - - - - - - - - - - - - - - - - - - - - - - #
def GeraGradeHk(G, qtde_linhas, qtde_colunas):
    """
Gera uma grade hexgonal com qtde_linhas e qtde_colunas.

Parâmetros
---------
qtde_linhas: int
   quantidade de linhas da grade hexagonal

qtde_colunas: int
   quantidade de colunas da grade hexagonal

Retorno
------
nx.Graph()
A grade hexagonal com k linhas e c colunas.
    """

    ## cria as listas que serão usadas para criar a lista
    ## com os vértices da grade
    K = [k for k in range(1, qtde_linhas +1)]
    Z = [z for z in range(0, qtde_colunas)]

    ## cria a lista com os vértices de grade
    V = [(z, k) for z in Z for k in K]

    ## cria lista com as arestas da grade
    E = []

    ## cria as arestas entre dois vértices que estão na mesma linha
    for j in range(1, qtde_linhas +1):
        for i in range(0, qtde_colunas -1):
            v = (i, j)
            u = (i +1, j)
            E.append((v, u))

    ## cria as arestas entre vértices que estão em linhas distintas
    i = 1
    while i < qtde_linhas:
        if i % 2 != 0:
            j = 0
        else:
            j = 1

        while j < qtde_colunas:
            v = (j, i)
            u = (j, i +1)
            E.append((v, u))
            j = j +2

        i = i +1

    ## cria o grafo que irá armazenar a grade
    G = nx.Graph()
    G.add_nodes_from(V)
    G.add_edges_from(E)
    return G


def VerificaCodigo(G, C):
    """
Verifica se um conjunto de vértices C é um código de identificação, ou
não

Parâmetros
----------
G: nx.Graph
    Um grafo que representa a grade hexagonal

C: list
    Uma lista de vértices

Retorno
-------
Bool
    Retorn True se C é um código de identificação, caso contrário,
    retorna False
    """

    ## cria os identificadores dos vértice do grafo
    identificadores = dict()

    for v in G.nodes():
        id_v = set()

        if v in C:
            id_v.add(v)

        for u in G.adj[v]:
            if u in C:
                id_v.add(u)

        identificadores.update({v : id_v})

    resposta = True

    ## verifica se existe um identificador vazio
    for v in G.nodes():
        if len(identificadores[v]) == 0:
            print(f"O identificador do vértices {v} é vazio!\n")
            resposta = False

    ## verifica se existe dois vértices distintos com o mesmo
    ## identificador
    for v in G.nodes():
        for u in G.nodes():
            if v != u and identificadores[v] == identificadores[u]:
                print(f"O vértices {v} e {u} têm o mesmo identificador!"
                      f"\nC[{v}] == C[{u}] == {identificadores[v]}\n")
                resposta = False

    return resposta


def DesenhaGrafo(G, C):
    """
Faz um desenho de um grafo.

Parâmetros
----------
G: networkx.Graph
   Uma grafo

C: list
   Uma lista com os vértices que pertencem a um código de identificação
    """

    ## obtém as posições dos vértices
    posicoes = dict((v, v) for v in G.nodes())

    ## cria uma lista com os vértices que não pertencem ao código
    U = [u for u in list(G) if u not in C]

    ## desenha os vértices que pertencem ao código
    nx.draw_networkx(G, posicoes, nodelist = C, node_color = "r", \
                     with_labels = True)

    ## desenha os vértices que não pertencem ao código
    nx.draw_networkx(G, posicoes, nodelist = U, with_labels = True)

    plt.axis("off")
    plt.show()


## Script - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -#
if len(sys.argv) != 2:
    print("ERRO! Deve ser passado um arquivo!", file = sys.stderr)
    sys.exit(EX_USAGE)

with open(sys.argv[1] , encoding = "utf-8") as arqCodigo:
    linha = arqCodigo.readline()
    qtde_linhas, qtde_colunas, densidade = linha.strip('\n').split(' ')
    qtde_linhas = int(qtde_linhas)
    qtde_colunas = int(qtde_colunas)
    densidade = float(densidade)
    print(f"linhas: {qtde_linhas}\tcolunas: {qtde_colunas}"
          f"\tdensidade: {densidade}")
    linha = arqCodigo.readline()
    linha = linha.strip('\n').rstrip().split(' ')
    G = nx.Graph()
    G = GeraGradeHk(G, qtde_linhas, qtde_colunas)
    C = []

    for l in linha:
        numeros = l.strip("()").split(',')
        C.append( ( int(numeros[0]), int(numeros[1]) ) )

    ## adiciona arestas artificiais para simular o padrão
    F = []
    for i in range(1, qtde_linhas +1):
        v = (0, i)
        u = (qtde_colunas -1, i)
        F.append((v, u))

    G.add_edges_from(F)

    if VerificaCodigo(G, C) == False:
        print("Não é um código de identificação!")

    else:
        print("É um código de identificação!")

    print(f"Densidade: {len(C)/(qtde_linhas * qtde_colunas)}")
    DesenhaGrafo(G, C)
