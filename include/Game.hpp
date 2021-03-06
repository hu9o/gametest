#ifndef GAME_H
#define GAME_H

#include "common.hpp"
#include "TileMap.hpp"
#include "Player.hpp"
#include <list>

class Tilemap;
class ResourceManager;

class Game
{
    public:
        Game(sf::RenderWindow& win);
        virtual ~Game();

        void run();
        void loadFromFile(const str& path);

        void registerEntity(Entity& e);

        const TileMap& getTileMap() const;
        const std::list<Player*>& getPlayers() const;

        void destroyTileAt(int x, int y);
        void spawnZombieAt(int x, int y);

    protected:
        sf::RenderWindow& m_win;

        js::Document json;
        TileMap m_tilemap;
        sf::Sprite m_bg;

        std::list<Player*> m_players;
        std::list<Entity*> m_entities;

        bool m_debugMode;
        bool m_usePostFX;

        void mainLoop();

    private:
};

#endif // GAME_H
