#ifndef TILESET_H
#define TILESET_H

#include "common.hpp"
#include "AutoTile.hpp"


struct RandomTileData
{
    str name;
    int totalProbas;
    std::vector<TileInfo> tiles;
    std::vector<int> probas;
};

struct AutoTileData
{
    str name;
    str top, bot, mid, one, over, under;
};

class Tileset
{
    public:
        Tileset();
        virtual ~Tileset();

        void loadFromFile(str path);
        Tile* makeTileFromChar(TileMap& map, char c);
        void addToTileFromChar(Tile& t, char c);

        sf::Texture& getTexture();
        int getTileSize();

    protected:
        std::map<char, str> m_namesByChar;
        std::map<str, TileInfo*> m_normalTiles;
        std::map<str, RandomTileData*> m_randomTiles;
        std::map<str, AutoTileData*> m_autoTiles;

        sf::Texture* m_texture;
        int m_tileSize;


        TileInfo* getTileInfoFromName(str s);
        str getTileNameFromChar(char c);
        TileInfo& getExistingTileInfoFromName(str s);

    private:
};

#endif // TILESET_H
