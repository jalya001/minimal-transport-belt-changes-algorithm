#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <iomanip>
#include <cstdint>
#include <algorithm>

using namespace std;

constexpr bool DISPLAY_MODE = false;
constexpr bool BENCHMARK_MODE = false;

chrono::high_resolution_clock::time_point start;

constexpr int8_t belt_dx[5] = {0, 1, -1, 0, 0};
constexpr int8_t belt_dy[5] = {0, 0, 0, 1, -1};

uint16_t grid_width, grid_height, start_x, start_y, goal_x, goal_y;

// We put visited and belt stuff together for cache efficiency
uint8_t* grid_data;
// Bit:   7 6 5 4  3        2 1 0
// Usage: Empty    Visited  Belt dir code
constexpr uint8_t MASK_VISITED = 0x08;
constexpr uint8_t MASK_BELT    = 0x07;

int32_t* cost_grid; // only for DISPLAY_MODE

inline uint32_t idx(uint16_t x, uint16_t y) {
    return y * (grid_width + 2) + x; // +2 for padded grid
}

inline bool is_visited(uint32_t index) {
    return grid_data[index] & MASK_VISITED;
}

inline void set_visited(uint32_t index) {
    grid_data[index] |= MASK_VISITED;
}

inline uint8_t get_belt(uint32_t index) {
    return grid_data[index] & MASK_BELT;
}

inline void set_belt(uint32_t index, uint8_t code) {
    grid_data[index] = (grid_data[index] & MASK_VISITED) | (code & MASK_BELT);
}

inline void set_cost(uint16_t x, uint16_t y, uint16_t cost) {
    if constexpr (DISPLAY_MODE) cost_grid[(y-1) * grid_height + (x-1)] = cost;
}

inline void expand_enqueue(
    uint16_t x, uint16_t y,
    vector<pair<uint16_t,uint16_t>>& next_level,
    uint16_t cost)
{
    while (true) {
        uint32_t nidx = idx(x, y);
        if (is_visited(nidx)) break;

        set_visited(nidx);
        set_cost(x, y, cost);
        next_level.emplace_back(x, y);

        uint8_t dir_code = get_belt(nidx);
        if (dir_code == 0) break;

        x += belt_dx[dir_code];
        y += belt_dy[dir_code];
    }
}

template<int dx, int dy>
inline void expand_direction(
    uint16_t x, uint16_t y,
    vector<pair<uint16_t,uint16_t>>& next_level,
    uint16_t cost)
{
    uint16_t nx = x + dx;
    uint16_t ny = y + dy;
    uint32_t nidx = idx(nx, ny);
    if (is_visited(nidx)) return;

    expand_enqueue(nx, ny, next_level, cost + 1); // Need to check if +1 is compiled away on display false
}

int main() {
    if constexpr (BENCHMARK_MODE) start = chrono::high_resolution_clock::now();

    cin >> grid_width >> grid_height;
    cin >> start_x >> start_y;
    cin >> goal_x >> goal_y;
    //cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Probably need this, but some padding offset stuff below is wrong?
    
    grid_data = new uint8_t[(grid_width + 2) * (grid_height + 2)]();   // () initializes to 0

    if constexpr (DISPLAY_MODE) {
        cost_grid = new int32_t[grid_width * grid_height];
        for (uint32_t i = 0; i < grid_width * grid_height; i++) cost_grid[i] = -1;
    }

    // Pad border: mark visited. Better than bound checks probably
    for (uint16_t x = 0; x <= grid_width + 1; x++){
        set_visited(idx(x, 0));
        set_visited(idx(x, grid_height + 1));
    }
    for(uint16_t y = 0; y <= grid_height + 1; y++){
        set_visited(idx(0, y));
        set_visited(idx(grid_width + 1, y));
    }

    // Input
    string line;
    for (uint16_t y = 0; y < grid_height; y++) {
        getline(cin, line);
        uint16_t row = grid_height - y;
        for (uint16_t x = 0; x < grid_width; x++) {
            char c = line[x];
            uint8_t code = 0;
            switch(c){
                case 'r': code = 1; break; // right
                case 'l': code = 2; break; // left
                case 'u': code = 3; break; // up
                case 'd': code = 4; break; // down
            }
            if (code >= 1 && code <= 4) {
                set_belt(idx(x + 1, row + 1), code); // +1 for padding
            }
        }
    }
    
    uint32_t start_idx = idx(start_x, start_y);
    vector<pair<uint16_t,uint16_t>> current_level = {{start_x, start_y}};
    set_visited(start_idx);
    set_cost(start_x, start_y, 0);
    uint8_t start_dir_code = get_belt(start_idx); // Hacky stuff in case it starts on a belt
    if (start_dir_code != 0) expand_enqueue(start_x + belt_dx[start_dir_code], start_y + belt_dy[start_dir_code], current_level, 0);
    vector<pair<uint16_t,uint16_t>> next_level;

    uint16_t cost = 0;

    while (true) { // Need to add a condition to fail
        next_level.clear();
        for(auto [x, y] : current_level) {
            if (x == goal_x && y == goal_y) goto end; // For some reason, this performs better than a macro to check on the frontier
            
            // Manually unrolling because compiler is stupid
            expand_direction<1, 0>(x, y, next_level, cost);  // right
            expand_direction<-1, 0>(x, y, next_level, cost); // left
            expand_direction<0, 1>(x, y, next_level, cost);  // down
            expand_direction<0, -1>(x, y, next_level, cost); // up
        }
        swap(current_level, next_level);
        cost++;
    }

end:
    cout << cost;
    
    if constexpr (DISPLAY_MODE) {
        cout << "\nCost grid:\n";
        for (uint16_t y = 0; y < grid_height; ++y) {
            uint16_t print_y = grid_height - 1 - y;
            for (uint16_t x = 0; x < grid_width; ++x) {
                int32_t c = cost_grid[print_y * grid_width + x];
                if (c == -1) cout << " . ";
                else cout << setw(2) << c << " ";
            }
            cout << "\n";
        }
    }

    if constexpr (BENCHMARK_MODE) {
        auto end = chrono::high_resolution_clock::now();
        auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << "\nElapsed time: " << elapsed_ms << " ms\n";
    }

    delete[] grid_data;
    if constexpr (DISPLAY_MODE) delete[] cost_grid;
}
