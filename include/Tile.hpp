#ifndef TILE_H
#define TILE_H

#include "common.hpp"


enum Layer { LAYER_BACK, LAYER_FRONT, LAYER_WATER, LAYER_OVERLAY, LAYER_SHADOW };
enum TileType { TILE_VOID, TILE_SOLID, TILE_LADDER, TILE_WATER, TILE_SPIKE };

class TileMap;

struct TileInfo
{
    std::string name;
    TileType type;
    int x;
    int y;
    int nbframes; // les frames DOIVENT êtres consécutives
    int delay;
    int light;
};

class Tile
{
    public:
        //static const int WIDTH = 16;
        //static const int HEIGHT = 16;

        Tile(TileMap& map);
        Tile(TileMap& map, TileInfo tileInfo);
        virtual ~Tile();

        static TileType getTypeByString(std::string s);

        void loadFromTileInfo(TileInfo tileInfo);
        void setPosition(int x, int y);
        virtual void setMapInfo(sf::Texture& tex, int size);
        void setLayer(Layer layer);
        void appendTile(Tile& t);

        void setBrightness(int b);
        int getBrightness();

        void setLightEmitted(int light);
        int getLightEmitted();
        vec2i getPosition();

        virtual void drawLayer(sf::RenderTarget& win, int layer, int elapsedTime);
        virtual void update();

        virtual std::string getName();
        //virtual TileType getType();
        bool hasType(TileType t);

    protected:
        TileMap& m_map;
        Tile* m_nextTile;
        sf::Sprite m_sprite;
        sf::RectangleShape m_shadowShape;
        vec2i m_pos;
        vec2i m_posOnTileset;
        int m_size;
        int m_lightEmitted; // 0 à part pour la dernière
        int m_brightness; // 0 à part pour la dernière
        bool m_enableLighting;
        bool m_transparent;

        int m_minBrightness;
        int m_voidLightDisp;
        int m_solidLightDisp;
        int m_waterLightDisp;

        std::string m_name;
        TileType m_type;
        Layer m_layer;

        int m_nbFrames;
        int m_delay;

        void init();
        virtual void updateSprite();

    private:
};

#endif // TILE_H
