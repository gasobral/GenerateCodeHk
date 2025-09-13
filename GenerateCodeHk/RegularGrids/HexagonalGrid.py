import networkx as nx


## User defined functions - - - - - - - - - - - - - - - - - - - - - - -#
## Here is defined the functions that manipulate the graph which
## represents a hexagonal grid with finite number of rows
def build_grid_hk(G: nx.Graph,
                  qtde_linhas: int,
                  qtde_colunas: int) -> nx.Graph:
    """
Gera uma grade hexgonal com qtde_linhas linhas e qtde_colunas
colunas.
Generates a hexgonal grid with qtde_linhas rows and qtde_colunas
columns.
Создаёт шестиугольную сетку со строками qtde_linhas и столбцами
qtde_colunas.


Parâmetros/Arguments/Аргументы
------------------------------
qtde_linhas: quantidade de linhas da grade hexagonal
             amount of rows of the hexognal grid
             количество строк шестиугольной сетки

qtde_colunas: quantidade de colunas da grade hexagonal
              amount of rows of the hexognal grid
              количество столбцов шестиугольной сетки

Retorno/Return/Возврат
----------------------
A grade hexagonal com qtde_linhas linhas e qtde_colunas colunas.
A hexagonal grid with qtde_linhas rows and qtde_colunas columns.
Шестиугольная сетка со строками qtde_linhas и столбцами qtde_colunas.
    """

    ## cria as listas que serão usadas na criação dos vértices da
    ## grade, cada vértice corresponde a um par ordenado (z, k)
    ## creates the lists to be used in the creation of vertex set,
    ## each vertex corresponds to an order pair (z, k)
    ## создаёт списоки, которые будет использоваться при создании
    ## набора вершин, каждая вершина соответствует
    ## упорядоченной паре (z, k)
    K = [k for k in range(1, qtde_linhas +1)]
    Z = [z for z in range(0, qtde_colunas)]

    ## cria a lista com os vértices de grade
    ## creates the list with the vertices of the hexagonal grid
    ## создаёт список с вершинами шестиугольной сетки
    V = [(z, k) for z in Z for k in K]

    ## cria as arestas dos vértices que estão na mesma linha
    ## creates the edges bewteen veritces which are in the same row
    ## создаёт рёбра между вершинами, которые находятся в одном ряду
    E = [((i,j), (i+1,j)) for j in range(1, qtde_linhas +1)
                          for i in range(0, qtde_colunas -1)]

    ## adiciona as arestas entre os vértices que estão em linhas
    ## consecutivas e numa coluna par
    F = [((j,i), (j,i+1)) for i in range(2, qtde_linhas, 2)
                          for j in range(0, qtde_colunas, 2)]
    E.extend(F)

    ## adiciona as arestas entre os vértices que estão em linhas
    ## consecutivas e numa coluna ímpar
    F = [((j,i), (j,i+1)) for i in range(1, qtde_linhas, 2)
                          for j in range(1, qtde_colunas, 2)]
    E.extend(F)

    ## cria o grafo que armazena a grade
    ## creates the graph which stores the grid
    ## создаёт граф, который хранит шестиугольную сетку
    G = nx.Graph()
    G.add_nodes_from(V)
    G.add_edges_from(E)
    return G


def create_extra_columns_hk(G: nx.Graph,
                            qtde_linhas: int,
                            qtde_colunas: int) -> tuple:
    """
Cria o conjunto de vértices e o conjunto de arestas das colunas
colocadas antes de depois do padrão do código


Parâmetros/Arguments/Аргументы
------------------------------
G: um objeto da classe nx.Graph que representa a grade hexgonal com
   qtde_linhas linhas e qtde_colunas colunas.

qtde_linhas: a quantidade de linhas da grade hexagonal.

qtde_colunas: a quantidade de colunas da grade hexagonal.


Retorno/Return/Возврат
----------------------
As listas que contém os vértices e arestas das colunas -2, -1, qtde_colunas
e qtde_colunas+1, estrutura adicional do grafo usada apenas para garantir a
repetição do padrão de um código de identificação.
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
