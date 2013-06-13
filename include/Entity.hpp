#ifndef ENTITY_H
#define ENTITY_H

#include "common.hpp"

class Game;

class Entity
{
    public:
        Entity(Game& game);
        virtual ~Entity();

        virtual void draw(sf::RenderTarget& win, int elapsedTime);
        virtual void update(float frameTime);

        vec2f getPosition() const;
        void setPosition(vec2i pos);
        void setPosition(vec2f pos);

        void displayBubble(int index=0);

    protected:
        Game& m_game;
        sf::Sprite m_sprite;
        vec2f m_pos;

        float m_bubbleTime;
        sf::Sprite m_bubbleSprite;

        void drawSprite(sf::RenderTarget& win);

    private:
};

#endif // ENTITY_H
