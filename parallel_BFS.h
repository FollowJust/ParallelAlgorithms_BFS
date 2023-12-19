#pragma once

#include "common.h"

#include <atomic>
#include <algorithm>
#include <oneapi/tbb/parallel_scan.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/global_control.h>

static constexpr int MAX_NUM_PROCESSES = 4;

inline u32 edges_prefix_sum(std::vector<u32> &prefix_sum_vec, const Graph &graph, const std::vector<u32> &frontier) 
{
    return oneapi::tbb::parallel_scan(
        oneapi::tbb::blocked_range<int>(0, (int)frontier.size()),
        0,
        [&prefix_sum_vec, &graph, &frontier](const oneapi::tbb::blocked_range<int> &r, u32 sum, bool is_final_scan) -> u32
        {
            u32 temp = sum;
            for (int i = r.begin(); i < r.end(); ++i) {
                temp = temp + graph.adjacency_list[frontier[i]].size();
                if (is_final_scan)
                {
                    prefix_sum_vec[i] = temp;
                }
            }
            return temp;
        },
        [](u32 left, u32 right) 
        {
            return left + right;
        }
    );
}

int64_t parallel_BFS(const Graph &graph, const Node &start_node)
{
    // Limiting max number of processes
    oneapi::tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism, MAX_NUM_PROCESSES);

    time_point start = std::chrono::high_resolution_clock::now();
    
    std::deque<std::atomic_bool> visited;
    
    for (u32 i = 0; i < graph.nodes.size(); ++i)
    {
        visited.emplace_back(false);
    }

    {
        std::vector<u32> frontier;
        std::vector<u32> next_frontier;
        std::vector<u32> frontier_edges_prefix_sum;
        
        const u32 cur_node_idx = graph.get_node_idx(start_node);
        frontier.push_back(cur_node_idx);

        while (!frontier.empty())
        {
            if (frontier_edges_prefix_sum.size() < frontier.size())
            {
                frontier_edges_prefix_sum.resize(frontier.size());
            }
            u32 edges_next_frontier = edges_prefix_sum(frontier_edges_prefix_sum, graph, frontier);

            next_frontier = std::move(std::vector<u32>(edges_next_frontier, std::numeric_limits<u32>::max()));

            oneapi::tbb::parallel_for(0, (int)frontier.size(), [&frontier, &graph, &next_frontier, &visited, &frontier_edges_prefix_sum](u32 i){
                const u32 &v = frontier[i];
                const std::vector<u32> &adj_node_indices = graph.adjacency_list[v];
                const u32 &base_next_frontier_idx = i != 0 ? frontier_edges_prefix_sum[i - 1] : 0;
                
                for (u32 idx = 0; idx < adj_node_indices.size(); ++idx)
                {
                    const u32 &adj_node_idx = adj_node_indices[idx];
                    bool expected = false;
                    if (visited[adj_node_idx].compare_exchange_strong(expected, true))
                    {
                        // assert((base_next_frontier_idx + idx) < next_frontier.size());
                        next_frontier[base_next_frontier_idx + idx] = adj_node_idx;
                    }
                }
            });
            
            next_frontier.erase(std::remove_if(next_frontier.begin(), next_frontier.end(), [](const u32 v){return v == std::numeric_limits<u32>::max();}), next_frontier.end());
            frontier = std::move(next_frontier);

            // frontier_edges_prefix_sum = {};
            // next_frontier = {};
        }
    }
    time_point end = std::chrono::high_resolution_clock::now();

    for (const bool i: visited)
    {
        assert(i);
    }
    
    return get_microseconds(start, end);
}