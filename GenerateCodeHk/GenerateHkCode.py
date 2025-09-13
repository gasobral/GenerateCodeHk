#! /usr/bin/env python3 #
# -*- coding: utf-8 -*- #


## Required Modules - - - - - - - - - - - - - - - - - - - - - - - - - -#
## python built-in modules
import sys
import argparse
from datetime import datetime
from os       import EX_OK
from os       import EX_USAGE

## third party modules
import networkx          as nx
import matplotlib.pyplot as plt
import gurobipy          as gp
from gurobipy            import GRB

## user defined modules
from RegularGrids.HexagonalGrid import build_grid_hk
from RegularGrids.HexagonalGrid import create_extra_columns_hk
from Utilities.DrawCode         import draw_code


## Implementações de Funções - - - - - - - - - - - - - - - - - - - - - -#
def GeraCodigo(G, C):
    """
Utiliza um programa linear para gerar o padrão de um código de
identificação periódico C de menor densidade de um grafo G.

Parametros
----------
G: networkx.Graph()
   Um grafo.

C: list
   Uma lista que irá armazenar um código de identificação de G.

Retorno
-------
list
Uma lista com os vértices do padrão de um código de identificação
periódico de menor densidade do grafo G.
    """

    ## criação do programa linear usando gurobi
    try:
        modelo = gp.Model("MinCodID")

        # associa cada vértice do grafo a uma variável de decisão
        vertices = G.nodes()
        x = modelo.addVars(vertices, lb = 0, vtype = GRB.BINARY,\
                           name = "x")

        # criação da função objetivo
        expressao = 0

        for v in G.nodes():
            if v[0] >= 0 and v[0] < qtde_colunas:
                expressao += x[v]

        modelo.setObjective(expressao, GRB.MINIMIZE)

        # adiciona as restrições de dominiação e separação
        # condição de dominação
        for v in G.nodes():
            # if para descartar os vértices das colunas -2 e
            # qtde_colunas +1 da verificação da condição de dominação, pois
            # são cópias da parte interna do padrão
            if v[0] >= 0 and v[0] < qtde_colunas:
                expressao = x[v]

                for u in G.adj[v]:
                    expressao = expressao + x[u]

                modelo.addConstr(expressao >= 1)

        # condição de separação
        for v in G.nodes():
            if v[0] >= 0 and v[0] < qtde_colunas:
                for u in G.nodes():
            # if para descartar os vértices das colunas -2 e
            # qtde_colunas +1 da verificação da condição de dominação, pois
            # são cópias da parte interna do padrão
                    if u[0] >= 0 and u[0] < qtde_colunas \
                       and u is not v:
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

        ## condições para que o padrão encontrado resulte num código
        ## de identificação periódico
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

        modelo.update()
        modelo.optimize()

        ## construção da lista com os vértices do padrão do
        ## código de identificação periódico de menor densidade
        for v in G.nodes():
            if x[v].x == 1 \
               and v[0] not in [-2,-1,qtde_colunas, qtde_colunas +1]:
                C.append(v)

    except gp.GurobiError as e:
        print('Error code ' + str(e.erno) + ': ' + str(e))

    except AttributeError:
        print(f'Não existe cód. id com densidade {Cmin/QtdeVerticesMin}')

    return C


## Script - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -#
## configuração dos argumentos do script
arg_parser = argparse.ArgumentParser(
    prog="GenerateHkCode",
    description="Generates an identification code for hexgonal grid"
)

arg_parser.add_argument(
    "nrows",
    type=int,
    help="number of rows of the hexagonal grid"
)

arg_parser.add_argument(
    "ncols",
    type=int,
    help="maximum number of cols of the hexagonal grid"
)

script_args = arg_parser.parse_args()
qtde_linhas = script_args.nrows
max_colunas = script_args.ncols

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
    G = build_grid_hk(G, qtde_linhas, qtde_colunas)

    ## adiciona colunas extras
    W, F = create_extra_columns_hk(G, qtde_linhas, qtde_colunas)
    G.add_nodes_from(W)
    G.add_edges_from(F)

    ## obtém um código de identificação da grade hexagonal
    C.clear()
    GeraCodigo(G, C)

    ## remover colunas extras
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

## exibe o código de identificação de menor densidade
print(f"\nDados do melhor código!\n"
      f"Quantidade de colunas: {int(QtdeVerticesMin/qtde_linhas)}")
densidade = len(Cmin)/QtdeVerticesMin
print(f'Densidade: {len(Cmin)}/{QtdeVerticesMin} = '
      f'{densidade}')
print(Cmin)

del G
G = nx.Graph()
G = build_grid_hk(G, qtde_linhas, QtdeVerticesMin//qtde_linhas)
draw_code(G, Cmin)

sys.exit(EX_OK)
