#include "Entity.hpp"
#include "Game.hpp"

using namespace std;

Entity::Entity(Game& game) : m_game(game)
{
    game.registerEntity(*this);
}

Entity::~Entity()
{
    //dtor
}

void Entity::update(float frameTime)
{
    if (m_pos.x >= 400)
        m_pos.x -= 400;

    if (m_pos.x < 0)
        m_pos.x += 400;
}

void Entity::setPosition(vec2i pos)
{
    m_pos.x = pos.x;
    m_pos.y = pos.y;
    m_sprite.setPosition(m_pos);
}

void Entity::setPosition(vec2f pos)
{
    m_pos = pos;
    //m_sprite.setPosition(m_pos);
}

void Entity::drawSprite(sf::RenderTarget& win)
{
    // Dessine le sprite
    win.draw(m_sprite);

    // Le dessine de l'autre côté de la fenêtre
    vec2f lastPos = m_sprite.getPosition();

    if (lastPos.x > 400/2)
        m_sprite.setPosition(lastPos.x - 400, lastPos.y);
    else
        m_sprite.setPosition(lastPos.x + 400, lastPos.y);

    win.draw(m_sprite);

    // Et le remet à sa place!
    m_sprite.setPosition(lastPos);
}
