#include <stdlib.h>
#include "tile.h"
#include <assert.h>

struct maptile {
    const char* name;
    MapTiles mapTile;
    Texture texture;
};

Tile TileCreate(const char* name, MapTiles maptile, const char* imgname) {
    Tile tile = malloc(sizeof(struct maptile));
    assert(tile != NULL);

    tile->name = name;
    tile->mapTile = maptile;
    tile->texture = LoadTexture(imgname);

    return tile;
}

void TileDestroy(Tile* tilep) {
    assert(tilep != NULL);
    assert(*tilep != NULL);

    Tile tile = *tilep;

    UnloadTexture(tile->texture);
    free(tile);

    tilep = NULL;
}

const char* TileGetName(Tile tile) {
    assert(tile != NULL);
    return tile->name;
}

MapTiles TileGetMapTiles(Tile tile) {
    assert(tile != NULL);
    return tile->mapTile;
}

Texture TileGetTexture(Tile tile) {
    assert(tile != NULL);
    return tile->texture;
}

