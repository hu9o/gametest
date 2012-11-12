#ifndef TILESET_H
#define TILESET_H

#include "common.h"
#include "AutoTile.h"


struct RandomTileData
{
    std::string name;
    int totalProbas;
    std::vector<TileInfo> tiles;
    std::vector<int> probas;
};

struct AutoTileData
{
    std::string name;
    std::string top, bot, mid, one, over, under;
};

class Tileset
{
    public:
        Tileset();
        virtual ~Tileset();

        void loadFromFile(std::string path);
        Tile* makeTileFromChar(TileMap& map, char c);
        void addToTileFromChar(Tile& t, char c);

        sf::Texture& getTexture();
        int getTileSize();

    protected:
        std::map<char, std::string> m_namesByChar;
        std::map<std::string, TileInfo*> m_normalTiles;
        std::map<std::string, RandomTileData*> m_randomTiles;
        std::map<std::string, AutoTileData*> m_autoTiles;

        sf::Texture* m_texture;
        int m_tileSize;


        TileInfo* getTileInfoFromName(std::string s);
        std::string getTileNameFromChar(char c);
        TileInfo& getExistingTileInfoFromName(std::string s);

    private:
};

#endif // TILESET_H
