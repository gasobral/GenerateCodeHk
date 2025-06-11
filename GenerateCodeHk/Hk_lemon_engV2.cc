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


/* Bibliotecas - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
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

// define the amount of columns in a 4-bar
#define AMT_COLUMNS 4

// define the amount of columns that two 4-bars overlap
#define AMT_OVERLAP 2


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
  int size;
  int vertex_size;
  node  *first;
  node  *last;
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
 *       line: the line of the vertex in hexagonal grid
 *
 *     column: the column of th vertex in hexagonal grid
 *
 * identifier: the identifier of a vertex, such that:
 *             identifier[] = -1 -> there is no vertex
 *             identifier[] =  i -> id of the vertex that belongs
 *                                  to the code
 */
struct config_vertex
{
  int line;
  int column;
  int identifier[NEIGHBOORHOD_SIZE] = {-1, -1, -1, -1};
                        // o identifier tem 4 posições pois estamos
                        // trabalhando com o conceito de 1-código de
                        // identificação
};

typedef struct config_vertex config_vertex;


/* Function Implementation - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 * Function: init_bar
 * ------------------
 * Initialize a bar
 *
 * v: points to a vertex in configuration graph (struct vertex)
 */
void init_bar(vertex *v)
{
  v->bar = nullptr;
  v->weight = 0;
}

/*
 * Function: create_bar
 * --------------------
 * Allocates a bar of given size
 *
 *       v: points a vertex, which represents a bar
 *
 *    size: the amount of vertices, of the hexgonal grid, represented in
 *          the bar
 *
 * returns: 1, if the bar was allocated, otherwise, 0
 */
int create_bar(vertex *v, int size)
{
  int i;

  try
    {
      v->bar = new int[size];
    }
  catch (bad_alloc& e)
    {
      cerr << "bad_alloc: " << e.what() << "\n";
      v->bar = nullptr;
      v->weight = 0;
      return 0;
    }

  // initialize the array with zeros
  // 0 means that a vertex (from the hexagonal grid) does not belongs
  // to the code
  for (i = 0; i < size; i++)
    v->bar[i] = 0;

  return 1;
}

/*
 * Function: deallocate_bar
 * ------------------------
 * Deallocates a bar
 *
 * v: points a vertex, which represents a bar
 */
void deallocate_bar(vertex *v)
{
  if (v->bar != nullptr)
    {
      delete[] v->bar;
      v->bar = nullptr;
    }

  v->weight = 0;
}

/*
 * Function: print_bar
 * -------------------
 * Prints the data of a vertex associate to a bar
 *
 *       v: points a vertex, which represents a bar
 *
 *    size: the amount of vertices, of the hexgonal grid, represented in
 *          the bar
 */
void print_bar(vertex *v, int size)
{
  int i;

  cout << "weight: " << v->weight << "  [";

  for (i = 0; i < size -1; i++)
    cout << v->bar[i] << ", ";

  if (size != 0)
    cout << v->bar[size -1];

  cout << "]\n";
}


/*
 * Function: generate_all_bars
 * ---------------------------
 * Generate all the permutations of the vertices (hexagonal grid) of a bar
 * in the code of a given size
 *
 *       v: points a vertex, which represents a bar
 *
 *    size: the amount of vertices, of the hexgonal grid, represented in
 *          the bar
 *
 * returns: of the permutations
 */
int generate_all_bars(vertex* v, int size) {
  int k;

  k = size -1;

  while (k >= 0)
    {
      if (v->bar[k] == 1)
	v->bar[k--] = 0;

      else
	{
	  v->bar[k] = 1;
	  break;
	}
    }

  return k;
}

/*
 * Function: compute_bar_weight
 * ----------------------------
 * Computes the weight of a vertex which represents a bar
 *
 *       v: points a vertex, which represents a bar
 *
 *    size: the amount of vertices, of the hexgonal grid, represented in
 *          the bar
 *
 * returns: the weight (numeber of vertices in the code) of v
 */
int compute_bar_weight(vertex *v, int size)
{
  int i, weight;

  weight = 0;

  for (i = 0; i < size; i++)
    {
      if (v->bar[i] == 1)
	weight++;
    }

  return weight;
}

/*
 * Function: copy_bar
 * ------------------
 * Copies the data from v2 (source) to v1 (target), of same size
 *
 * v1: points a veterx which represents a bar
 *
 * v2: points a veterx which represents a bar
 *
 * size: the number of vertices contained in the bar
 *
 */
void copy_bar(vertex *v1, vertex *v2, int size)
{
  int i;

  v1->weight = v2->weight;

  for (i = 0; i < size; i++)
    v1->bar[i] = v2->bar[i];
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
 * Function: deallocate_list
 * -------------------------
 * Deallocate a linked list
 *
 * l: points to a linked list
 */
void deallocate_list(linked_list *l)
{
  node *elemento;
  node *aux;

  if (l->size != 0)
    {
      elemento = l->first;

      while (elemento != NULL)
	{
	  deallocate_bar(elemento->v);
	  delete elemento->v;
	  elemento->v = nullptr;
	  aux = elemento;
	  elemento = elemento->next;
	  delete aux;
	}

      l->first = nullptr;
      l->last = nullptr;
      l->size = 0;
    }
}

/*
 * Function: append_list
 * ---------------------
 * Appends an element to a linked list
 *
 * l: points to a linked list
 * v: an array which represents a bar code
 *
 * returns: 1 if the element was successfully appended, otherwise, 0
 */
int append_list(linked_list *l, vertex *v)
{
  node *node_list;

  try
    {
      node_list = new node;
    }
  catch (bad_alloc& e)
    {
      cerr << "A node could not be allocated!\n" << e.what() << "\n";
      return 0;
    }

  try
    {
      node_list->v = new vertex;
    }
  catch (bad_alloc& e)
    {
      cerr << "It was not possible to allocate a bar!\n" << e.what()
	   << "\n";
      delete node_list;
      node_list = nullptr;
      return 0;
    }

  node_list->next = nullptr;

  if (create_bar(node_list->v, l->vertex_size) == 0)
    {
      delete node_list;
      node_list = nullptr;
      return 0;
    }

  copy_bar(node_list->v, v, l->vertex_size);

  if (l->size == 0)
    l->first = node_list;
  else
    l->last->next = node_list;

  l->last = node_list;
  l->size++;
  return 1;
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
      print_bar(element->v, l->vertex_size);
      element = element->next;
    }
}


/*
 * Function: allocate_hexagonal_grid
 * ---------------------------------
 * Allocates the hexgonal grid with k lines and z columns
 *
 * k: the number of lines of the hexagonal grid
 * z: the number of columns of the hexagonal grid
 * H: points to the graph which will represent the hexagonal grid
 */
void allocate_hexagonal_grid(int k, int z, SmartGraph *H)
{
  int size;
  int parity;
  int i, j;

  // number of vertices
  size = k * z;

  // creates the vertices
  for (i = 0; i < size; i++)
    H->addNode();

  // add edges which represents the lines of the hexagonal grid
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
 * Function: identifier_size
 * -------------------------
 * Computes the number of vertices in an identifier
 *
 *      id: an array of integers that represents an identifier
 *
 *    size: the number of vertices in the array id
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
    if (id[i] != -1)
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
  int amt_equal;
  int size_id1, size_id2;
  int i, j;

  amt_equal = 0;
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
	      amt_equal++;
	    }

	  j++;
	}

      i++;
    }

  if (amt_equal == size_id1)
    return true;

  return false;
}


/*
 * Function: generate_all_barcodes
 * -------------------------------
 * Generates a list wit all bar codes
 *
 * linked_list: the list which has all the bar codes
 *
 *           k: number of lines of the hexagonal grid
 *
 *           z: number of columns of the hexagonal grid
 *
 *     returns: 1 if the list was created, otherwise, 0
 */
int generate_all_barcodes(linked_list *l, int k, int z)
{
  vertex v;       // vertex which represents a bar
  SmartGraph H;   // graph used to check if a bar is a bar code (valid)
  bool valid_bar; // true if the bar, associated to v, is a bar code,
                  // otherwise, false
  int i, j;


  // initialize the vertex which will be used to generate all the
  // possible bars
  init_bar(&v);

  if (create_bar(&v, l->vertex_size) == 0)
    return 0;

  // creates the hexagonal grid which will be used to check if a bar
  // is a bar code
  SmartGraph::NodeMap<config_vertex> map_vertex_id(H);
  allocate_hexagonal_grid(k, z, &H);

  // do the maping from a vertex from H to a vertex in the hexagonal grid
  // (z, k) (a vertex from the hexagonal grid)
  for(i = 0; i < l->vertex_size; i = i +z)
    for(j = 0; j < z; j++)
      {
        map_vertex_id[H.nodeFromId(i +j)].column = j;
        map_vertex_id[H.nodeFromId(i +j)].line = i/z;
      }

  // loop that generates all the bar code
  while (generate_all_bars(&v, l->vertex_size) >= 0)
    {
      v.weight = compute_bar_weight(&v, l->vertex_size);
	  // build the identifiers for all vertices
	  for (SmartGraph::NodeIt vertice(H); vertice != INVALID;
	       ++vertice)
	    {
	      if (map_vertex_id[vertice].column != 0 &&
		  map_vertex_id[vertice].column != z -1)
		{
		  i = 0;

		  if (v.bar[H.id(vertice)] == 1)
		    {
		      map_vertex_id[vertice].identifier[i] =
			H.id(vertice);
		      i++;
		    }

		  for (SmartGraph::IncEdgeIt aresta(H, vertice);
		       aresta != INVALID; ++aresta)
		    {
		      if (H.id(H.u(aresta)) != H.id(vertice) &&
			  v.bar[H.id(H.u(aresta))] == 1)
			{
			  map_vertex_id[vertice].identifier[i] =
			    H.id(H.u(aresta));
			  i++;
			}

		      else if (H.id(H.v(aresta)) != H.id(vertice) &&
			       v.bar[H.id(H.v(aresta))] == 1)
			{
			  map_vertex_id[vertice].identifier[i] =
			    H.id(H.v(aresta));
			  i++;
			}
		    }
		}
	    }

	  valid_bar = true;

	  // check if there is an empty identifier
	  for (SmartGraph::NodeIt vertice(H); vertice != INVALID;
	       ++vertice)
	    {
	      if (map_vertex_id[vertice].column != 0 &&
		  map_vertex_id[vertice].column != z -1)
		if (identifier_size(map_vertex_id[vertice].identifier, NEIGHBOORHOD_SIZE) == 0)
		  valid_bar = false;
	    }

	  // check if there is two distinct vertices with the same identifier
          if (valid_bar == true)
	    {
	      for (SmartGraph::NodeIt v1(H); v1 != INVALID; ++v1)
		{
		  if (map_vertex_id[v1].column != 0 &&
		      map_vertex_id[v1].column != z -1)
		    for (SmartGraph::NodeIt v2(H); v2 != INVALID; ++v2)
		      {
			if (H.id(v1) != H.id(v2) &&
			    map_vertex_id[v2].column != 0 &&
			    map_vertex_id[v2].column != z -1)
			  if (equal_identifier(map_vertex_id[v1].identifier, map_vertex_id[v2].identifier, NEIGHBOORHOD_SIZE) == true)
			    valid_bar = false;
		      }
		}
	    }

	  if (valid_bar == true)
	    append_list(l, &v);

	  // reinitialize the identifier for the next iteration
          for (SmartGraph::NodeIt w(H); w != INVALID; ++w)
	    {
	      map_vertex_id[w].identifier[0] = -1;
	      map_vertex_id[w].identifier[1] = -1;
	      map_vertex_id[w].identifier[2] = -1;
	      map_vertex_id[w].identifier[3] = -1;
	    }
    }

  deallocate_bar(&v);
  return 1;
}


/*
 * Function: check_unon_bars
 * -------------------------
 * Check if the union of two bars, by overlaping two columns,
 * forms a bar code.
 *
 *   v1: points to a vertex which represents a bar code
 *   v2: points to a vertex which represents a bar code
 * size: the number of vertices in v1 (or v2)
 *    k: number of lines of the hexagonal grids
 *
 * return: 1 if bar codes v1 and v2 froms a bar code, otherwise, 0
 *
 *  representation of how the union of v1 and v2 are made
 *                columns that overlap (must have the same pattern)
 *                ____________
 *                |           |
 * line 3: 12-13-14-15 == 12-13-14-15
 *         |      |       |      |
 * line 2: 8--9--10-11 == 8--9--10-11
 *            |     |        |     |
 * line 1: 4--5--6--7  == 4--5--6--7
 *         |     |        |     |
 * line 0: 0--1--2--3  == 0--1--2--3
 *               |           |
 *               -------------
 *         v1             v2
 */
int check_unon_bars(vertex *v1, vertex *v2,
	       vertex *new_vertex, int size, int k)
{
  SmartGraph H;          // graph used to check if the union of v1 and v2,
                         // overlaping two columns, forms a bar code
  int amt_columns_new_v; // number of vertices in H
  int i, j;

  // check if v1 and v2 overlaps
  for (i = 0; i < size; i = i + AMT_COLUMNS)
    {
      if (v1->bar[i+2] != v2->bar[i])
	return 0;

      if (v1->bar[i+3] != v2->bar[i+1])
	return 0;
    }

  // creates a bar by the union of v1 and v2,
  // overlaping two columns
  amt_columns_new_v = AMT_COLUMNS + (AMT_COLUMNS - AMT_OVERLAP);
  j = 0;
  for (i = 0; i < size; i = i +AMT_COLUMNS)
    {
      new_vertex->bar[j] = v1->bar[i];
      new_vertex->bar[j +1] = v1->bar[i +1];
      new_vertex->bar[j +2] = v1->bar[i +2];
      new_vertex->bar[j +3] = v1->bar[i +3];

      new_vertex->bar[j +4] = v2->bar[i +2];
      new_vertex->bar[j +5] = v2->bar[i +3];

      j = j + amt_columns_new_v;
    }

  new_vertex->weight = compute_bar_weight(new_vertex, amt_columns_new_v * k);

  // create the graph to check the union of v1 and v2 is a bar code
  SmartGraph::NodeMap<config_vertex> map_vertice_id(H);
  allocate_hexagonal_grid(k, amt_columns_new_v, &H);

  // set the column and line for the vertices of H, maping the vertices
  // of H into the struct of the hexagonal grid
  for(i = 0; i < amt_columns_new_v * k; i = i + amt_columns_new_v)
    for(j = 0; j < amt_columns_new_v; j++)
      {
        map_vertice_id[H.nodeFromId(i +j)].column = j;
        map_vertice_id[H.nodeFromId(i +j)].line = i/amt_columns_new_v;
      }

  // create the identifiers
  for (SmartGraph::NodeIt vertice(H); vertice != INVALID;
       ++vertice)
    {
      if (map_vertice_id[vertice].column != 0 &&
	  map_vertice_id[vertice].column != amt_columns_new_v -1)
	{
	  i = 0;

	  if (new_vertex->bar[H.id(vertice)] == 1)
	    {
	      map_vertice_id[vertice].identifier[i] =
		H.id(vertice);
	      i++;
	    }

	  for (SmartGraph::IncEdgeIt aresta(H, vertice);
	       aresta != INVALID; ++aresta)
	    {
	      if (H.id(H.u(aresta)) != H.id(vertice) &&
		  new_vertex->bar[H.id(H.u(aresta))] == 1)
		{
		  map_vertice_id[vertice].identifier[i] =
		    H.id(H.u(aresta));
		  i++;
		}

	      else if (H.id(H.v(aresta)) != H.id(vertice) &&
		      new_vertex->bar[H.id(H.v(aresta))] == 1)
		{
		  map_vertice_id[vertice].identifier[i] =
		    H.id(H.v(aresta));
		  i++;
		}
	    }
	}
    }

  // check if there is an empty identifier
  for (SmartGraph::NodeIt vertice(H); vertice != INVALID;
       ++vertice)
    {
      if (map_vertice_id[vertice].column != 0 &&
	  map_vertice_id[vertice].column != amt_columns_new_v -1)
	if (identifier_size(map_vertice_id[vertice].identifier, NEIGHBOORHOD_SIZE) == 0)
	  return 0;
    }

  // check if the identifiers are pairwise distinct
  for (SmartGraph::NodeIt v1(H); v1 != INVALID; ++v1)
    {
      if (map_vertice_id[v1].column != 0 &&
	  map_vertice_id[v1].column != amt_columns_new_v -1)
	for (SmartGraph::NodeIt v2(H); v2 != INVALID; ++v2)
	  {
	    if (H.id(v1) != H.id(v2) &&
		map_vertice_id[v2].column != 0 &&
		map_vertice_id[v2].column != amt_columns_new_v -1)
	      if (equal_identifier(map_vertice_id[v1].identifier,
				   map_vertice_id[v2].identifier,
				   NEIGHBOORHOD_SIZE) == true)
		return 0;
	  }
    }

  return 1;
}


/*
 * Function: allocate_vertex_config_graph
 * --------------------------------------
 * Given a list with bar codes, creates the vertices of the configuration
 * graph
 *
 *   G: points to a digraph, which represents the configuration graph
 *
 *   l: linke list with bar codes
 *
 * map: a mapping from a vertex (configuration graph) to a bar code
 *      in the list l
 */
void allocate_vertex_config_graph(SmartDigraph *G, linked_list *l, node **map)
{
  SmartDigraph::Node v;
  node *No;
  int i;

  No = l->first;

  for (i = 0; i < l->size; i++)
    {
      v = G->addNode();
      map[i] = No;
      No = No->next;
    }
}


/*
 * Function: allocate_edge_config_graph
 * ------------------------------------
 * Given a list with bar codes, creates the edges of the configuration
 * graph
 *
 *   G: points to a digraph, which represents the configuration graph
 *
 *   l: linke list with bar codes
 *
 * map: a mapping from a vertex (configuration graph) to a bar code
 *      in the list l
 *
 *   k: the number of lines of the hexagonal grid
 */
int allocate_edge_config_graph(SmartDigraph *G, linked_list *l, node **map, int k)
{
  node *nodeU;
  node *nodeV;
  vertex NovaCfg;

  // creates a bar
  if (create_bar(&NovaCfg, k * (2 * AMT_COLUMNS -2)) == 0)
    return 0;

  // add the edges
  for (SmartDigraph::NodeIt u(*G); u != INVALID; ++u)
    {
      for (SmartDigraph::NodeIt v(*G); v != INVALID; ++v)
	{
	  nodeU = map[G->id(u)];
	  nodeV = map[G->id(v)];

          if (check_unon_bars(nodeU->v, nodeV->v, &NovaCfg,
			 l->vertex_size, k) == 1)
	    G->addArc(u, v);
	}
    }

  return 1;
}


/*
 * Function: compute_weigth_barcode
 * --------------------------------
 */
double compute_weigth_barcode(int *bar, int k, int z)
{
  int i, j;
  double weigth;

  weigth = 0;

  for (i = 0; i < k * z; i = i +z)
      for (j = 2; j < z; j++)
	weigth = weigth + (double) bar[i +j];

  return weigth;
}


/* Main Program - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int main(int argc, char **argv)
{
  int num_lines;              // number of lines of the hexagonal grid
  linked_list bar_codes;      // list of bar codes
  SmartDigraph G;             // digraph which represents the configuration graph
  node **map_vertex_grap_cfg; // array which makes the mapping from a vertex, in
                              // configuration graph, to a bar code
  ofstream code_file;         // file where the code will be outputed
  int config_graph_size;      // size of the configuration graph
  int config_graph_columns;   // number of columns represented in
                              // configuration graph
  int i, j, h;

  // check if the all the arguments were properly passed
  if (argc != 2)
    {
      cerr << "Invalid number of arguments!\n";
      cerr << "Usage: " << argv[0] << " <number of lines>\n";
      return EXIT_FAILURE;
    }
  else
    num_lines = atoi(argv[1]);

  // computes the time to create the graph
  auto start = std::chrono::high_resolution_clock::now();

  // builds all the bar codes
  init_list(&bar_codes, num_lines, AMT_COLUMNS);
  generate_all_barcodes(&bar_codes, num_lines, AMT_COLUMNS);

  // creates the vertex which is used to make a maping from a bar code
  // to a node list the list of bar codes
  try
    {
      map_vertex_grap_cfg = new node*[bar_codes.size];
    }
  catch (bad_alloc& e)
    {
      cerr << "It was not possible to allotace the array to map vertices!\n"
	   << e.what() << "\n";
      deallocate_list(&bar_codes);
      return EXIT_FAILURE;
    }

  // creates the vertices and the edges of the configuration graph
  allocate_vertex_config_graph(&G, &bar_codes, map_vertex_grap_cfg);

  if (allocate_edge_config_graph(&G, &bar_codes, map_vertex_grap_cfg, num_lines) == 0)
    {
      cerr << "It was not possible to create the edges of the"
	   << " configuration graph!\n";
      deallocate_list(&bar_codes);
      return EXIT_FAILURE;
    }

  // add the weights to the edges
  SmartDigraph::ArcMap<double> MapPeso(G);
  SmartDigraph::Node u;
  SmartDigraph::Node v;

  for (SmartDigraph::ArcIt arco(G); arco != INVALID; ++arco)
    {
      u = G.source(arco);
      v = G.target(arco);
      MapPeso[arco] = compute_weigth_barcode(map_vertex_grap_cfg[G.id(v)]->v->bar,
					     num_lines,
					     NEIGHBOORHOD_SIZE);
    }

  auto end = std::chrono::high_resolution_clock::now();

  cout << "Configuration Graph information\n";
  cout << "Number of vertices: " << countNodes(G) << "\t";
  cout << "Number of edges: " << countArcs(G) << "\n\n";
  cout << "Time to create the graph:\n"
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

  // compute the time to run a MMC algorithm
  start = std::chrono::high_resolution_clock::now();

  // execute an algorithm to find a minimum mean cycle
  HartmannOrlinMmc<SmartDigraph, SmartDigraph::ArcMap<double>>
    MMC(G, MapPeso);
  Path<SmartDigraph> direct_path;
  MMC.cycle(direct_path);
  MMC.run();

  end = std::chrono::high_resolution_clock::now();
  cout << "Time to run Hartmann and Orlin's MMC algorithm:\n"
       << chrono::duration_cast<chrono::hours>(end - start).count()
       << "h "
       << chrono::duration_cast<chrono::minutes>(end - start).count() % 60
       << "m "
       << chrono::duration_cast<chrono::seconds>(end - start).count() % 60
       << "s "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() % 1000
       << "ms "
       << chrono::duration_cast<chrono::nanoseconds>(end - start).count() % 1000000
       << "ns\n\n";

  // shows the information about the patter of the code
  config_graph_columns = AMT_COLUMNS + AMT_OVERLAP * (MMC.cycleSize() -2);
  config_graph_size = config_graph_columns * num_lines;

  cout << "Data about the code found:\n";
  cout << "lines: "   << num_lines     << "\t";
  cout << "columns: " << config_graph_columns   << "\t";
  cout << "density: " << (MMC.cycleMean() * MMC.cycleSize())/ (num_lines * config_graph_columns) << endl;

  code_file.open("code_pattern.txt");
  code_file << num_lines << " " << config_graph_columns
	    << " " << MMC.cycleMean() << "\n";

  // prints the identifying code
  h = 0;
  for (Path<SmartDigraph>::ArcIt arco(direct_path); arco != INVALID; ++arco)
    {
      u = G.source(arco);
      if (h == 0)
	{
	  for (i = 0; i < bar_codes.vertex_size; i = i +AMT_COLUMNS)
	    for (j = AMT_OVERLAP; j < AMT_COLUMNS; j++)
	      if (map_vertex_grap_cfg[G.id(u)]->v->bar[i +j] == 1)
		{
		  cout << "(" << j - AMT_OVERLAP << ","
		       << i/AMT_COLUMNS +1 << ") ";
		  code_file << "(" << j - AMT_OVERLAP
			    << "," << i/AMT_COLUMNS +1 << ") ";
		}
	}
      else
	{
	  for (i = 0; i < bar_codes.vertex_size; i = i +AMT_COLUMNS)
	    for (j = AMT_OVERLAP; j < AMT_COLUMNS; j++)
	      if (map_vertex_grap_cfg[G.id(u)]->v->bar[i +j] == 1)
		{
		  cout << "(" << j + AMT_COLUMNS -2 +
		    (h -1) * AMT_OVERLAP - AMT_OVERLAP
		       << "," << i/AMT_COLUMNS +1 << ") ";
		  code_file << "(" << j + AMT_COLUMNS -2 +
		    (h -1) * AMT_OVERLAP - AMT_OVERLAP
			    << "," << i/AMT_COLUMNS +1 << ") ";
		}
	}

      h++;
    }

  cout << "\n";
  code_file << "\n";
  code_file.close();

  deallocate_list(&bar_codes);
  delete[] map_vertex_grap_cfg;
  return EXIT_SUCCESS;
}
