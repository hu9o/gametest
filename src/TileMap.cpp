#include "TileMap.hpp"
#include "ResourceManager.hpp"
#include "Zombie.hpp"
#include <math.h>

using namespace std;

TileMap::TileMap() : m_tileset(rm::getTileset())
{
    //m_tileset.loadFromFile("map/tiles.json");
}

TileMap::~TileMap()
{
    //dtor
}

void TileMap::loadFromJson(const js::Value& map)
{
    assert(map.IsArray());

    // on prend la longueur de la première ligne comme longueur de référence
    js::SizeType i = 0;
    string line = map[i].GetString();
    uint lineSize = line.size();
    uint mapWidth = lineSize/2;


    // on alloue le tableau
    m_tilemap = new Tile**[map.Size()];

    // lineSize représente le nombre de caractère dans une ligne
    // mapWidth est le nombre de tuiles (la moitié)
    m_tilemapSize.x = mapWidth;
    m_tilemapSize.y = map.Size();

    m_tileSize = m_tileset.getTileSize();

    // boucle sur tout le tableau
    for (i = 0; i < map.Size(); i++)
    {
        line = map[i].GetString();

        // toutes les lignes doivent avoir la même longueur lineSize
        assert(line.size() == lineSize);

        // on ajoute une ligne au tableau
        m_tilemap[i] = new Tile*[mapWidth];

        // boucle sur les caractères de la ligne
        for (uint j = 0; j < mapWidth; j++)
        {
            m_tilemap[i][j] = NULL;

            // traite les caractères spéciaux
            if (line[j*2+1] == '@')
            {
                m_startPositions.push_back(vec2i(j, i));
                cout << "Position de départ en " << j << ":" << i << endl;
            }
        }
    }

    // recommence et ajoute les tuiles au niveau du joueur
    for (i = 0; i < map.Size(); i++)
    {
        line = map[i].GetString();

        // boucle sur les caractères de la ligne
        for (uint j = 0; j < mapWidth; j++)
        {
            m_tilemap[i][j] = m_tileset.makeTileFromChar(*this, line[j*2]);
            assert(m_tilemap[i][j] != NULL);

            Tile* t = m_tilemap[i][j];

            t->setPosition(j, i);
            t->setMapInfo(m_tileset.getTexture(), m_tileSize);
            t->setLayer(LAYER_BACK);

            if (t->getLightEmitted() > 0)
                m_lightSources.push_back(t);

            if (line[j*2] == 'N')
                test_tombstonesPositions.push_back(vec2i(j, i));
        }
    }

    // encore une fois pour la couche du dessus
    for (i = 0; i < map.Size(); i++)
    {
        line = map[i].GetString();

        // boucle sur les caractères de la ligne
        for (uint j = 0; j < mapWidth; j++)
        {
            char c = line[j*2+1];
            //m_tileset.addDecorationFromChar(*m_tilemap[i][j], line[j*2 + 1]);
            assert(m_tilemap[i][j] != NULL);


            Tile* t = (c != '@')? m_tileset.makeTileFromChar(*this, c) : NULL;

            if (t != NULL)
            {
                t->setPosition(j, i);
                t->setMapInfo(m_tileset.getTexture(), m_tileSize);
                t->setLayer(LAYER_FRONT);

                if (t->getLightEmitted() > 0)
                    m_lightSources.push_back(t);

                m_tilemap[i][j]->appendTile(*t);
            }
        }
    }

    /*// luminosité (test)
    for (i = 0; i < map.Size(); i++)
    {
        // boucle sur les caractères de la ligne
        for (uint j = 0; j < mapWidth; j++)
        {
            int brightness = 0;

            for (
                 list<Tile*>::iterator l = m_lightSources.begin();
                                                                  l != m_lightSources.end(); l++)
            {
                vec2i pos = (*l)->getPosition();

                int dist = (pos.x - j)*(pos.x - j) + (pos.y - i)*(pos.y - i);

                int b = (*l)->getLightEmitted()*32 - dist*2;

                if (b > brightness)
                    brightness = b;
            }

            if (brightness < 1)
                brightness = 1;
            if (brightness > 256)
                brightness = 256;

                cout << "light " << 256 - brightness << endl;
            m_tilemap[i][j]->setShadow(256 - brightness);
        }
    }*/

    // update le tout
    update();

    cout << "Chargement de la carte terminé" << endl;
}

void TileMap::drawLayer(sf::RenderTarget& win, int layer, int elapsedTime)
{
    int i, j;

    // afficher toutes les tuiles
    for (i = 0; i < m_tilemapSize.y; i++)
    {
        for (j = 0; j < m_tilemapSize.x; j++)
        {
            m_tilemap[i][j]->drawLayer(win, layer, elapsedTime);
        }
    }

    if (layer == LAYER_TEST)
        test_displayGraph(win);
}


vec2i TileMap::getSize() const
{
    return m_tilemapSize;
}

Tile* TileMap::getTileAt(vec2i v) const
{
    getTileAt(v.x, v.y);
}
Tile* TileMap::getTileAt(int x, int y) const
{
    if (!withinBounds(x, y))
        return NULL;

    return m_tilemap[y][x];
}

bool TileMap::withinBounds(int x, int y) const
{
    return x >= 0 && y >= 0 && x < m_tilemapSize.x && y < m_tilemapSize.y;
}

vec2i TileMap::getStartPosition()
{
    vec2i pos;

    if (m_startPositions.size() > 0)
    {
        int n = rand() % m_startPositions.size();

        pos = m_startPositions[n];
        m_startPositions.erase(m_startPositions.begin() + n);
    }
    else
    {
        pos.x = 0, pos.y = 0;
    }

    pos.x *= m_tileSize;
    pos.y *= m_tileSize;

    return pos;
}

bool TileMap::isCollision(int x, int y, TileType t) const
{
    // warp
    if (x >= 400)
        x -= 400;
    if (x < 0)
        x += 400;

    x /= m_tileSize;
    y /= m_tileSize;

    Tile* tile = getTileAt(x, y);

    if (tile == NULL)
        return false;

    return tile->hasType(t);
}

bool TileMap::isCollision(sf::IntRect rect, TileType t) const
{
    return    isCollision(rect.left, rect.top, t)
            || isCollision(rect.left + rect.width, rect.top, t)
            || isCollision(rect.left, rect.top + rect.height, t)
            || isCollision(rect.left + rect.width, rect.top + rect.height, t);
}

/*void TileMap::test_displayShadow(sf::RenderTarget& win)
{
    return;

    sf::Sprite shadow(ResourceManager::getInstance().getTileset().getTexture());
    shadow.setTextureRect(sf::IntRect(3 * m_tileSize, 3*m_tileSize, m_tileSize, m_tileSize));

    for (float a = 0; a < 2*3.14; a += 3.14/16)
    {
        for (int d = 0; d < 6*4; d += m_tileSize/4)
        {
            //Tile* t = getTileAt(std::cos(a)*d, std::sin(a)*d);

            shadow.setPosition(((int)(cos(a)*d) % m_tileSize) + 12, ((int)(sin(a)*d) % m_tileSize) + 12);
            win.draw(shadow);
        }
    }
}*/

void TileMap::test_displayGraph(sf::RenderTarget& win)
{
    sf::Sprite mark(rm::getTileset().getTexture());
    mark.setTextureRect(sf::IntRect(102, 142, 10, 10));
    int i, j;


    for (vector<vec2i>::const_iterator it = test_graph.begin(); it != test_graph.end(); ++it)
    {
        mark.setPosition(it->x*m_tileSize + m_tileSize/2 - 5,
                         it->y*m_tileSize + m_tileSize/2 - 5);
        win.draw(mark);
    }
}

void TileMap::destroyTileAt(int x, int y)
{
    x /= m_tileSize;
    y /= m_tileSize;

    char c = '.';
    //std::cin >> c;

    if (withinBounds(x, y))
    {
        Tile*& t = m_tilemap[y][x];

        c = t->hasType(TILE_SOLID)? '.':'j';

        deleteTile(t);
        t = rm::getTileset().makeTileFromChar(*this, c);

        t->setBrightness(0);
        t->setPosition(x, y);
        t->setMapInfo(m_tileset.getTexture(), m_tileSize);
        t->setLayer(LAYER_BACK);

        Zombie::updateGraphAt(*this, x, y);
        update();
    }
}
void TileMap::deleteTile(Tile* t)
{
    list<Tile*>::iterator it = m_lightSources.begin();

    while (it != m_lightSources.end())
    {
        if (t->hasTile(*it))
        {
            it = m_lightSources.erase(it);
        }

        it++;
    }

    delete t;
}

void TileMap::update()
{
    cout << "update" << endl;

    // éteint tout
    for (int i = 0; i < m_tilemapSize.y; i++)
    {
        for (int j = 0; j < m_tilemapSize.x; j++)
        {
            m_tilemap[i][j]->setBrightness(rm::getKeyValue<int>("m-light-min"));
        }
    }

    // soleil
    for (int j = 0; j < m_tilemapSize.x; j++)
    {
        m_tilemap[0][j]->update();
    }

    // sources de lumière
    for (std::list<Tile*>::const_iterator it = m_lightSources.begin(); it != m_lightSources.end(); ++it)
    {
        (*it)->update();
    }
}

bool TileMap::tileAtHasType(int x, int y, TileType type) const
{
    Tile* t = getTileAt(x % m_tilemapSize.x, y);

    return t? t->hasType(type) : false;
}
