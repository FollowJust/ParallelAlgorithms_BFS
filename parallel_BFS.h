#pragma once

#include "common.h"

#include <atomic>
#include <algorithm>
#include <cstring>
#include <execution>

#include <oneapi/tbb/parallel_scan.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/global_control.h>

static constexpr int MAX_NUM_PROCESSES = 4;

inline u32 edges_prefix_sum(std::vector<u32> &prefix_sum_vec, const Graph &graph, const std::vector<u32> &frontier) 
{
    prefix_sum_vec.resize(frontier.size(), 0);
    oneapi::tbb::parallel_for(0, (int)frontier.size(), [&graph, &frontier, &prefix_sum_vec](u32 i)
    {
        const u32 &v = frontier[i];
        prefix_sum_vec[i] = graph.adjacency_list[v].size();
    });

    std::inclusive_scan(std::execution::par, prefix_sum_vec.begin(), prefix_sum_vec.end(), prefix_sum_vec.begin(), std::plus<>(), (u32)0);
    return prefix_sum_vec.back();
}

void filter(std::vector<u32> &frontier_filtered, const std::vector<u32> &frontier_to_filter, std::vector<std::atomic_bool> &visited)
{
    std::vector<bool> predicate_res(frontier_to_filter.size(), false);
    oneapi::tbb::parallel_for((size_t)0, frontier_to_filter.size(), [&predicate_res, &frontier_to_filter, &visited](u32 i)
    {
        const u32 &v = frontier_to_filter[i];
        bool expected = false;
        predicate_res[i] = (!visited[v] && visited[v].compare_exchange_strong(expected, true));
    });

    std::vector<u32> prefix_sum(predicate_res.size(), 0);
    std::inclusive_scan(std::execution::par, predicate_res.begin(), predicate_res.end(), prefix_sum.begin(), std::plus<>(), (u32)0);

    frontier_filtered.resize(prefix_sum.back());
    oneapi::tbb::parallel_for(0, (int)frontier_to_filter.size(), [&frontier_to_filter, &frontier_filtered, &predicate_res, &prefix_sum](u32 i)
    {
        const bool is_needed = predicate_res[i];
        if (is_needed)
        {
            const u32 offset = prefix_sum[i];
            frontier_filtered[offset - prefix_sum[0]] = frontier_to_filter[i];
        }
    });
}

int64_t parallel_BFS(const Graph &graph, const Node &start_node)
{
    // Limiting max number of processes
    oneapi::tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism, MAX_NUM_PROCESSES);

    std::vector<std::atomic_bool> visited(graph.nodes.size());
    for (u32 i = 0; i < graph.nodes.size(); ++i)
    {
        visited[i].store(false);
    }

    time_point start = std::chrono::high_resolution_clock::now();

    {
        std::vector<u32> frontier;
        std::vector<u32> next_frontier;
        std::vector<u32> frontier_edges_prefix_sum;
        
        const u32 cur_node_idx = graph.get_node_idx(start_node);
        frontier.push_back(cur_node_idx);

        while (!frontier.empty())
        {
            u32 edges_next_frontier = edges_prefix_sum(frontier_edges_prefix_sum, graph, frontier);
            next_frontier.resize(edges_next_frontier);
            oneapi::tbb::parallel_for(0, (int)frontier.size(), [&graph, &frontier, &next_frontier, &frontier_edges_prefix_sum](u32 i)
            {
                const u32 &v = frontier[i];
                const std::vector<u32> &adj_node_indices = graph.adjacency_list[v];
                const u32 &base_next_frontier_idx = i != 0 ? frontier_edges_prefix_sum[i - 1] : 0;

                std::memcpy(next_frontier.data() + base_next_frontier_idx, adj_node_indices.data(), adj_node_indices.size() * sizeof(u32));
            });

            filter(frontier, next_frontier, visited);
        }
    }
    time_point end = std::chrono::high_resolution_clock::now();

    for (const bool i: visited)
    {
        // assert(i);
    }
    
    return get_microseconds(start, end);
}