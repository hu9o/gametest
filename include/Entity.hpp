#ifndef ENTITY_H
#define ENTITY_H

#include "common.hpp"

class Game;

class Entity
{
    public:
        Entity(Game& game);
        virtual ~Entity();

        virtual void draw(sf::RenderTarget& win, int elapsedTime) = 0;
        virtual void update(float frameTime);

        void setPosition(vec2i pos);
        void setPosition(vec2f pos);

    protected:
        Game& m_game;
        sf::Sprite m_sprite;
        vec2f m_pos;

        void drawSprite(sf::RenderTarget& win);

    private:
};

#endif // ENTITY_H
