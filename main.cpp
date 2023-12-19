
#include "sequential_BFS.h"
#include "parallel_BFS.h"

/*

Практическое задание #2.

Нужно реализовать параллельный bfs. От Вас требуется написать последовательную версию алгоритма  (seq) и параллельную версию (par). 
Протестировать надо на кубическом графе со стороной 500 и источником в (0, 0, 0). (Усреднить по 5 запускам) 
Сравнить время работы par на 4 процессах и seq на одном процессе - у Вас должно быть раза в 3 быстрее.  
(Если будет медленнее, то выставление баллов оставляется на моё усмотрение.) 
Учтите, что Ваш bfs должен работать на любом графе, если Вам дан его список смежности.

Нужен код на гитхабе, видимо, ссылка на него и результаты запусков. Код, который запускает, тоже должен лежать в репо.

Присылать письмо на почту с темой "CW2. <ФИО>".

*/

static constexpr u32 CUBE_GRAPH_SIZE = 300;
static constexpr u32 TESTS_COUNT = 1;

int main() 
{
    Graph g;
    g.generate_cube(CUBE_GRAPH_SIZE);
    // g.print();

    float average_sequential_time = 0;
    {
        int64_t sequential_time = 0;
        for (u32 i = 0; i < TESTS_COUNT; ++i)
        {
            sequential_time += sequential_BFS(g, Node(0, 0, 0));
        }
        average_sequential_time = (float)sequential_time / (float)TESTS_COUNT;
    }

    float average_parallel_time = 0;
    {
        int64_t parallel_time = 0;
        for (u32 i = 0; i < TESTS_COUNT; ++i)
        {
            parallel_time += parallel_BFS(g, Node(0, 0, 0));
        }
        average_parallel_time = (float)parallel_time / (float)TESTS_COUNT;
    }

    std::cout << "Average Sequential Time: " << average_sequential_time << "\n";
    std::cout << "Average Parallel Time: " << average_parallel_time << "\n";
    std::cout << "(Average Sequential Time) / (Average Parallel Time): " << (float)average_sequential_time / (float)average_parallel_time << '\n';

    return 0;
}