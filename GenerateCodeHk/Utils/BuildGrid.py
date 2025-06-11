import networkx as nx


def BuildGridHk(G: nx.Graph,
                qtde_linhas: int,
                qtde_colunas: int) -> nx.Graph:
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
