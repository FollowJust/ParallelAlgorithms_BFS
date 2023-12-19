#pragma once

#include "common.h"

int64_t sequential_BFS(const Graph &graph, const Node &start_node)
{
    time_point start = std::chrono::high_resolution_clock::now();

    std::vector<bool> visited(graph.nodes.size(), false);
    
    {
        std::queue<u32> queue;

        u32 start_node_idx = graph.get_node_idx(start_node);
        queue.push(start_node_idx);

        while (!queue.empty())
        {
            u32 cur_node = queue.front();
            queue.pop();

            const std::vector<u32> &adj_nodes_indices = graph.adjacency_list[cur_node];
            for (const u32 adj_node_idx: adj_nodes_indices)
            {
                if (!visited[adj_node_idx])
                {
                    visited[adj_node_idx] = true;
                    queue.push(adj_node_idx);
                }
            }
        }
    }

    time_point end = std::chrono::high_resolution_clock::now();

    for (const bool i: visited) 
    {
        assert(i);
    }

    return get_microseconds(start, end);
}