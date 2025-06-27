#include <stdlib.h>
#include "tile.h"
#include <assert.h>
#include <stdio.h>

struct maptile {
    const char* name;
    bool is_transparent;
    int mapTile;
    Texture texture;
};

Tile TileCreate(const char* name, int maptile, const char* imgname, bool is_transparent) {
    Tile tile = malloc(sizeof(struct maptile));
    assert(tile != NULL);

    tile->name = name;
    tile->is_transparent = is_transparent;
    tile->mapTile = maptile;
    tile->texture = LoadTexture(imgname);
    if (tile->texture.id == 0) {
        // Texture loading failed
        perror("Failed to load texture!\n");
        exit(EXIT_FAILURE);
    }

    return tile;
}

void TileDestroy(Tile* tilep) {
    assert(tilep != NULL);
    assert(*tilep != NULL);

    Tile tile = *tilep;

    UnloadTexture(tile->texture);
    free(tile);

    *tilep = NULL;
}

const char* TileGetName(Tile tile) {
    assert(tile != NULL);
    return tile->name;
}

int TileGetMapTiles(Tile tile) {
    assert(tile != NULL);
    return tile->mapTile;
}

Texture TileGetTexture(Tile tile) {
    assert(tile != NULL);
    return tile->texture;
}

bool TileIsTransparent(Tile tile) {
    assert(tile != NULL);
    return tile->is_transparent;
}
