#ifndef TILEMAP_H
#define TILEMAP_H

#include "common.hpp"
#include "Tileset.hpp"
#include <list>


class TileMap
{
    public:
        TileMap();
        virtual ~TileMap();

        void loadFromJson(const js::Value&);
        void drawLayer(sf::RenderTarget& win, int layer, int elapsedTime);
        void update();

        bool withinBounds(int x, int y) const;
        Tile* getTileAt(vec2i v) const;
        Tile* getTileAt(int x, int y) const;

        vec2i getStartPosition();

        bool isCollision(int x, int y, TileType t = TILE_SOLID) const;
        bool isCollision(sf::IntRect rect, TileType t = TILE_SOLID) const;
        void destroyTileAt(int x, int y);

        bool tileAtHasType(int x, int y, TileType type) const;


        mutable std::vector<vec2i> test_graph;
        void test_displayGraph(sf::RenderTarget& win);

    protected:
        Tile*** m_tilemap;
        Tileset& m_tileset;
        vec2i m_tilemapSize;
        int m_tileSize;
        std::list<Tile*> m_lightSources;

        std::vector<vec2i> m_startPositions;
        //void test_displayShadow(sf::RenderTarget& win);

        void deleteTile(Tile* t);

    private:
};

#endif // TILEMAP_H
