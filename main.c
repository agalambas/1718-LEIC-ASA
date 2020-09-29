#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct vertex {
    int d, low;
    int max, n, *next;
    int *scc;
};

typedef struct connection {
    int *o, *d;
    struct connection *next;
} Connection;

typedef struct graph {
    struct vertex *vertices;
    int n_vertices;
    int n_sccs, n_connections;
    Connection *connections;
} Graph;

#define d(g,v) ( g->vertices[v].d )
#define low(g,v) ( g->vertices[v].low )
#define n(g,v) ( g->vertices[v].n )
#define max(g,v) ( g->vertices[v].max )
#define pNext(g,v) ( g->vertices[v].next )
#define next(g,v,n) ( g->vertices[v].next[n] )
#define lastNext(g,v) ( g->vertices[v].next[g->vertices[v].n] )
#define SCC(g,v) ( g->vertices[v].scc )

#define nVertices(g) ( g->n_vertices )
#define vertices(g) ( g->vertices )
#define nSCCS(g) ( g->n_sccs )
#define nConnections(g) ( g->n_connections )
#define connections(g) ( g->connections )

#define min(a,b) ( a < b ? a : b )

void stack(Graph *G, int v, int p) {
    static int *L;
    static int l = 0;
    int scc, i;

    if( p == 0 )
        L[l++] = v;

    else if( p == 1 ) {
        l--;
        scc = v;
        for( i = l; L[i] != v; i-- )
            scc = min(scc, L[i]);
        while( L[l] != v )
            *SCC(G,L[l--]) = scc;
        *SCC(G,v) = scc;
    }

    else if( p == 2)
        L = (int*) malloc( sizeof(int)*(nVertices(G)) );
    else free(L);
}

void save_connection(Graph *G, int u, int v) {
    nConnections(G)++;

    Connection *connection = (Connection*) malloc( sizeof(Connection) );

    connection->o = SCC(G,u);
    connection->d = SCC(G,v);
    connection->next = connections(G);
    connections(G) = connection;
}

void sort_connections(Graph *G) {
    Connection *connection, *sorted, *sorted_last, *to_sort, *aux;

    sorted = (Connection*) malloc( sizeof(Connection) );
    to_sort = (Connection*) malloc( sizeof(Connection) );

    *sorted = *connections(G);
    sorted_last = sorted;
    connection = sorted->next;
    sorted->next = NULL;

    while( connection != NULL && *(connection->o) == *(sorted->o) ) {

        *to_sort = *connection;
        aux = sorted;
        if( *(aux->d) < *(to_sort->d) ) {
            while( aux->next != NULL && *(aux->next->d) < *(to_sort->d) )
                aux = aux->next;

            if( aux->next == NULL || *(aux->next->d) > *(to_sort->d) ) {
                to_sort->next = aux->next;
                aux->next = to_sort;
                if( to_sort->next == NULL )
                    sorted_last = to_sort;
            }
            else {
                nConnections(G)--;
                free(to_sort);
            }
        }
        else if( *(aux->d) > *(to_sort->d) ) {
            to_sort->next = sorted;
            sorted = to_sort;
        }
        else {
            nConnections(G)--;
            free(to_sort);
        }

        connection = connection->next;
        to_sort = (Connection*) realloc( NULL, sizeof(Connection) );
    }

    free(to_sort);
    free(connections(G));

    if( connection == NULL )
        connections(G) = sorted;
    else if( connection->o == NULL || *(connection->o) < *(sorted->o) ) {
        connections(G) = connection;
        while( connection->next != NULL && ( connection->next->o == NULL || *(connection->next->o) < *(sorted->o) ) )
            connection = connection->next;
        sorted_last->next = connection->next;
        connection->next = sorted;
    }
    else {
        connections(G) = sorted;
        sorted_last->next = connection;
    }
}

bool tarjan_visit(Graph *G, int u) {
    static int visited = 1;
    int n, v;
    d(G,u) = low(G,u) = visited++;
    stack(G, u, 0);
    for( n = 0; n < n(G,u); n++ ) {
        v = next(G,u,n);
        if( *SCC(G,v) == -1 ) {
            if ( d(G,v) == 0 && tarjan_visit(G, v) )
                save_connection(G, u, v);
            low(G,u) = min( low(G,u), low(G,v) );
        }
        else {
            save_connection(G, u, v);
        }
    }
    if( d(G,u) == low(G,u) ) {
        stack(G, u, 1);
        nSCCS(G)++;
        if( connections(G) != NULL && *(connections(G)->o) == *(SCC(G,u)) )
            sort_connections(G);
        return true;
    }
    return false;
}

void SCC_tarjan(Graph *G) {
    int u;
    stack(G, 0, 2);
    for( u = 0; u < nVertices(G); u++)
        if( d(G,u) == 0 )
            tarjan_visit(G, u);
}

int main() {

    int origin, destination, count, n_edges;
    Connection *aux, *connection;

    Graph *G = (Graph*) malloc( sizeof(Graph) );
    connections(G) = NULL;
    nSCCS(G) = 0;
    nConnections(G) = 0;

    /* ATRIBUICOES RELATIVAS AO NUMERO DE VERTICES */
    scanf("%d", &nVertices(G));
    vertices(G) = (struct vertex*) malloc( sizeof(struct vertex)*nVertices(G) );
    for( count = 0; count < nVertices(G); count++ ) {
        max(G,count) = 4;
        pNext(G,count) = (int*) malloc( sizeof(int)*max(G,count) );
        d(G,count) = 0;
        n(G,count) = 0;
        SCC(G,count) = (int*) malloc( sizeof(int) );
        *SCC(G,count) = -1;
        }

    /* ATRIBUICOES RELATIVAS AO NUMERO DE ARESTAS */
    scanf("%d", &n_edges);
    for( count = 0; count < n_edges; count++ ) {
        scanf("%d %d", &origin, &destination);
        origin--;
        if( n(G,origin) == max(G,origin) ) {
            max(G,origin) *= 2;
            pNext(G,origin) = (int*) realloc( pNext(G,origin), sizeof(int)*max(G,origin) );
        }
        lastNext(G,origin) = destination-1;
        n(G,origin)++;
    }

    SCC_tarjan(G);

    /* OUTPUT  E LIBERTACAO DE MEMORIA */
    printf("%d\n%d\n", nSCCS(G), nConnections(G));
    connection = connections(G);
    while( connection != NULL ) {
        printf("%d %d\n", *(connection->o)+1, *(connection->d)+1);
        aux = connection;
        connection = connection->next;
        free(aux);
    }

    stack(G, 0, 3);
    for( count = 0; count < nVertices(G); count++ ) {
        free(pNext(G,count));
        free(SCC(G,count));
    }
    free(vertices(G));
    free(G);
    return 0;
}
