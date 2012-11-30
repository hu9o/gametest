#include "AutoTile.h"
#include "TileMap.h"

AutoTile::AutoTile(TileMap& map) : Tile(map)
{
}

AutoTile::AutoTile(TileMap& map, AutoTileInfo tileInfo) : Tile(map)
{
    loadFromAutoTileInfo(tileInfo);
}

AutoTile::~AutoTile()
{
    //dtor
}

void AutoTile::loadFromAutoTileInfo(AutoTileInfo tileInfo)
{
    m_autoTileInfo = tileInfo;

    updateSprite();
}

void AutoTile::setMapInfo(sf::Texture& tex, int size)
{
    m_overSprite.setTexture(tex);
    m_underSprite.setTexture(tex);

    Tile::setMapInfo(tex, size);
}

void AutoTile::updateSprite()
{
    //TODO: Màj m_posOnTileset en fonction des tuiles environnantes
    bool top = m_map.getTileAt(m_pos.x, m_pos.y-1) && m_map.getTileAt(m_pos.x, m_pos.y-1)->getName() == getName();
    bool bot = m_map.getTileAt(m_pos.x, m_pos.y+1) && m_map.getTileAt(m_pos.x, m_pos.y+1)->getName() == getName();

    m_currentTileInfo = top? (bot? &m_autoTileInfo.mid:&m_autoTileInfo.bot):(bot? &m_autoTileInfo.top:&m_autoTileInfo.one);

    m_type = m_currentTileInfo->type;
    m_posOnTileset.x = m_currentTileInfo->x;
    m_posOnTileset.y = m_currentTileInfo->y;
    m_nbFrames = m_currentTileInfo->nbframes;
    m_delay = m_currentTileInfo->delay;

    // màj le overSprite
    m_overSprite.setTextureRect(sf::IntRect(m_autoTileInfo.over.x * m_size,
                                        m_autoTileInfo.over.y * m_size,
                                        m_size,
                                        m_size));
    m_underSprite.setTextureRect(sf::IntRect(m_autoTileInfo.under.x * m_size,
                                        m_autoTileInfo.under.y * m_size,
                                        m_size,
                                        m_size));

    m_overSprite.setPosition(m_pos.x * m_size, (m_pos.y-1) * m_size);
    m_underSprite.setPosition(m_pos.x * m_size, (m_pos.y+1) * m_size);


    Tile::updateSprite();
}

void AutoTile::drawLayer(sf::RenderTarget& win, int layer, int elapsedTime)
{
    Tile::drawLayer(win, layer, elapsedTime);

    if (layer == LAYER_OVERLAY)
    {
        if (m_currentTileInfo == &m_autoTileInfo.top || m_currentTileInfo == &m_autoTileInfo.one)
        {
            win.draw(m_overSprite);
        }

        if (m_currentTileInfo == &m_autoTileInfo.bot || m_currentTileInfo == &m_autoTileInfo.one)
        {
            win.draw(m_underSprite);
        }
    }
}

std::string AutoTile::getName()
{
    return m_autoTileInfo.name;
}

TileType AutoTile::getType()
{
    return m_currentTileInfo->type;
}
