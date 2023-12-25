#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <ctime>
#include <cassert>

using u32 = u_int32_t;
using s32 = u_int32_t;

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

inline int64_t get_microseconds(const time_point &start, const time_point &end)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

inline int64_t get_milliseconds(const time_point &start, const time_point &end)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline int64_t get_seconds(const time_point &start, const time_point &end)
{
    return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
}

struct Node 
{
    Node() = default;
    Node(const s32 _x, const s32 _y, const s32 _z) : x(_x), y(_y), z(_z) {};

    s32 x = 0;
    s32 y = 0;
    s32 z = 0;
};

struct Graph 
{
    u32 size;
    u32 edges;
    std::vector<Node> nodes;
    std::vector<std::vector<u32>> adjacency_list;

    void generate_cube(const u32 s)
    {
        size = s;
        edges = 0;
        u32 nodes_cnt = size * size * size;
        nodes.resize(nodes_cnt);
        adjacency_list.resize(nodes_cnt);

        for (u32 x = 0; x < size; ++x)
        {
            for (u32 y = 0; y < size; ++y)
            {
                for (u32 z = 0; z < size; ++z)
                {
                    Node node = Node(x, y, z);

                    nodes[get_node_idx(node)] = node;

                    generate_adjacency_for_node(node);
                }
            }
        }
    }

    void print() const
    {
        std::cout << "Total Nodes Count:" << nodes.size() << '\n';
        for (u32 i = 0; i < nodes.size(); ++i)
        {
            const Node &cur_node = nodes[i];
            printf("Node #%d (%d, %d, %d) [\n", i, cur_node.x, cur_node.y, cur_node.z);

            auto adj_list = adjacency_list[i];
            for (const u32 idx: adj_list)
            {
                const Node &neighbour = nodes[idx];
                printf("\tNode #%d (%d, %d, %d)\n", idx, neighbour.x, neighbour.y, neighbour.z);
            }

            printf("]\n");
        }
    }

    void generate_adjacency_for_node(const Node &node)
    {
        u32 node_idx = get_node_idx(node);

        try_to_add_neightbour(node_idx, Node(node.x + 1,  node.y,     node.z      ));
        try_to_add_neightbour(node_idx, Node(node.x - 1,  node.y,     node.z      ));

        try_to_add_neightbour(node_idx, Node(node.x,      node.y + 1, node.z      ));
        try_to_add_neightbour(node_idx, Node(node.x,      node.y - 1, node.z      ));

        try_to_add_neightbour(node_idx, Node(node.x,      node.y,     node.z + 1  ));
        try_to_add_neightbour(node_idx, Node(node.x,      node.y,     node.z - 1  ));
    }

    void try_to_add_neightbour(const u32 node_idx, const Node &neighbour)
    {
        u32 neighbour_idx = get_node_idx(neighbour);
        if (check_node_in_bounds(neighbour))
        {
            adjacency_list[node_idx].push_back(neighbour_idx);
            edges++;
        }
    }

    bool check_node_in_bounds(const Node &node) const
    {
        return node.x >= 0 && node.x < size && 
                node.y >= 0 && node.y < size &&
                node.z >= 0 && node.z < size;
    }

    u32 get_node_idx(const Node &node) const
    {
        return node.x * size * size + node.y * size + node.z;
    }
};
