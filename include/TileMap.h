#ifndef TILEMAP_H
#define TILEMAP_H

#include "common.h"
#include "Tileset.h"
#include <list>


class TileMap
{
    public:
        TileMap();
        virtual ~TileMap();

        void loadFromJson(const js::Value&);
        void drawLayer(sf::RenderWindow& win, int layer, int elapsedTime);
        void update();

        bool withinBounds(int x, int y);
        Tile* getTileAt(int x, int y);

        vec2i getStartPosition();

        bool isCollision(int x, int y, TileType t = TILE_SOLID);
        bool isCollision(sf::IntRect rect, TileType t = TILE_SOLID);
        void destroyTileAt(int x, int y);

    protected:
        Tile*** m_tilemap;
        Tileset& m_tileset;
        vec2i m_tilemapSize;
        int m_tileSize;
        std::list<Tile*> m_lightSources;

        std::vector<vec2i> m_startPositions;
        void test_displayShadow(sf::RenderWindow& win);

        void deleteTile(Tile* t);

    private:
};

#endif // TILEMAP_H
