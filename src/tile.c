#include <stdlib.h>
#include "tile.h"

struct maptile {
    char* name;
    MapTiles mapTile;
    Image image;
};

Tile TileCreate() {
    Tile tile = malloc(sizeof(struct maptile));
    assert(tile != NULL);

    

    return tile;
}
void TileDestroy(Tile* tilep);

void TileGetName(Tile tile);
MapTiles TileGetMapTiles(Tile tile);
Image TileGetImage(Tile tile);
