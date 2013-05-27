#ifndef AUTOTILE_H
#define AUTOTILE_H

#include "Tile.hpp"


struct AutoTileInfo
{
    std::string name;
    TileType type;
    TileInfo top, bot, mid, one, over, under;
};


class AutoTile : public Tile
{
    public:
        AutoTile(TileMap& map);
        AutoTile(TileMap& map, AutoTileInfo tileInfo);
        virtual ~AutoTile();

        void loadFromAutoTileInfo(AutoTileInfo tileInfo);
        virtual void setMapInfo(sf::Texture& tex, int size);

        virtual void drawLayer(sf::RenderTarget& win, int layer, int elapsedTime);

        virtual std::string getName();
        virtual TileType getType();

    protected:
        AutoTileInfo m_autoTileInfo;
        TileInfo* m_currentTileInfo;

        sf::Sprite m_overSprite;
        sf::Sprite m_underSprite;

        virtual void updateSprite();

    private:
};

#endif // AUTOTILE_H
