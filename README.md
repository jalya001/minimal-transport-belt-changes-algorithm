# minimal-transport-belt-changes-algorithm
A C++ 17 algorithm that finds the minimum of changes in a grid needed for a path of transport belts from a point to another.

## Quirks
- Single-threaded.
- Hardware and platform independent.
- Well optimized for the above.
- Processes ~100 million tiles per second on a decent computer.
- No inbuilt malformed input check.
- It does not exit on failure. (There is no possibility for it to fail on a meaningful search, but maybe I will add a toggleable safe mode later.)
- If you need more speed and memory is not a concern, change the queues into a buffer with size of the grid.

## Features
- A mode to display the costs of the grid the algorithm found.
- A mode to see how long the script took.

## How to use
See example.txt for an example of input.

File "header" format:<br/>
<grid_width> <grid_height><br/>
<start_x> <start_y><br/>
<goal_x> <goal_y>

NB: The grid begins at index 1, not 0.

## Algorithm
- Conceptually, it is a BFS that switches to linearly traversing a belt when it encounters one.
- Never revisits a tile, making its runtime asymptotically dominated by width * height in the worst case.
- Average memory use growth of 1 byte per grid tile. Running out of memory is a concern on very big grids. (Could have used a bitmap for visited and a hashmap for belts, for many times less use, but that is less cache efficient and has a higher lookup overhead.)
- Worst case memory use is something like 5 bytes * size of grid * 2/3, but this only happens if your grid is designed a very specific way.

