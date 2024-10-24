#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include "user_code.h"

/*
    DIJKSTRA
    
    NORMAL RUN:
        - Binary size: 3968 Byte
        - Execution time: 0.08474540710449219s

    SECURE RUN:
        - Binary size: 4448 Byte
        - Execution time: 0.09491897583007812s
        - Instrumentation time: 0.002639293670654297s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0018155574798583984s
*/

#define V 6

int minDistance(int dist[], bool sptSet[])
{
    int min = INT_MAX;
    int min_index = -1;

    for (int v = 0; v < V; v++)
    {
        if (sptSet[v] == false && dist[v] <= min)
        {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

void dijkstra(int graph[V][V], int src)
{
    int dist[V];
    bool sptSet[V];

    for (int i = 0; i < V; i++)
    {
        dist[i] = INT_MAX;
        sptSet[i] = false;
    }

    dist[src] = 0;

    for (int count = 0; count < V - 1; count++)
    {
        int u = minDistance(dist, sptSet);

        sptSet[u] = true;

        for (int v = 0; v < V; v++)
        {
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v])
            {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    printf("Vertex \t Distance from Source\n");
    for (int i = 0; i < V; i++)
    {
        printf("%d \t\t %d\n", i, dist[i]);
    }
}

void start_u_code()
{
    int graph[V][V] = {
        {0, 7, 9, 0, 0, 14},
        {7, 0, 10, 15, 0, 0},
        {9, 10, 0, 11, 0, 2},
        {0, 15, 11, 0, 6, 0},
        {0, 0, 0, 6, 0, 9},
        {14, 0, 2, 0, 9, 0}};

    dijkstra(graph, 0);
}
