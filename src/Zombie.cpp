#include "Zombie.hpp"
#include "Game.hpp"
#include "ResourceManager.hpp"


std::map<int, Zombie::GraphNode> Zombie::s_graph;
bool Zombie::s_useGraph = false;

Zombie::Zombie(Game& game) : Mob(game), m_target(NULL), m_wandering(false)
{
    s_useGraph = rm::getKeyValue<bool>("g-zombie-graph");
    setStatus(STAT_UNDEAD, true);

    m_reloadTime = 0.f;
}

Zombie::~Zombie()
{
    //dtor
}

void Zombie::goTo(vec2i targetPos)
{
    // Beaucoup de copies de vector...
    std::vector<vec2i> newPath = findPath(m_game.getTileMap(), m_game.getTileMap().toTileCoords(m_pos.x, m_pos.y-16), targetPos, PATH_FIND);

    if (!newPath.empty())
    {
        if (m_wandering)
            displayBubble(); // on a rep�r� le joueur !

        m_path = newPath;
        m_wandering = false;
    }
}

void Zombie::wander()
{
    vec2i tileCoords = m_game.getTileMap().toTileCoords(m_pos.x, m_pos.y-16);
    int mapWidth = m_game.getTileMap().getSize().x;

    if (s_useGraph)
    {
        int nextPos = tileCoords.x + tileCoords.y*mapWidth;
        int dist = (rand()%4==0)? 6:0;

        do
        {
            std::map<int, GraphNode>::iterator it = s_graph.find(nextPos);
            if (it == s_graph.end()) break;

            const std::vector<int>& neighsPos = it->second.neighboursPos;
            if (neighsPos.empty()) break;

            nextPos = neighsPos[rand() % neighsPos.size()];
            m_path.push_back(vec2i(nextPos%mapWidth, nextPos/mapWidth));
        }
        while (dist--);

        if (m_path.size() == 1)
            m_path.clear();
    }
    else
    {
        vec2i pos(rand() % m_game.getTileMap().getSize().x,
                  rand() % m_game.getTileMap().getSize().y);

        if (!m_game.getTileMap().tileAtHasType(pos.x, pos.y, TILE_WATER))
        {
            if (rand()%4==0)
                m_path = findPath(m_game.getTileMap(), tileCoords, pos, PATH_WANDER);
        }
    }

    m_wandering = true;
}

void Zombie::seekAir()
{
    vec2i pos(rand() % m_game.getTileMap().getSize().x,
              rand() % m_game.getTileMap().getSize().y);

    m_path = findPath(m_game.getTileMap(), m_game.getTileMap().toTileCoords(m_pos.x, m_pos.y-16), pos, PATH_SEEK_AIR);
    m_wandering = false;
}

bool Zombie::isIdle() const
{
    return m_path.empty();
}

void Zombie::setTarget(const Entity& e)
{
    m_target = &e;
}


void Zombie::update(float frameTime)
{
    if (m_state != ST_DEAD)
    {
        const TileMap& map = m_game.getTileMap();
        bool actLeft = false;
        bool actRight = false;
        bool actJump = false;
        bool actUp = false;
        bool actDown = false;

        if (m_reloadTime <= 0)
        {
            for (std::list<Mob*>::iterator it = s_mobs.begin(); it != s_mobs.end(); ++it)
            {
                if (isCollision(**it) && !(*it)->hasStatus(STAT_UNDEAD))
                {
                    (*it)->damage(10);
                    (*it)->poison(4000);

                    m_reloadTime = 16.f;
                }
            }
        }
        else
        {
            m_reloadTime -= frameTime;
        }

        if (changePathClock.getElapsedTime().asMilliseconds() >= 1000)
        {
            // Si on manque d'oxyg�ne ou si on ne poursuit personne, on remonte.
            if (m_oxygen < 3 || (hasStatus(STAT_APNEA) && m_wandering))
            {
                seekAir();
            }
            else if (m_target)
            {

                goTo(map.toTileCoords(m_target->getPosition().x, m_target->getPosition().y-16));

                if (isIdle()) // Cible hors de port�e, on erre
                    wander();
            }
            changePathClock.restart();
        }

        if (!m_path.empty())
        {
            vec2i currPos(map.toTileCoords(m_pos.x, m_pos.y-16));
            vec2i nextPos(m_path.back());

            if (currPos.x > map.getSize().x-2 && nextPos.x < 2) // warp
                nextPos.x += map.getSize().x;
            if (currPos.x < 2 && nextPos.x > map.getSize().x-2) // warp
                nextPos.x -= map.getSize().x;

            vec2i nextPosAbs(map.fromTileCoords(.5+nextPos.x, 1.+nextPos.y));

            // saute si sur une �chelle et but hors de l'�chelle
            if (false && map.tileAtHasType(currPos.x, currPos.y, TILE_LADDER) && !map.tileAtHasType(nextPos.x, nextPos.y, TILE_LADDER))
            {
                actJump = true;
            }
            // saute si s'accroche
            if (m_state == ST_HANG)
                actJump = true;

            if (nextPos == currPos)
            {
                m_path.pop_back();

                if (!map.tileAtHasType(nextPos.x, nextPos.y+1, TILE_SOLID))
                {
                    if (map.tileAtHasType(nextPos.x-1, nextPos.y, TILE_SOLID) && !map.tileAtHasType(nextPos.x-1, nextPos.y-1, TILE_SOLID))
                        actLeft = true;
                    if (map.tileAtHasType(nextPos.x+1, nextPos.y, TILE_SOLID) && !map.tileAtHasType(nextPos.x+1, nextPos.y-1, TILE_SOLID))
                        actRight = true;
                }
            }
            else
            {
                if (true)
                {
                    if (nextPosAbs.x < m_pos.x)// && map.tileAtHasType(nextPos.x, nextPos.y+1, TILE_SOLID))
                    {
                        actLeft = true;
                    }
                    if (nextPosAbs.x > m_pos.x)// && map.tileAtHasType(nextPos.x, nextPos.y+1, TILE_SOLID))
                    {
                        actRight = true;
                    }
                }

                if (map.tileAtHasType(currPos.x, currPos.y, TILE_LADDER) || m_state == ST_CLIMB)
                {
                    if (nextPosAbs.y+1 < m_pos.y)
                        actUp = true;
                    else if (nextPosAbs.y+1 > m_pos.y)
                        actDown = true;
                }
                else if (nextPos.y == currPos.y-1)
                {
                    actJump = true;
                }
                else if (nextPos.y == currPos.y+1)
                    actDown = true;
            }

            if (false && m_pos.x < nextPosAbs.x+4 && m_pos.x > nextPosAbs.x-4)
            {
                if (m_pos.x < nextPosAbs.x)
                    m_pos.x += 2;
                if (m_pos.x > nextPosAbs.x)
                    m_pos.x -= 2;
            }

            // emp�che de grimper sous l'eau
            if (map.tileAtHasType(currPos.x, currPos.y, TILE_WATER))
                actUp = actDown = false;
        }

        pressAction(ACT_LEFT, actLeft);
        pressAction(ACT_RIGHT, actRight);
        pressAction(ACT_JUMP, actJump);
        pressAction(ACT_UP, actUp);
        pressAction(ACT_DOWN, actDown);
    }

    Mob::update(frameTime);
}

std::vector<vec2i> Zombie::findPath(const TileMap& map, vec2i sourcePos, vec2i targetPos, FindPathAction action)
{
    /**
      * On s'arr�te si la liste ferm�e est vide (pas de chemin)
      * ou si l'arriv�e est dans la liste ferm�e
      * TODO: Surveiller la m�moire!!
      *
      * std::cout << "pathfinding start" << std::endl;
      *
      * Am�lioration: utiliser une liste tri�e pour avoir le plus petit F en 1er?
      * ou garder trace du plus petit F pour ne pas avoir � boucler
      *
      * EDIT : si seekingAir, on n'atteint pas le but, juste une case d'air.
      * EDIT : sinon, si 'wandering>0', on n'atteint pas le but non plus, on parcourt seulement 'wandering' cases.
      */

    int maxPathLength = rm::getKeyValue<int>("g-zombie-max-path-length");
    vec2i mapSize = map.getSize();

    Node target(NULL, targetPos.x, targetPos.y, map.getTileAt(targetPos));
    Node source(NULL, sourcePos.x, sourcePos.y, map.getTileAt(targetPos));

    source.target = &target;
    target.target = &target;

    /*
     * std::cout << "target: ("
     * << target.x << ", " << target.y
     * << ')' << std::endl;
     */

    std::list<Node*> open, closed;
    //Node* openLowest = NULL;

    open.push_back(&source);
    //openLowest = &source;
    source.g = 0;

    while (!open.empty())
    {
        std::list<Node*>::iterator it;
        std::list<Node*>::iterator currentIt;
        Node* current = NULL;

        /// On prend l'�l�ment qui a le plus petit F

        it = open.begin();
        currentIt = it;
        current = *currentIt;
        int lowerF = current->getF();

        for (++it ; it != open.end(); ++it)
        {
            if ((*it)->getF() < lowerF)
            {
                currentIt = it;
                current = *currentIt;
                lowerF = current->getF();
            }
        }

        if (sourcePos==targetPos || (action == PATH_WANDER && current->distance >= 4))
        {
            target = *current;
            break;
        }

        /// d�place 'current' dans la liste ferm�e

        open.erase(currentIt);
        closed.push_back(current);

        /// Recherche des voisins

        std::vector<Node*> neighbors;

        int currentPosInGraph = current->x + current->y*map.getSize().x;
        std::map<int, GraphNode>::const_iterator graphNodeIt = s_graph.find(currentPosInGraph);
        bool nodeIsInGraph = (s_useGraph && graphNodeIt != s_graph.end());
        std::vector<int> currentsNeighsInGraph;

        for (int i=-1; i<=1; i++)
        {
            for (int j=-1; j<=1; j++)
            {
                /**
              * Ajoute le noeud aux voisins si ce n'est pas le noeud courant
              * et si le noeud est traversable
              * et s'il n'est pas dans la liste ferm�e
              */

                bool notInClosed = true;
                int currentxi = (current->x+i + mapSize.x)%mapSize.x;

                Tile* currentCase = map.getTileAt(currentxi, current->y+j);

                if (!currentCase)
                    continue;

                for (it = closed.begin(); it!=closed.end() && notInClosed; ++it)
                {
                    if ((*it)->x == currentxi && (*it)->y == current->y+j)
                        notInClosed = false;
                }


                bool ok = false;
                //bool thisNodeIsInGraph = s_graph.find((currentxi) + (current->y+j) * map.getSize().x) != s_graph.end();

                // si d�j� dans le graphe, et pas encore parcouru
                if ((i || j) && nodeIsInGraph && action == PATH_FIND)
                {
                    if (notInClosed)
                    {
                        const std::vector<int>& neighsPos = graphNodeIt->second.neighboursPos;

                        for (std::vector<int>::const_iterator neighIt=neighsPos.begin(); neighIt != neighsPos.end(); ++neighIt)
                        {
                            if (*neighIt == currentxi + (current->y+j) * map.getSize().x)
                            {
                                ok = true;
                            }
                        }
                    }
                }
                // condition pour ajouter au graphe, pas aux voisins !
                else if ((i || j) && !currentCase->hasType(TILE_SOLID) && !currentCase->hasType(TILE_SPIKE))
                {
                    //// si c'est la cible, on fonce !
                    //if ((currentxi == target.x && current->y+j == target.y)
                    //    || (action == PATH_SEEK_AIR && !currentCase->hasType(TILE_WATER)))
                    //{
                    //    ok = true;
                    //}
                    //// nage
                    //else
                    if (map.tileAtHasType(current->x, current->y, TILE_WATER))
                    {
                        // pas errer dans l'eau, �a tue.
                        if (action != PATH_WANDER)
                            if ((!i || !j) || (!map.tileAtHasType(current->x, current->y+j, TILE_SOLID)
                                                       && !map.tileAtHasType(currentxi, current->y, TILE_SOLID)))
                                ok = true;
                    }
                    // tombe
                    else if (!map.tileAtHasType(current->x, current->y+1, TILE_SOLID)
                     && !map.tileAtHasType(current->x-1, current->y, TILE_SOLID)
                     && !map.tileAtHasType(current->x+1, current->y, TILE_SOLID)

                     && !map.tileAtHasType(current->x, current->y+1, TILE_LADDER))
                    {
                        if (i==0 && j==1)
                            ok = true;
                    }
                    else
                    {
                        // saute � gauche ou � droite
                        if (j==-1 && i!=0 && !map.tileAtHasType(current->x, current->y, TILE_LADDER)
                                        && !map.tileAtHasType(current->x, current->y-1, TILE_SOLID)
                                        && !map.tileAtHasType(currentxi, current->y+j, TILE_SOLID)
                                        && map.tileAtHasType(currentxi, current->y, TILE_SOLID))
                            ok = true;

                        // gauche ou droite
                        if (j==0 && i!=0 && !map.tileAtHasType(currentxi, current->y, TILE_SOLID) &&
                                            (map.tileAtHasType(current->x, current->y+1, TILE_SOLID)
                                          || (map.tileAtHasType(current->x, current->y+1, TILE_LADDER)
                                           && !map.tileAtHasType(current->x, current->y, TILE_SOLID)
                                           && !map.tileAtHasType(current->x, current->y, TILE_LADDER)
                                           && !map.tileAtHasType(current->x, current->y, TILE_LADDER))) )
                            ok = true;

                        // �chelle
                        if (i==0 && j!=0 && (currentCase->hasType(TILE_LADDER) || map.tileAtHasType(current->x, current->y-j, TILE_LADDER)))
                            ok = true;

                        // saut simple
                        if (i==0 && j==-1 && !map.tileAtHasType(current->x, current->y-1, TILE_SOLID)
                                          &&   (map.tileAtHasType(current->x, current->y+1, TILE_SOLID)
                                            ||  map.tileAtHasType(current->x, current->y, TILE_WATER)))
                            ok = true;

                        // tombe
                        if (i==0 && j==1 && !map.tileAtHasType(current->x, current->y+1, TILE_SOLID))
                            ok = true;

                    }
                }

                if (ok)
                {
                    if (s_useGraph)
                        currentsNeighsInGraph.push_back((currentxi) + (current->y+j) * map.getSize().x);

                    // pas encore parcouru, pas trop loin, on l'ajoute
                    if (notInClosed && (!maxPathLength || current->distance < maxPathLength))
                    {
                        Node* c = new Node(current, currentxi, current->y+j, currentCase);

                        c->g = current->g + ((i && j)? 14:10); //*currentCase->getCost();

                        neighbors.push_back(c);
                    }
                }
            }
        }

        if (s_useGraph && !nodeIsInGraph && action == PATH_FIND)
        {
            s_graph[currentPosInGraph] = GraphNode{currentPosInGraph, currentsNeighsInGraph};
        }


        /// Boucle sur les voisins...

        for (std::vector<Node*>::iterator jt = neighbors.begin(); jt != neighbors.end(); ++jt)
        {
            Node* neigh = *jt;
            bool found = false;

            std::list<Node*>::iterator open_n;
            for (open_n = open.begin(); open_n != open.end(); ++open_n)
            {
                if ((*open_n)->x == neigh->x &&
                    (*open_n)->y == neigh->y)
                {
                    found = true;
                    break;
                }
            }

            /// Si le voisin est dans la liste ouverte, plus court chemin?
            /// sinon, on l'ajoute simplement

            if (found)
            {
                if ((*open_n)->g > neigh->g)
                {
                    (*open_n)->g = neigh->g;
                    (*open_n)->parent = current;
                }
                delete neigh;
            }
            else
            {
                open.push_back(neigh);
            }
        }

        if ((current->x == target.x && current->y == target.y)
            || (action == PATH_SEEK_AIR && !map.tileAtHasType(current->x, current->y, TILE_WATER) && !map.tileAtHasType(current->x, current->y, TILE_SOLID)))
        {
            target = *current;
            break;
        }
    }

    /// On remonte par parents depuis target

    std::vector<vec2i> res;
    Node* node = &target;
    bool foundPath;
    while (node != NULL)
    {
        res.push_back(vec2i(node->x, node->y));

        /// Si le noeud parent est nul, on a soit target soit source.
        /// si c'est source, le chemin a �t� trouv�

        if (node->parent == NULL)
            foundPath = (node == &source);

        node = node->parent;
    }

    /// Lib�rer la m�moire
    for (std::list<Node*>::iterator it = closed.begin(); it != closed.end(); ++it)
        delete *it;
    for (std::list<Node*>::iterator it = open.begin(); it != open.end(); ++it)
        delete *it;


    /// Retire le premier noeud.
    if (foundPath)
        res.pop_back();

    /// Si un chemin a �t� trouv�, le renvoie. Sinon renvoie une liste vide.
    //map.test_graph = foundPath? res : std::vector<vec2i>();
    return foundPath? res : std::vector<vec2i>();
}

void Zombie::updateGraphAt(const TileMap& map, int tileX, int tileY)
{
    // Peut �tre une fa�on moins radicale ?
    s_graph.clear();
}

