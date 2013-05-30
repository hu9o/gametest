#include "Tile.hpp"
#include "TileMap.hpp"
#include "ResourceManager.hpp"

using namespace std;

Tile::Tile(TileMap& map) : m_map(map)
{
    init();
}

Tile::Tile(TileMap& map, TileInfo tileInfo) : m_map(map)
{
    init();
    loadFromTileInfo(tileInfo);
}

Tile::~Tile()
{
    if (m_nextTile != NULL)
        delete m_nextTile;
}

void Tile::init()
{
    m_nextTile = NULL;
    m_layer = LAYER_BACK;

    // attention, il faut régler à zéro la lum. minimale dès le début;
    // impossible de mettre une luminosié inférieure par la suite
    m_minBrightness = rm::getKeyValueInt("m-light-min");
    m_voidLightDisp = rm::getKeyValueInt("m-light-disp-void");
    m_solidLightDisp = rm::getKeyValueInt("m-light-disp-solid");
    m_waterLightDisp = rm::getKeyValueInt("m-light-disp-water");
    m_enableLighting = rm::getKeyValueBool("m-light-enabled") && m_minBrightness < 255;
    m_sunlight = rm::getKeyValueInt("m-light-sun");

    m_brightness = 0;
    m_transparent = false; // TODO: gérer transparence
    m_shadowShape.setOutlineThickness(0);
}

void Tile::loadFromTileInfo(TileInfo tileInfo)
{
    m_posOnTileset.x = tileInfo.x;
    m_posOnTileset.y = tileInfo.y;

    m_nbFrames = tileInfo.nbframes;
    m_delay = tileInfo.delay;

    m_name = tileInfo.name;
    m_type = tileInfo.type;

    m_lightEmitted = tileInfo.light;

    updateSprite();
}

void Tile::setPosition(int x, int y)
{
    m_pos.x = x;
    m_pos.y = y;

    updateSprite();
}

void Tile::setMapInfo(sf::Texture& tex, int size)
{
    m_sprite.setTexture(tex);

    m_size = size;

    updateSprite();
}

void Tile::setLayer(Layer layer)
{
    if (m_type != TILE_WATER)
        m_layer = layer;
    else
        m_layer = LAYER_WATER;
}

void Tile::appendTile(Tile& t)
{
    if (m_nextTile == NULL)
    {
        m_nextTile = &t;
    }
    else
    {
        m_nextTile->appendTile(t);
    }
}

void Tile::drawLayer(sf::RenderTarget& win, int layer, int elapsedTime)
{
    if (m_enableLighting && layer == LAYER_SHADOW && m_nextTile == NULL)
        win.draw(m_shadowShape);

    if (m_layer == layer)
    {
        if (m_nbFrames > 1)
        {
            int currentframe = ( elapsedTime % (m_nbFrames * m_delay) ) / m_delay;

            m_sprite.setTextureRect(sf::IntRect((m_posOnTileset.x + currentframe) * m_size,
                                                m_posOnTileset.y * m_size,
                                                m_size,
                                                m_size));
        }

        win.draw(m_sprite);
    }

    if (m_nextTile != NULL)
    {
        m_nextTile->drawLayer(win, layer, elapsedTime);
    }
}

void Tile::update()
{
    updateSprite();
}

void Tile::updateSprite()
{
    m_sprite.setTextureRect(sf::IntRect(m_posOnTileset.x * m_size,
                                        m_posOnTileset.y * m_size,
                                        m_size,
                                        m_size));

    m_sprite.setPosition(m_pos.x * m_size, m_pos.y * m_size);

    m_shadowShape.setSize(vec2f(m_size, m_size));
    m_shadowShape.setPosition(m_sprite.getPosition());


    // Luminosité
    if (m_enableLighting)
    {
        int b = getLightEmitted();
        const int NB_NEIGH = 8;

        // Liste les voisins
        Tile* neighbours[NB_NEIGH] = {m_map.getTileAt(m_pos.x-1, m_pos.y),
                                m_map.getTileAt(m_pos.x, m_pos.y-1),
                                m_map.getTileAt(m_pos.x+1, m_pos.y),
                                m_map.getTileAt(m_pos.x, m_pos.y+1),
                                m_map.getTileAt(m_pos.x-1, m_pos.y-1), // 0 0
                                m_map.getTileAt(m_pos.x-1, m_pos.y+1), // 0 1
                                m_map.getTileAt(m_pos.x+1, m_pos.y-1), // 1 0
                                m_map.getTileAt(m_pos.x+1, m_pos.y+1)}; // 1 1

        // Prend la luminosité d'un voisin, qu'on diminue selon le type et la distance
        for (int i = 0; i < NB_NEIGH; i++)
        {
            if (neighbours[i] != NULL)
            {
                float dist = (i < 4)? 1.0 : 1.4;

                if (neighbours[i]->hasType(TILE_SOLID))
                {
                    if (hasType(TILE_SOLID) && neighbours[i]->getBrightness() - m_solidLightDisp*dist > b)
                        b = neighbours[i]->getBrightness() - m_solidLightDisp*dist;
                }
                else
                {
                    bool diagonaleLibre = true;

                    if (i >= 4) // pour que la lumière le passe en diagonale que si les blocs "droits" ne la bloquent pas.
                    {
                        Tile* ta = neighbours[((i-4) & 0b10)];
                        Tile* tb = neighbours[((i-4) & 0b01)*2 + 1];

                        diagonaleLibre = !(tb && tb->hasType(TILE_SOLID)) || !(tb && tb->hasType(TILE_SOLID));
                    }

                    if (diagonaleLibre)
                    {
                        if (neighbours[i]->hasType(TILE_WATER) && neighbours[i]->getBrightness() - m_waterLightDisp*dist > b)
                            b = neighbours[i]->getBrightness() - m_waterLightDisp*dist;
                        else if (neighbours[i]->getBrightness() - m_voidLightDisp*dist > b)
                            b = neighbours[i]->getBrightness() - m_voidLightDisp*dist;
                    }
                }
            }
        }

        // Applique en tenant compte de la luminosité minimale
        if (b < m_minBrightness)
            b = m_minBrightness;

        setBrightness(b);

        // On actualise les voisins s'ils sont moins éclairés
        // Ainsi on ne risque pas d'actualiser la tuile qui a demandé à actualiser celle-ci (ce serait une boucle infinie)
        int dispersion = hasType(TILE_SOLID)? m_solidLightDisp : (hasType(TILE_WATER)? m_waterLightDisp:m_voidLightDisp);

        for (int i = 0; i < NB_NEIGH; i++)
        {
            float dist = (i < 4)? 1.0 : 1.4;

            if (neighbours[i] != NULL && neighbours[i]->getBrightness() < b - dispersion*dist)
                neighbours[i]->update();
        }
    }
}

std::string Tile::getName()
{
    return m_name;
}

//TileType Tile::getType()
//{
//    return m_type;
//}

bool Tile::hasType(TileType t)
{
    if (m_nextTile == NULL)
    {
        return (m_type == t);
    }
    else
    {
        return (m_type == t) || m_nextTile->hasType(t);
    }
}

TileType Tile::getTypeByString(std::string s)
{
    static std::map<std::string, TileType> tileTypesByString;

    if (tileTypesByString.size() == 0)
    {
        tileTypesByString["void"] = TILE_VOID;
        tileTypesByString["solid"] = TILE_SOLID;
        tileTypesByString["ladder"] = TILE_LADDER;
        tileTypesByString["water"] = TILE_WATER;
        tileTypesByString["spike"] = TILE_SPIKE;
    }

    return tileTypesByString[s];
}

void Tile::setBrightness(int b)
{
    if (!m_enableLighting)
        return;

    // On va donner la luminosité à la tile la plus haute
    if (m_nextTile)
        m_nextTile->setBrightness(b);
    else
    {
        m_brightness = b;
        m_shadowShape.setFillColor(sf::Color(0, 0, 0, 255 - m_brightness));
    }
}
int Tile::getBrightness()
{
    if (!m_enableLighting)
        return 255;

    // On va prendre la luminosité de la tile la plus haute
    if (m_nextTile)
        return m_nextTile->getBrightness();
    else
        return m_brightness;
}

void Tile::setLightEmitted(int light)
{
    if (m_nextTile)
        m_nextTile->setLightEmitted(light);
    else
        m_lightEmitted = light;
}
int Tile::getLightEmitted()
{
    // directement exposé au soleil ?
    int i = m_pos.y;
    while (i>=0 && m_map.getTileAt(m_pos.x, i) && !m_map.getTileAt(m_pos.x, i)->hasType(TILE_SOLID)) i--;

    int lightEmitted = (i==-1)? max(m_lightEmitted, m_sunlight) : m_lightEmitted;

    if (m_nextTile)
        if (m_nextTile->m_transparent)
            return max(lightEmitted, m_nextTile->getLightEmitted());
        else
            return m_nextTile->getLightEmitted();
    else
        return lightEmitted;
}

vec2i Tile::getPosition()
{
    return m_pos;
}
