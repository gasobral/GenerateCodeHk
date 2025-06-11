#! /usr/bin/env python3 #
# -*- coding: utf-8 -*- #


## Bibliotecas - - - - - - - - - - - - - - - - - - - - - - - - - - - - -#
import sys
import networkx as nx
import matplotlib.pyplot as plt
import gurobipy as gp
from gurobipy import GRB
from os import EX_OK
from os import EX_USAGE
from datetime import datetime


## Implementações de Funções - - - - - - - - - - - - - - - - - - - - - -#
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
            j = 0 #            j = 1
        else:
            j = 1 #            j = 0
        ## comentei o código acima para que, nas linhas pares, as
        ## colunas comecem na coluna 0

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


def GeraCodigo(G, C, Cmin, QtdeVerticesMin):
    """
Gera um código de identificação de um grafo G.

Parametros
---------
G: networkx.Graph()
   Um grafo.

C: list
   Uma lista que irá armazenar um código de identificação de G.

densidade: float
    O valor da densidade que será usado como limite superior

Retorno
-------
list
Uma lista com um código de identificação de G.
    """

    try:
        modelo = gp.Model("MinCodID")

        # atrela cada vértice do grafo a uma variável de decisão
        vertices = G.nodes()
        x = modelo.addVars(vertices, lb = 0, vtype = GRB.BINARY,\
                           name = "x")

        # associa o número de vértices do grafo e a densidade a duas
        # variáveis do modelo
        N = modelo.addVar(vtype = GRB.INTEGER, name = "N")
        varCmin = modelo.addVar(vtype = GRB.INTEGER, name = "Cmin")
        varQtdeVerticesMin = modelo.addVar(vtype = GRB.INTEGER,\
                                        name = "QtdeVerticesMin")

        # cria a função de objetivo
#        expressao = x.sum()
        expressao = 0

        for v in G.nodes():
            if v[0] >= 0 and v[0] < qtde_colunas:
                expressao += x[v]

        modelo.setObjective(expressao, GRB.MINIMIZE)

        # adiciona as restrições de dominiação e separação
        # condição de dominação
        for v in G.nodes():
#             expressao = x[v]
# 
#             for u in G.adj[v]:
#                 expressao = expressao + x[u]
            # if para descartar os vértices das colunas -2 e
            # qtde_colunas +1 da verificação da condição de dominação, pois
            # são cópia da parte interna do padrão
            if v[0] >= 0 and v[0] < qtde_colunas:
                expressao = x[v]

                for u in G.adj[v]:
                    expressao = expressao + x[u]

            modelo.addConstr(expressao >= 1)

        # condição de separação
        for v in G.nodes():
            for u in G.nodes():
#                 if u != v:
#                     vizinhanca_fechada_u = [u]
#                     vizinhanca_fechada_u.extend(G.adj[u])
#                     N_u = set(vizinhanca_fechada_u)
#                     vizinhanca_fechada_v = [v]
#                     vizinhanca_fechada_v.extend(G.adj[v])
#                     N_v = set(vizinhanca_fechada_v)
#                     N_vu = N_v.symmetric_difference(N_u)
#                     w = N_vu.pop()
#                     expressao = x[w]
# 
#                     while len(N_vu) > 0:
#                         w = N_vu.pop()
#                         expressao = expressao + x[w]
# 
#                     modelo.addConstr(expressao >= 1)
            # if para descartar os vértices das colunas -2 e
            # qtde_colunas +1 da verificação da condição de dominação, pois
            # são cópia da parte interna do padrão
                    if u[0] >= 0 and u[0] < qtde_colunas \
                       and u is not v:
#                    if u != v:
                        vizinhanca_fechada_u = [u]
                        vizinhanca_fechada_u.extend(G.adj[u])
                        N_u = set(vizinhanca_fechada_u)
                        vizinhanca_fechada_v = [v]
                        vizinhanca_fechada_v.extend(G.adj[v])
                        N_v = set(vizinhanca_fechada_v)
                        N_vu = N_v.symmetric_difference(N_u)
                        w = N_vu.pop()
                        expressao = x[w]
    
                        while len(N_vu) > 0:
                            w = N_vu.pop()
                            expressao = expressao + x[w]
    
                        modelo.addConstr(expressao >= 1)

        ## condições para repetir o códgio
        for i in range(1, qtde_linhas +1):
            v = (-1,i)
            u = (qtde_colunas-1,i)
            expressao = x[v] - x[u]
            modelo.addConstr(expressao == 0)

            v = (-2,i)
            u = (qtde_colunas-2,i)
            expressao = x[v] - x[u]
            modelo.addConstr(expressao == 0)

            v = (qtde_colunas,i)
            u = (0,i)
            expressao = x[v] - x[u]
            modelo.addConstr(expressao == 0)

            v = (qtde_colunas +1,i)
            u = (1,i)
            expressao = x[v] - x[u]
            modelo.addConstr(expressao == 0)

        ## junção das colunas -1 e 0 (antes do padrão)
        for i in range(1, qtde_linhas +1):
            W = [(coluna, i) for coluna in (-1,0,1)
                 for i in range(1, qtde_linhas +1)]

            for w1 in W:
                vizinhanca_fechada_w1 = [w1]
                vizinhanca_fechada_w1.extend(G.adj[w1])
                N_w1 = set(vizinhanca_fechada_w1)

                for w2 in W:
                    if w2 != w1:
                        vizinhanca_fechada_w2 = [w2]
                        vizinhanca_fechada_w2.extend(G.adj[w2])
                        N_w2 = set(vizinhanca_fechada_w2)
                        N_w1_w2 = N_w1.symmetric_difference(N_w2)
                        w = N_w1_w2.pop()
                        expressao = x[w]

                        while len(N_w1_w2) > 0:
                            w = N_w1_w2.pop()
                            expressao = expressao + x[w]

                        modelo.addConstr(expressao >= 1)

        ## junção das colunas qtde_colunas-1 e qtde_colunas (depois do padrão)
        for i in range(1, qtde_linhas +1):
            W = [(coluna, i) for coluna in (qtde_colunas -2, qtde_colunas -1, qtde_colunas)
                 for i in range(1, qtde_linhas +1)]

            for w1 in W:
                vizinhanca_fechada_w1 = [w1]
                vizinhanca_fechada_w1.extend(G.adj[w1])
                N_w1 = set(vizinhanca_fechada_w1)

                for w2 in W:
                    if w2 != w1:
                        vizinhanca_fechada_w2 = [w2]
                        vizinhanca_fechada_w2.extend(G.adj[w2])
                        N_w2 = set(vizinhanca_fechada_w2)
                        N_w1_w2 = N_w1.symmetric_difference(N_w2)
                        w = N_w1_w2.pop()
                        expressao = x[w]

                        while len(N_w1_w2) > 0:
                            w = N_w1_w2.pop()
                            expressao = expressao + x[w]

                        modelo.addConstr(expressao >= 1)

        # utiliza um valor de densidade como limite supeior de uma
        # condição do PL
        N = G.number_of_nodes()
        varCmin = Cmin
        varQtdeVerticesMin = QtdeVerticesMin
        modelo.addConstr((x.sum()) * varQtdeVerticesMin <= varCmin * N)

        modelo.update()
#        print(modelo.display())

        # chamando o solver
        modelo.optimize()

        # lendo a resposta
        for v in G.nodes():
#             if x[v].x == 1:
#                 C.append(v)
            if x[v].x == 1 \
               and v[0] not in [-2,-1,qtde_colunas, qtde_colunas +1]:
                C.append(v)

    except gp.GurobiError as e:
        print('Error code ' + str(e.erno) + ': ' + str(e))

    except AttributeError:
        print(f'Não existe cód. id com densidade {Cmin/QtdeVerticesMin}')

    return C


def ImprimeDensidade(G, C):
    """
Imprime na saída padrão a densidade de um código de identificação C.

Parâmetros
----------
C: list
   Uma lista com um código de identificação

G: networkx.Graph()
   Um grafo

Retorno
-------
float
O valor da densidade de C em G
    """

    return float(len(C) / G.__len__())


def CriaColunasExtras(G, qtde_linhas, qtde_colunas):
    """
    Cria o conjunto de vértices e o conjunto de arestas das colunas
    colocadas antes de depois do padrão do código
    """

    ## cria os vértices das colunas -2, -1, qtde_colunas e qtde_colunas+1
    # (colunas antes do início do padrão)
    W = [(i,j) for i in [-2,-1, qtde_colunas, qtde_colunas +1]
         for j in range(1, qtde_linhas +1) ]

    ## cria das arestas da coluna -2, -1, qtde_colunas, qtde_colunas +1
    F = []

    ## adiciona as arestas das linhas
    for j in range(1, qtde_linhas +1):
        F.extend([
            ((-2,j), (-1,j)),
            ((-1,j), (0,j)),
            ((qtde_colunas-1,j), (qtde_colunas,j)),
            ((qtde_colunas,j), (qtde_colunas+1,j)) ])

    ## adiciona as arestas das colunas
    for i in range(1, qtde_linhas):
        if i % 2 == 1:
            F.append(((-2,i), (-2, i+1)))
            F.append(((qtde_colunas,i), (qtde_colunas, i+1)))

        else:
            F.append(((-1,i), (-1, i+1)))
            F.append(((qtde_colunas+1,i), (qtde_colunas+1, i+1)))

    return (W, F)


## Script - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -#
if len(sys.argv) != 3:
    print("ERRO: Deve ser informado o número de linhas e o número"
          "máximo de colunas!", file = sys.stderr)
    print(f"Modo de uso: {sys.argv[0]} <número de linhas>"
          f" <número máximo de colunas>")
    sys.exit(EX_USAGE)

## obtém a quantidade de linhas e colunas da grade hexagonal
qtde_linhas = int(sys.argv[1])

## obtém a quantidade máxima de colunas da grade hexagonal
max_colunas = int(sys.argv[2])

## instancia o grafo usado para representar a grade hexagonal com
## qtde_linhas e qtde_colunas
G = nx.Graph()

## inicialização das listas usadas (respectivamente) para armazenar:
## F: lista de arestas artificiais
## C: código de identificação corrente
## Cmin: código de identificação mínimo
## QtdeVerticesMin: quantidade de vértices do grafo onde foi encontrado
##                  um código de identificação de menor densidade
F = []
C = []
Cmin = [i for i in range(qtde_linhas * 4)]
QtdeVerticesMin = qtde_linhas * 4
densidade = 1

## obtém um código de identificação de menor densidade para a
## grade hexagonal com qtde_linhas e qtde_colunas (de 4 a max_colunas)
for qtde_colunas in range(4, max_colunas +2, 2):
    tempo_inicial = datetime.now()
    print(f"{qtde_colunas} colunas\n   início: "
          f"{tempo_inicial.strftime('%H:%M:%S:%f')}")
    G = GeraGradeHk(G, qtde_linhas, qtde_colunas)

#     ## insere as arestas artificiais entre os vértices das colunas
#     ## 0 e qtde_colunas -1 para simular a repetição de um padrão
#     ## (escolha dos vértices que pertencem ao código)
#     F.clear()
#     for i in range(1, qtde_linhas +1):
#         v = (0, i)
#         u = (qtde_colunas -1, i)
#         F.append((v, u))
# 
#     G.add_edges_from(F)

    ## adiciona colunas extras
    W, F = CriaColunasExtras(G, qtde_linhas, qtde_colunas)
    G.add_nodes_from(W)
    G.add_edges_from(F)

    ## obtém um código de identificação da grade hexagonal
    C.clear()
    GeraCodigo(G, C, len(Cmin), QtdeVerticesMin)

#     ## remove as arestas artificiais
#     G.remove_edges_from(F)

    ## remove colunas extras
    G.remove_nodes_from(W)
    G.remove_edges_from(F)

    ## armazena o código de identificação de menor densidade
    if len (C) != 0 and \
       len(C)/G.number_of_nodes() < len(Cmin)/QtdeVerticesMin:
        Cmin.clear()
        Cmin = C.copy()
        QtdeVerticesMin = G.number_of_nodes()

    print(f"densidade: {len(C)/G.number_of_nodes()}")
    tempo_final = datetime.now()
    print(f"      fim: {tempo_final.strftime('%H:%M:%S:%f')}")
    print(f"    tempo: {tempo_final - tempo_inicial}\n")

## exige o código de identificação de menor densidade
print(f"\nDados do melhor código!\n"
      f"Quantidade de colunas: {int(QtdeVerticesMin/qtde_linhas)}")
densidade = len(Cmin)/QtdeVerticesMin
print(f'Densidade: {len(Cmin)}/{QtdeVerticesMin} = '
      f'{densidade}')
print(Cmin)

del G
G = nx.Graph()
G = GeraGradeHk(G, qtde_linhas, QtdeVerticesMin//qtde_linhas)
DesenhaGrafo(G, Cmin)

sys.exit(EX_OK)
