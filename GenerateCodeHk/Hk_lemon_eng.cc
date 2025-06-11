/* Description - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Authors:
   Gabriel Sobral      - IME USP
   Yoshiko Wakabayashi - IME USP
   Rudini Sampaio      - UFC

   Implementation of a program that builds a configuration graph, find a
   minimum mean cycle, on this graph, and use it as a pattern of code,
   with mininum density, for the hexagonal grid with k rows, denoted by
   H_k
*/


/* Libraries - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <new>
#include <lemon/list_graph.h>
#include <lemon/hartmann_orlin_mmc.h>
#include <lemon/path.h>
#include <lemon/smart_graph.h>
#include <lemon/full_graph.h>
#include <chrono>


/* Namespaces - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
using namespace std;
using namespace lemon;
using namespace std::chrono;


/* Macros - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
// defines the maximum number of vertices in the closed neighborhood of a
// vertex in hexagonal grid with finite number of rows
#define NEIGHBOORHOD_SIZE 4


/* Data Structure Declaration - - - - - - - - - - - - - - - - - - - - - */
/*
 * Struct: vertex
 * --------------
 * Represents a vertex in configuration graph
 *
 *    bar: bit array that indicantes which vertex, from the
 *         hexagonal grid H_k, belongs to the code; which are represented
 *         in the following way (for a 4-bar)
 *         ...
 *         line 3: 12-13-14-15
 *                 |      |
 *         line 2: 8--9--10-11
 *                    |     |
 *         line 1: 4--5--6--7
 *                 |     |
 *         line 0: 0--1--2--3
 *
 * weight: the number of vertices that belongs to the code
 */
struct vertex
{
  int    *bar;
  double weight;
};

typedef struct vertex vertex;


/*
 * Struct: node
 * ------------
 * Represents a node of a linked list
 *
 *   next: points to the next node of the list
 *
 * vertex: points to a vertex in configuration graph (struct vertex)
 */
struct node
{
  struct node *next;
  vertex      *v;
};

typedef struct node node;


/*
 * Struct: linked_list
 * -------------------
 * Represents a linked list, where each node points to a vertex in
 * configuration graph
 *
 *        size: the number of nodes in the list
 *
 * vertex_size: the number of vertices that a vertex, in configuration
 *              graph, contains (number of columns = size of l-bar x
 *                               number of lines)
 *
 *       first: points to the first node of the list
 *
 *        last: points to the last element of the list
 */
struct linked_list
{
  int  size;
  int  vertex_size;
  node *first;
  node *last;
};

typedef struct linked_list linked_list;


/*
 * Struct: config_vertex
 * ---------------------
 * Represents the line, column and identifier of a vertex in the
 * hexagonal grid. This struct is used to map the vertices from the bar
 * to the struct of the hexagonal grid and check if a set of vertices
 * is an identifying code; restricted to the vertices of the bar
 * (discarting the firts and last columns)
 *
 *    line: the line of the vertex in hexagonal grid
 *
 *   column: the column of th vertex in hexagonal grid
 *
 * pertence: indicates if a vertex belongs to the code
 *
 * identificador: the identifier of a vertex, such that:
 *                identificador[] = -1 -> there is no vertex
 *                identificador[] =  i -> id of the vertex that belongs
 *                                        to the code
 */
struct config_vertex
{
  int line;
  int column;
  int pertence;
  int identificador[NEIGHBOORHOD_SIZE] = {-1, -1, -1, -1};
                        // the identifier has 4 positions since we are
                        // working with 1-identifying code
};

typedef struct config_vertex config_vertex;


/* Function Implementation - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 * Function: allocate_vertex
 * -------------------------
 * Allocates a vertex (struct vertex) of configuration graph
 *
 * vertex: a vertex in configuration graph
 *
 *   size: the number of vertices, from the hexagonal grid, in a
 *         vertex of the configuration graph
 *
 *  return: 1 when the vertex was allocated, otherwise, 0
 */
int allocate_vertex(vertex *v, int size)
{
  v->weight = 0;
  v->bar = NULL;
  v->bar = new (nothrow) int[size];

  if (v->bar == NULL)
      return 0;

  return 1;
}

/*
 * Function: deallocate_vertex
 * ---------------------------
 * Deallocate a vertex (configuration graph)
 *
 * vertex: a vertex of the configuration graph
 */
void deallocate_vertex(vertex *v)
{
  v->weight = 0;

  if (v->bar != NULL)
    delete[] v->bar;

  v->bar = NULL;
}

/*
 * Function: copy_vertex
 * ---------------------
 * Copies a bar code, of size tam, to a vertex of the configuration graph
 *
 *    v: points to a vertex (configuration graph)
 * size: the number of vertices in v
 *    u: array of integer, of the same size of v, which represents a bar
 *       code
 */
void copy_vertex(vertex *v, int size, int *u)
{
  int i;

  if (v->bar != NULL)
    {
      for (i = 0; i < size; i++)
	{
	  v->bar[i] = u[i];

	  // if the vertex u[i] belongs to the code, then v-> weight is
	  // incremented in one unit
	  if (u[i] == 1)
	    v->weight++;
	}

      v->weight = v->weight;
    }
}

/*
 * Function: print_vertex
 * ----------------------
 * Output to stdout the data of a vertex (configuration graph)
 *
 *    v: points to a vertex (configuration graph)
 * size: the number of vertex in v
 */
void print_vertex(vertex *v, int size)
{
  int i;

  printf("\nDensity: %f\n", v->weight);
  printf("[");

  for (i = 0; i < size -1; i ++)
    printf("%d, ", v->bar[i]);

  if (size == 0)
    printf("]");

  else
    printf("%d]\n", v->bar[size -1]);
}


/*
 * Function: init_list
 * -------------------
 * Initializes a linked list l
 *
 * l: points to a linked list
 * k: number of lines of the hexagonal grid
 * z: number of columns of the hexagonal grid
 */
void init_list(linked_list *l, int k, int z)
{
  l->size = 0;
  l->vertex_size = z * k;
  l->first = NULL;
  l->last = NULL;
}

/*
 * Function: append_list
 * ---------------------
 * Appends an element to a linked list
 *
 * l: points to a linked list
 * c: an array which represents a bar code
 *
 * returns: 1 if the element was successfully appended, otherwise, 0
 */
int append_list(linked_list *l, int *c)
{
  node *element;

  element = NULL;
  element = new (nothrow) node;

  if (element == NULL)
    return 0;

  element->v = NULL;
  element->v = new (nothrow) vertex;

  if (element->v == NULL)
    {
      delete element;
      element = NULL;
      return 0;
    }

  if (allocate_vertex(element->v, l->vertex_size) == 0)
    {
      delete element;
      element = NULL;
      return 0;
    }

  copy_vertex(element->v, l->vertex_size, c);
  element->next = NULL;

  if (l->size == 0)
    l->first = element;

  else
    l->last->next = element;

  l->last = element;
  l->size++;
  return 1;
}

/*
 * Function: deallocate_list
 * -------------------------
 * Deallocate a linked list l
 *
 * l: points to a linked lists
 */
void deallocate_list(linked_list *l)
{
  node *element;
  node *aux;

  if (l->size != 0)
    {
      element = l->first;

      while (element != NULL)
	{
	  deallocate_vertex(element->v);
	  delete element->v;
	  element->v = NULL;
	  aux = element;
	  element = element->next;
	  delete aux;
	}

      l->first = NULL;
      l->last = NULL;
      l->size = 0;
    }
}

/*
 * Function: print_list
 * --------------------
 * Ouputs to stdout the data of a linked list l
 *
 * l: points to a linked list
 */
void print_list(linked_list *l)
{
  node *element;

  printf("Size: %d\t Size of a vertex (configuration graph): %d\n",
	 l->size, l->vertex_size);
  element = l->first;

  while (element != NULL)
    {
      print_vertex(element->v, l->vertex_size);
      element = element->next;
    }
}


/*
 * Function: next_config
 * ---------------------
 * Generates all permutations of the vertices, in the bar, in the code
 *
 * c: an array, of integers, which represents the vertices in a bar
 *
 * retuns: one of the possibile permutation of the vertices in the code
 */
int next_config(int* c, int size) {
  int k;

  k = size -1;

  while (k >= 0)
    {
      if (c[k] == 1)
	c[k--] = 0;

      else
	{
	  c[k] = 1;
	  break;
	}
    }

  return k;
}


/*
 * Function: identifier_size
 * -------------------------
 * Computes the number of vertices in an identifier
 *
 *   id: an array of integers that represents an identifier
 * size: the number of vertices in the array id
 *
 * returns: the number of vertices in the array id which are in the
 *          code
 */
int identifier_size(int *id, int size)
{
  int i;
  int size_id;

  size_id = 0;

  for (i = 0; i < size; i++)
    if (id[i] != -1) // -1 means that there is no vertex
      size_id++;

  return size_id;
}


/*
 * Function: equal_identifier
 * --------------------------
 * Check if two identifiers have the same vertices, that is, if they are
 * equal
 *
 *  id1: array of integers that represents the vertices of an identifier
 *
 *  id2: array of integers that represents the vertices of an identifier
 *
 * size: the number of vertices in a identifier (it applies to id1 and id2)
 *
 * returns: true if id1 == id2, otherwise, false
 */
bool equal_identifier(int *id1, int *id2, int size)
{
  int qtde_elementos_iguais;
  int size_id1, size_id2;
  int i, j;

  qtde_elementos_iguais = 0;
  i = 0;

  // computes the actual size of the identifiers id1 and id2
  // this is done because an identifier has a fixed size of size
  // and when it is created, the identifier is initialized with -1
  // meaning that no vertex was assigned to the identifier
  size_id1 = 0;
  size_id2 = 0;

  for (i = 0; i < size; i++)
    {
      if (id1[i] != -1)
	size_id1++;

      if (id2[i] != -1)
	size_id2++;
    }

  // if size_d1 != size_id2, as id1 and id2 are not empty, then there is an
  // element from id1 which does not belogs to id2, or vice-versa; then
  // id1 and id2 are distinct
  if (size_id1 != size_id2)
    return false;

  // note that, from this part, size_id1 = size_id2
  i = 0;
  while (i < size_id1)
    {
      j = 0;

      while (j < size_id2)
	{
	  if (id1[i] == id2[j])
	    {
	      qtde_elementos_iguais++;
	    }

	  j++;
	}

      i++;
    }

  if (qtde_elementos_iguais == size_id1)
    return true;

  return false;
}


/*
 * Function: obtain_weigth
 * -----------------------
 * Computes the number of vertices, in bar of size tam, belongs to the
 * code
 *
 *   c: an array which represents a bar o length tam
 * tam: the number of vertices in the array c
 *
 * returns: the number of elements in c which belongs to the code
 */
int obtain_weigth(int *c, int size)
{
  int i;
  int weight = 0;

  for (i = 0; i < size; i++)
    {
      if (c[i] == 1)
	weight++;
    }

  return weight;
}


/*
 * Function: create_vertex_graph_cfg
 * ---------------------------------
 * Creates a vertex of configuration graph
 *
 * k: number of lines of the hexagonal grid
 * z: number of columns of the hexagonal grid
 * H: points to a SmathGraph
 */
void create_vertex_graph_cfg(int k, int z, SmartGraph *H)
{
  int size;
  int parity;
  int i, j;

  // number of vertices, from the hexagonal grid, that a vertex from
  // configuration graph has
  size = k * z;

  // creates the vertices
  for (i = 0; i < size; i++)
    H->addNode();

  // add edges between vertices in the same line
  for(i = 0; i < size; i = i +z)
    for(j = 0; j < z -1; j++)
      H->addEdge(H->nodeFromId(i +j), H->nodeFromId(i +j +1));

  // add the edges that represents the columns of the hexagonal grid
  // we assume that we start at line 0, and also, even lines has
  // parity -1
  parity = -1;

  // loop condition is i < size -z because, at each step we advance z
  // vertices (number of vertices in a column)
  // when z = 4 we got the following structure
  for (i = 0; i < size -z; i = i +z)
    {
      if (parity == -1)
	j = 0;
      else
	j = 1;

      while (j < z)
	{
	  H->addEdge(H->nodeFromId(i +j), H->nodeFromId(i +z +j));
	  j = j +2;
	}

      parity = parity * (-1);
    }
}


/*
 * Function: create_graph_cfg
 * --------------------------
 * Creates all the vertices of a configuration graph
 *
 * linked_list: list which contains all barcodes
 *
 *           k: number of lines of the hexagonal grid
 *
 *           z: number of columns of the hexagonal grid
 *
 * returns: the amount of vertices created
 */
int create_graph_cfg(linked_list *l, int k, int z)
{
  int size;       // the size of the array which represents a bar
  int *c;         // the array used to represent a bar
  SmartGraph H;   // graph used to check if a set of vertices is a bar
                  // code
  bool valid_bar; // true if a set of vertices is a bar code, otherwise
                  // is false
  int i, j;

  // allocates the array all the possible barcode of a bar
  c = nullptr;
  size = k * z;
  c = new (nothrow) int[size];

  if (c == nullptr)
    return 0;

  // initialize the array c with zeros, because 0 means a
  // vertex, in the bar, boes not belongs to code
  for (i = 0; i < size; i++)
    c[i] = 0;

  // generates the graph used to check if a set of vertices is a bar code
  SmartGraph::NodeMap<config_vertex> map_vertice_id(H);
  create_vertex_graph_cfg(k, z, &H);

  // does a mapping of a vertex, from the bar, to an order pair
  // (z, k) (column, line) which representes a vertex in hexagonal grid
  for(i = 0; i < size; i = i +z)
    for(j = 0; j < z; j++)
      {
        map_vertice_id[H.nodeFromId(i +j)].column = j;
        map_vertice_id[H.nodeFromId(i +j)].line = i/z;
      }

  // check if a set of vertices is a bar code
  while (next_config(c, size) >= 0)
    {
      valid_bar = true;

      if (valid_bar == true)
	{
	  // builds the identifier for the vertices (discarting the
	  // first and last columns) in the bar
	  for (SmartGraph::NodeIt vertex(H); vertex != INVALID;
	       ++vertex)
	    {
	      if (map_vertice_id[vertex].column != 0 &&
		  map_vertice_id[vertex].column != z -1)
		{
		  i = 0;

		  if (c[H.id(vertex)] == 1)
		    {
		      map_vertice_id[vertex].identificador[i] =
			H.id(vertex);
		      i++;
		    }

		  for (SmartGraph::IncEdgeIt aresta(H, vertex);
		       aresta != INVALID; ++aresta)
		    {
		      if (H.id(H.u(aresta)) != H.id(vertex) &&
			  c[H.id(H.u(aresta))] == 1)
			{
			  map_vertice_id[vertex].identificador[i] =
			    H.id(H.u(aresta));
			  i++;
			}

		      else if (H.id(H.v(aresta)) != H.id(vertex) &&
			       c[H.id(H.v(aresta))] == 1)
			{
			  map_vertice_id[vertex].identificador[i] =
			    H.id(H.v(aresta));
			  i++;
			}
		    }
		}
	    }

	  // check if the identifiers (dispate the first and last column)
	  // are not empty
	  for (SmartGraph::NodeIt vertex(H); vertex != INVALID;
	       ++vertex)
	    {
	      if (map_vertice_id[vertex].column != 0 &&
		  map_vertice_id[vertex].column != z -1)
		if (identifier_size(map_vertice_id[vertex].identificador, NEIGHBOORHOD_SIZE) == 0)
		  valid_bar = false;
	    }
	}

      // check if the identifiers are pairwise distinct
      if (valid_bar == true)
	{
	  for (SmartGraph::NodeIt v1(H); v1 != INVALID; ++v1)
	    {
	      if (map_vertice_id[v1].column != 0 &&
		  map_vertice_id[v1].column != z -1)
		for (SmartGraph::NodeIt v2(H); v2 != INVALID; ++v2)
		  {
		    if (H.id(v1) != H.id(v2) &&
			map_vertice_id[v2].column != 0 &&
			map_vertice_id[v2].column != z -1)
		      if (equal_identifier(
				  map_vertice_id[v1].identificador,
				  map_vertice_id[v2].identificador,
				  NEIGHBOORHOD_SIZE) == true)
			valid_bar = false;
		  }
	    }
	}

      if (valid_bar == true)
	append_list(l, c);

      // re-initialize the identifiers for the next iteration
      for (SmartGraph::NodeIt w(H); w != INVALID; ++w)
	{
	  map_vertice_id[w].identificador[0] = -1;
	  map_vertice_id[w].identificador[1] = -1;
	  map_vertice_id[w].identificador[2] = -1;
	  map_vertice_id[w].identificador[3] = -1;
	}
    }

  delete[] c;
  c = nullptr;
  return 1;
}


/*
 * Function: check_bar_code
 * ------------------------
 * Check if a set of vertices induces a bar code
 *
 * bar1: represents a set of vertices in a bar
 * bar2: represents a set of vertices in a bar
 * size: the size of the bars 1 and 2
 *    k: the number of lines in hexagonal grid
 *
 * returns: 1 if the union of bars 1 and 2 induces a bar code,
 *          otherwise, returns 0
 *
 * how the vertices of the bars 1 and 2 are represented
 * ...
 * line 3: 12-13-14-15 == 12-13-14-15
 *         |      |       |      |
 * line 2: 8--9--10-11 == 8--9--10-11
 *            |     |        |     |
 * line 1: 4--5--6--7  == 4--5--6--7
 *         |     |        |     |
 * line 0: 0--1--2--3  == 0--1--2--3
 *         bar1           bar2
 */
int check_bar_code(int *bar1, int *bar2, int z, int k)
{
  SmartGraph H;     // graph that represents that union of the bars 1 and 2
  int num_vertices; // the number of vertices in H
  int i, j;

  // creates the graphi which represents the union of bar 1 and bar 2
  num_vertices = 2 * z *k;

  SmartGraph::NodeMap<config_vertex> map_vertice_id(H);
  create_vertex_graph_cfg(k, 2 * z, &H);

  // set the column and line for the vertices of H, maping the vertices
  // of H into the struct of the hexagonal grid
  for(i = 0; i < num_vertices; i = i +(2 * z))
    for(j = 0; j < 2 * z; j++)
      {
        map_vertice_id[H.nodeFromId(i +j)].column = j;
        map_vertice_id[H.nodeFromId(i +j)].line = i/(2* z);
      }

  // set which vertices belongs to the code according to the information
  // in bar code 1 and 2
  for (i = 0; i < num_vertices; i = i + (2 * z))
    for (j = 0; j < z; j++)
      {
	map_vertice_id[H.nodeFromId(i +j)].pertence = bar1[i/2 +j];
	map_vertice_id[H.nodeFromId(i +j +z)].pertence = bar2[i/2 +j];
      }

  // create the identifiers
  for (SmartGraph::NodeIt v(H); v != INVALID; ++v)
    {
      i = 0;

      if (map_vertice_id[v].pertence == 1)
	{
	  map_vertice_id[v].identificador[0] = H.id(v);
	  i++;
	}

      for (SmartGraph::IncEdgeIt e(H, v); e != INVALID; ++e)
	{
	  if (H.id(H.u(e)) != H.id(v) &&
	      map_vertice_id[H.u(e)].pertence == 1)
	    {
	      map_vertice_id[v].identificador[i] = H.id(H.u(e));
	      i++;
	    }
	  else if (H.id(H.v(e)) != H.id(v) &&
		   map_vertice_id[H.v(e)].pertence == 1)
	    {
	      map_vertice_id[v].identificador[i] = H.id(H.v(e));
	      i++;
	    }
	}
    }

  // check if there is an empty identifier
  for (SmartGraph::NodeIt v(H); v != INVALID; ++v)
    {
      if (map_vertice_id[v].column > 0 &&
	  map_vertice_id[v].column < 2 * z -1)
	if (identifier_size(map_vertice_id[v].identificador,
			     NEIGHBOORHOD_SIZE) == 0)
	    return 0;
    }

  // check if the identifiers are pairwise distinct
  for(SmartGraph::NodeIt v(H); v != INVALID; ++v)
    {
      if (map_vertice_id[v].column > 0 &&
	  map_vertice_id[v].column < 2 * z -1)
	{
	for (SmartGraph::NodeIt u(H); u != INVALID; ++u)
	  if (map_vertice_id[u].column > 0 &&
	      map_vertice_id[u].column < 2 * z -1 &&
	      H.id(u) != H.id(v))
	    {
	      if (equal_identifier(map_vertice_id[v].identificador,
				   map_vertice_id[u].identificador,
				   NEIGHBOORHOD_SIZE) == true)
		return 0;
	    }
	}
    }

  return 1;
}


/* Main Program - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int main(int argc, char **argv)
{
  int k;                     // number of lines of the hexagonal
                             // grids
  SmartDigraph G;            // digraph which represents a
                             // configuration graph
  SmartGraph H;              // graph used to check if a set of
                             // vertices is a bar code
  linked_list bar_code_list; // list with all bar codes
  int i, j, h;
  node *nodeAux;
  node *nodeV, *nodeU;
  ofstream code_file;

  // check if the all the arguments were properly passed
  if (argc != 2)
    {
      cerr << "Invalid number of arguments!\n";
      cerr << "Usage: " << argv[0] << " <number of lines>\n";
      return EXIT_FAILURE;
    }
  else
    k = atoi(argv[1]);

  // builds all the bar codes
  auto start = std::chrono::high_resolution_clock::now();
  init_list(&bar_code_list, k, NEIGHBOORHOD_SIZE);

  if (create_graph_cfg(&bar_code_list, k, NEIGHBOORHOD_SIZE) == 0)
    {
      cerr << "ERRO: It was not possible to generate the bar codes!\n";
      deallocate_list(&bar_code_list);
      return EXIT_FAILURE;
    }

  auto end = std::chrono::high_resolution_clock::now();
  cout << "Time to build all bar codes: "
       << chrono::duration_cast<chrono::hours>(end - start).count()
       << "h "
       << chrono::duration_cast<chrono::minutes>(end - start).count() % 60
       << "m "
       << chrono::duration_cast<chrono::seconds>(end - start).count() % 60
       << "s "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() % 1000
       << "ms "
       << chrono::duration_cast<chrono::nanoseconds>(end - start).count() % 1000000
       << "ns\n";

  // builds the configuration graph
  start = std::chrono::high_resolution_clock::now();
  for (i = 0; i < bar_code_list.size; i++)
    G.addNode();

  // assign a bar code for each vertex of the configuration graph
  SmartDigraph::NodeMap<node *> map_vertices(G);
  nodeAux = bar_code_list.first;

  for (SmartDigraph::NodeIt v(G); v != INVALID; ++v)
    {
      map_vertices[v] = nodeAux;
      nodeAux = nodeAux->next;
    }

  end = std::chrono::high_resolution_clock::now();
  cout << "Time to build all the vertices: "
       << chrono::duration_cast<chrono::hours>(end - start).count()
       << "h "
       << chrono::duration_cast<chrono::minutes>(end - start).count() % 60
       << "m "
       << chrono::duration_cast<chrono::seconds>(end - start).count() % 60
       << "s "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() % 1000
       << "ms "
       << chrono::duration_cast<chrono::nanoseconds>(end - start).count() % 1000000
       << "ns\n";

  // build all the edges of the configuration graph
  start = std::chrono::high_resolution_clock::now();
  create_vertex_graph_cfg(k, 2 * NEIGHBOORHOD_SIZE, &H);

  # pragma omp parallel for
  for (SmartDigraph::NodeIt u(G); u != INVALID; ++u)
    {
      for (SmartDigraph::NodeIt v(G); v != INVALID; ++v)
	{
	  nodeU = map_vertices[u];
	  nodeV = map_vertices[v];

          if (check_bar_code(nodeU->v->bar, nodeV->v->bar,
			 NEIGHBOORHOD_SIZE, k) == 1)
	    {
	    G.addArc(u, v);
	    }
	}
    }

  // creates a map to add a weight to the edges
  SmartDigraph::ArcMap<double> map_weight(G);
  SmartDigraph::Node u;

  for (SmartDigraph::ArcIt arco(G); arco != INVALID; ++arco)
    {
      u = G.target(arco);
      nodeAux = map_vertices[u];
      map_weight[arco] = nodeAux->v->weight;
    }
  end = std::chrono::high_resolution_clock::now();
  cout << "Time to build all the edges: "
       << chrono::duration_cast<chrono::hours>(end - start).count()
       << "h "
       << chrono::duration_cast<chrono::minutes>(end - start).count() % 60
       << "m "
       << chrono::duration_cast<chrono::seconds>(end - start).count() % 60
       << "s "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() % 1000
       << "ms "
       << chrono::duration_cast<chrono::nanoseconds>(end - start).count() % 1000000
       << "ns\n";

  cout << "Number of vertices: " << countNodes(G) << "\t";
  cout << "Number of edges : " << countArcs(G) << endl;

  // execute an algorithm to find a minimum mean cycle
  start = std::chrono::high_resolution_clock::now();
  HartmannOrlinMmc<SmartDigraph, SmartDigraph::ArcMap<double>>
    MMC(G, map_weight);
  Path<SmartDigraph> mmc_path;
  MMC.cycle(mmc_path);
  MMC.run();
  end = std::chrono::high_resolution_clock::now();
  cout << "Time to run Hartmann and Orlin algorithm: "
       << chrono::duration_cast<chrono::hours>(end - start).count()
       << "h "
       << chrono::duration_cast<chrono::minutes>(end - start).count() % 60
       << "m "
       << chrono::duration_cast<chrono::seconds>(end - start).count() % 60
       << "s "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() % 1000
       << "ms "
       << chrono::duration_cast<chrono::nanoseconds>(end - start).count() % 1000000
       << "ns\n";

  code_file.open("code_pattern.txt");
  cout << "columns: " << MMC.cycleSize() * NEIGHBOORHOD_SIZE << endl;
  cout << "density: " << (MMC.cycleMean() * MMC.cycleSize())/ (k * MMC.cycleSize() * NEIGHBOORHOD_SIZE)
       << "\n";
  code_file << k << " " << MMC.cycleSize() * NEIGHBOORHOD_SIZE << " "
	    << MMC.cycleMean() << "\n";

  // output the pattern of the code found (minimum mean cycle)
  SmartDigraph::Node v;
  h = 0;

  for (Path<SmartDigraph>::ArcIt arco(mmc_path); arco != INVALID; ++arco)
    {
      u = G.source(arco);
      for (j = 0; j < NEIGHBOORHOD_SIZE * k; j = j +NEIGHBOORHOD_SIZE)
	{
	  for (i = 0; i < NEIGHBOORHOD_SIZE; i++)
	    {
	      if (map_vertices[u]->v->bar[i+j] == 1)
		{
		  cout << "(" << i + NEIGHBOORHOD_SIZE * h <<
		    "," << (j/4) +1 << ") ";
		  code_file << "(" << i + NEIGHBOORHOD_SIZE * h <<
		    "," << (j/4) +1 << ") ";
		}
	    }
	}
      h++;
    }
  cout << "\n";
  code_file << "\n";
  code_file.close();

  deallocate_list(&bar_code_list);
  return EXIT_SUCCESS;
}
