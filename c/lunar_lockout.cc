#include <cassert>
#include <cstdio>
#include <vector>
#include <set>
#include <queue>
#include <cstdint>
#include <algorithm>

using namespace std;

struct state
{
    char board[5][5];
};

state mirror(const state & s)
{
    state r;
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
            r.board[y][x] = s.board[4 - y][x];
    }
    return r;
}

state rotate(const state & s)
{
    state r;
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
            r.board[y][x] = s.board[x][4 - y];
    }
    return r;
}

bool is_solution(const state & s)
{
    return (s.board[2][2] == 'h');
}

bool identical(const state & lhs, const state & rhs)
{
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            if (lhs.board[y][x] != rhs.board[y][x])
            {
                return false;
            }
        }
    }
    return true;
}

bool operator<(const state & lhs, const state & rhs)
{
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            if (lhs.board[y][x] < rhs.board[y][x])
            {
                return true;
            }
            if (lhs.board[y][x] > rhs.board[y][x])
            {
                return false;
            }
        }
    }
    return false; // They are equal.
}

state offset_board()
{
    state r;
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            r.board[y][x] = y * 5 + x;
        }
    }
    return r;
}

void pr(const state & s)
{
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            if (x != 0)
            {
                printf("  ");
            }
            printf("%2d", s.board[y][x]);

            if (x == 4)
            {
                printf("\n");
            }
        }
    }
}

void prc(const state & s)
{
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            printf("%c", s.board[y][x]);
        }
        printf("\n");
    }
}
void mk_symmetries()
{
    state s;
    set<state> sym;

    queue<state> q;

    s = offset_board();
    q.push(s);

    while (!q.empty())
    {
        s = q.front();
        q.pop();

        if (sym.find(s) != sym.end())
        {
            continue;
        }

        sym.insert(s);

        q.push(mirror(s));
        q.push(rotate(s));
    }

    assert(sym.size() == 8);
}

state canonical(state s)
{
    state ss = s;
    s = rotate(s); if (s < ss) ss = s;
    s = rotate(s); if (s < ss) ss = s;
    s = rotate(s); if (s < ss) ss = s;
    s = rotate(s); // Back where we came from.
    s = mirror(s);
    s = rotate(s); if (s < ss) ss = s;
    s = rotate(s); if (s < ss) ss = s;
    s = rotate(s); if (s < ss) ss = s;
    return ss;
}

unsigned weight(uint32_t x)
{
    unsigned r = 0;
    while (x)
    {
        r += (x & 1);
        x >>= 1;
    }
    return r;
}

set<state> generate_boards(unsigned n_robots)
{
    set<state> vertices;
    for (uint32_t r = 0; r <= 0x1ffffff; r += 1)
    {
        if (weight(r) == n_robots)
        {
            for (uint32_t h = 1; h <= 0x1000000; h <<= 1)
            {
                if ((r & h) == 0)
                {
                    state s;
                    for (int y = 0; y < 5; ++y)
                    {
                        for (int x = 0; x < 5; ++x)
                        {
                            if (r & (1 << (y * 5 + x)))
                            {
                                s.board[y][x] = 'r';
                            }
                            else if (h & (1 << (y * 5 + x)))
                            {
                                s.board[y][x] = 'h';
                            }
                            else
                            {
                                s.board[y][x] = '.';
                            }
                        }
                    }
                    s = canonical(s);
                    vertices.insert(s);
                }
            }
        }
    }

    return vertices;
}

set<state> onestep(const state & s)
{
    set<state> onestep_set;

    for (int x = 0 ; x < 5; ++x)
    {
        for (int y = 0; y < 5; ++y)
        {
            if (s.board[y][x] != '.')
            {
                const int dx_arr[4] = {1, 0, -1, 0};
                const int dy_arr[4] = {0, -1, 0, 1};

                for (int dir = 0; dir < 4; ++dir)
                {
                    int dx = dx_arr[dir];
                    int dy = dy_arr[dir];

                    for (int q = 1;; ++q)
                    {
                        int xx = x + q * dx;
                        int yy = y + q * dy;
                        if (!(0 <= xx && xx < 5 && 0 <= yy && yy < 5))
                        {
                            break;
                        }

                        if (s.board[yy][xx] != '.')
                        {
                            // boom!
                            if (q > 1)
                            {
                                xx = x + (q - 1) * dx;
                                yy = y + (q - 1) * dy;

                                state ss = s;
                                ss.board[yy][xx] = ss.board[y][x];
                                ss.board[y][x] = '.';
                                ss = canonical(ss);
                                onestep_set.insert(ss);
                            }
                            break;
                        }
                    } // q-loop; loop until we hit something, or fly off the board.
                }
            }
        }
    }
    return onestep_set;
}

void solve(unsigned nr)
{
    printf("solving for nr = %u\n", nr);

    set<state> vertices_set(generate_boards(nr));
    vector<state> vertices(vertices_set.begin(), vertices_set.end());
    sort(vertices.begin(), vertices.end());

    unsigned nv = vertices.size();

    // Make edges.

    vector<vector<unsigned>> edges(nv);

    unsigned ne = 0;
    for (unsigned i = 0; i < nv; ++i)
    {
        set<state> onestep_set = onestep(vertices[i]);
        set<unsigned> reachable;
        for (auto & destination : onestep_set)
        {
            auto destination_iter = lower_bound(vertices.begin(), vertices.end(), destination);
            unsigned offset = destination_iter - vertices.begin();
            reachable.insert(offset);
        }
        ne += reachable.size();

        edges[i].assign(reachable.begin(), reachable.end());
    }

    printf("nr = %u: nv = %u ne = %u.\n", nr, nv, ne);

    // Determine distance-to-any-solution for all vertices.
    std::vector<int> distance(nv, -1);

    for (unsigned i = 0; i < nv; ++i)
    {
        if (is_solution(vertices[i]))
        {
            distance[i] = 0;
        }
    }

    for (int d = 0; ; ++d)
    {
        bool change = false;
        for (unsigned i = 0; i < nv; ++i)
        {
            if (distance[i] == -1)
            {
                for (unsigned j = 0; j < edges[i].size(); ++j)
                {
                    if (distance[edges[i][j]] == d)
                    {
                        distance[i] = d + 1;
                        change = true;
                    }
                }
            }
        }
        if (!change)
        {
            break;
        }
    }

    int max_d = *max_element(distance.begin(), distance.end());

    unsigned max_d_count = count(distance.begin(), distance.end(), max_d);

    printf("nr = %u max_d: %d -- %u\n", nr, max_d, max_d_count);

    for (unsigned i = 0; i < nv ; ++i)
    {
        if (distance[i] == max_d)
        {
            printf("nr = %u d = %d\n\n", nr, max_d);

            prc(vertices[i]);
            printf("\n");
        }
    }
    fflush(stdout);

    /*
    printf("digraph G {\n");
    // output vertices
    for (unsigned i = 0; i < nv; ++i)
    {
        if (distance[i] >= 0)
        {
            char label[1000], color[1000];

            sprintf(label, "label = \"(%d)\"", distance[i]);

            sprintf(color, " ");
            if (distance[i] == 0)
            {
                sprintf(color, "color=green");
            }
            printf("    v%u [%s %s];\n", i, label, color);
        }
    }

    // output edges
    for (unsigned i = 0; i < nv; ++i)
    {
        if (distance[i] >= 0)
        {
            for (unsigned zz = 0; zz < edges[i].size(); ++zz)
            {
                unsigned j = edges[i][zz];
                if (distance[j] >= 0)
                {
                    printf("    v%u -> v%u;\n", i, j);
                }
            }
        }
    }

    printf("}\n");
    */
}

int main()
{
    for (unsigned nr = 0; ; ++nr)
    {
        solve(nr);
    }

    return 0;
}
