#include "Entity.hpp"
#include "Game.hpp"
#include "ResourceManager.hpp"

using namespace std;

Entity::Entity(Game& game) : m_game(game), m_bubbleSprite(rm::getTexture(rm::getKeyValue<str>("d-interface")+"bubbles.png"))
{
    game.registerEntity(*this);
}

Entity::~Entity()
{
    //dtor
}

void Entity::update(float frameTime)
{
    int mapWidth = m_game.getTileMap().fromTileCoords(m_game.getTileMap().getSize()).x;

    if (m_pos.x >= mapWidth)
        m_pos.x -= mapWidth;

    if (m_pos.x < 0)
        m_pos.x += mapWidth;

    if (m_bubbleTime >= 0)
        m_bubbleTime -= frameTime;
}

void Entity::draw(sf::RenderTarget& win, int elapsedTime)
{
    if (m_bubbleTime >= 0)
    {
        m_bubbleSprite.setPosition(m_pos.x-8, m_pos.y-16 - 16);
        win.draw(m_bubbleSprite);
    }
}

void Entity::displayBubble(int index)
{
    m_bubbleSprite.setTextureRect(sf::IntRect(index*16, 0, 16, 16));
    m_bubbleTime = 20.f;
}

vec2f Entity::getPosition() const
{
    return m_pos;
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
    int mapWidth = m_game.getTileMap().fromTileCoords(m_game.getTileMap().getSize()).x;

    if (lastPos.x > mapWidth/2)
        m_sprite.setPosition(lastPos.x - mapWidth, lastPos.y);
    else
        m_sprite.setPosition(lastPos.x + mapWidth, lastPos.y);

    win.draw(m_sprite);

    // Et le remet à sa place!
    m_sprite.setPosition(lastPos);
}
