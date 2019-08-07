# OPENGL-CAR

A 3d car game by opengl.

## Usage

```bash
. compile.sh
./my3dcar.cpp
```

By default, the map in `map.txt` will be loaded, you can also specify the map you want using `-m` option.

e.g.

```bash
./ my3dcar.cpp -m line_map.txt
```

## Features

To win the game, you need to pass through the vertical red wall, the starting line, to start timing. Then stop into the parking lot which has a red ground(if exists). And finally you need to pass through the vertical red wall, the finishing line, to stop timing.

In map files, they can record your personal best of finishing that map, try your best to finish the map as fast as possible!
