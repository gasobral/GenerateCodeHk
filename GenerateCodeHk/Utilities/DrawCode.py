import networkx as nx
import matplotlib.pyplot as plt


def draw_code(G: nx.Graph, C: list = []) -> None:
    """
Faz o desenho de um grafo.

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
