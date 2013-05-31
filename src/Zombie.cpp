#include "Zombie.hpp"
#include "Game.hpp"

Zombie::Zombie(Game& game) : Mob(game), m_target(NULL), m_wandering(false)
{
    //ctor
}

Zombie::~Zombie()
{
    //dtor
}

void Zombie::goTo(vec2i targetPos)
{
    m_path = findPath(m_game.getTileMap().toTileCoords(m_pos.x, m_pos.y-16), targetPos);
    m_wandering = false;
}

void Zombie::wander()
{
    vec2i pos(rand() % m_game.getTileMap().getSize().x,
              rand() % m_game.getTileMap().getSize().y);

    if (!m_game.getTileMap().tileAtHasType(pos.x, pos.y, TILE_WATER))
    {
        if (rand()%4==0)
            m_path = findPath(m_game.getTileMap().toTileCoords(m_pos.x, m_pos.y-16), pos, 4);

        m_wandering = true;
    }
}

void Zombie::seekAir()
{
    vec2i pos(rand() % m_game.getTileMap().getSize().x,
              rand() % m_game.getTileMap().getSize().y);

    m_path = findPath(m_game.getTileMap().toTileCoords(m_pos.x, m_pos.y-16), pos, 0, true);
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

        if (changePathClock.getElapsedTime().asMilliseconds() >= 1000)
        {
            // Si on manque d'oxygène ou si on ne poursuit personne, on remonte.
            if (m_oxygen < 3 || (!m_canBreathe && m_wandering))
            {
                seekAir();
            }
            else if (m_target)
            {
                goTo(map.toTileCoords(m_target->getPosition().x, m_target->getPosition().y-16));

                if (isIdle()) // Cible hors de portée, on erre
                    wander();
            }
            changePathClock.restart();
        }

        if (!m_path.empty())
        {
            vec2i currPos(map.toTileCoords(m_pos.x, m_pos.y-16));
            vec2i nextPos(m_path.back());
            vec2i nextPosAbs(map.fromTileCoords(.5+nextPos.x, 1.+nextPos.y));

            // saute si sur une échelle et but hors de l'échelle
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
                    if (nextPosAbs.y < m_pos.y)
                        actUp = true;
                    else if (nextPosAbs.y > m_pos.y)
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

            // empêche de grimper sous l'eau
            if (map.tileAtHasType(currPos.x, currPos.y, TILE_WATER))
                actUp = actDown = false;
        }

        pressAction(ACT_LEFT, actLeft);
        pressAction(ACT_RIGHT, actRight);
        pressAction(ACT_JUMP, actJump);
        pressAction(ACT_UP, actUp);
        pressAction(ACT_DOWN, actDown);
    }

    // update, en réduisant la vitesse si wander
    Mob::update(frameTime);
}

std::vector<vec2i> Zombie::findPath(vec2i sourcePos, vec2i targetPos, int wander, bool seekAir) const
{
    /**
      * On s'arrête si la liste fermée est vide (pas de chemin)
      * ou si l'arrivée est dans la liste fermée
      * TODO: Surveiller la mémoire!!
      *
      * std::cout << "pathfinding start" << std::endl;
      *
      * Amélioration: utiliser une liste triée pour avoir le plus petit F en 1er?
      * ou garder trace du plus petit F pour ne pas avoir à boucler
      *
      * EDIT : si seekAir, on n'atteint pas le but, juste une case d'air.
      * EDIT : sinon, si wander>0, on n'atteint pas le but non plus, on parcourt seulement wander cases.
      */

    const TileMap& map = m_game.getTileMap();
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

        /// On prend l'élément qui a le plus petit F

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

        if (!seekAir && wander && current->distance >= wander)
        {
            target = *current;
            break;
        }

        /// déplace 'current' dans la liste fermée

        open.erase(currentIt);
        closed.push_back(current);

        /// Recherche des voisins

        std::vector<Node*> neighbors;
        for (int i=-1; i<=1; i++)
        {
            for (int j=-1; j<=1; j++)
            {
                /**
              * Ajoute le noeud aux voisins si ce n'est pas le noeud courant
              * et si le noeud est traversable
              * et s'il n'est pas dans la liste fermée
              */

                bool notInClosed = true;
                Tile* currentCase = map.getTileAt(current->x+i, current->y+j);

                if (!currentCase)
                    continue;

                for (it = closed.begin(); it!=closed.end() && notInClosed; ++it)
                {
                    if ((*it)->x == current->x+i && (*it)->y == current->y+j)
                        notInClosed = false;
                }

                /*
                if ( (current->x+i == target.x && current->y+j == target.y && skipLast) ||
                       ((i || j) && notInClosed
                        && isWalkable(current->x+i, current->y+j)
                        && ( (!i || !j) || (isWalkable(current->x, current->y+j)
                                            && isWalkable(current->x+i, current->y))
                           )
                       )
                    )
                */

                bool ok = false;

                if ((i || j) && notInClosed && !currentCase->hasType(TILE_SOLID) && !currentCase->hasType(TILE_SPIKE))
                {
                    // si c'est la cible, on fonce !
                    if ((current->x+i == target.x && current->y+j == target.y)
                        || (seekAir && !currentCase->hasType(TILE_WATER)))
                    {
                        ok = true;
                    }
                    // nage
                    else if (currentCase->hasType(TILE_WATER))
                    {
                        // pas errer dans l'eau, ça tue.
                        if (seekAir || !wander)
                            if ((!i || !j) || (!map.tileAtHasType(current->x, current->y+j, TILE_SOLID)
                                                       && !map.tileAtHasType(current->x+i, current->y, TILE_SOLID)))
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
                        // saute à gauche ou à droite
                        if (j==-1 && i!=0 && !map.tileAtHasType(current->x, current->y, TILE_LADDER)
                                        && !map.tileAtHasType(current->x, current->y-1, TILE_SOLID)
                                        && !map.tileAtHasType(current->x+i, current->y+j, TILE_SOLID)
                                        && map.tileAtHasType(current->x+i, current->y, TILE_SOLID))
                            ok = true;

                        // gauche ou droite
                        if (j==0 && i!=0 && !map.tileAtHasType(current->x+i, current->y, TILE_SOLID) &&
                                            (map.tileAtHasType(current->x, current->y+1, TILE_SOLID)
                                          || (map.tileAtHasType(current->x, current->y+1, TILE_LADDER)
                                           && !map.tileAtHasType(current->x, current->y, TILE_SOLID)
                                           && !map.tileAtHasType(current->x, current->y, TILE_LADDER)
                                           && !map.tileAtHasType(current->x, current->y, TILE_LADDER))) )
                            ok = true;

                        // échelle
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
                    Node* c = new Node(current, current->x+i, current->y+j, currentCase);

                    c->g = current->g + ((i && j)? 14:10); //*currentCase->getCost();

                    neighbors.push_back(c);
                }
            }
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
            || (seekAir && !map.tileAtHasType(current->x, current->y, TILE_WATER) && !map.tileAtHasType(current->x, current->y, TILE_SOLID)))
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
        /// si c'est source, le chemin a été trouvé

        if (node->parent == NULL)
            foundPath = (node == &source);

        node = node->parent;
    }

    /// Libérer la mémoire
    for (std::list<Node*>::iterator it = closed.begin(); it != closed.end(); ++it)
        delete *it;
    for (std::list<Node*>::iterator it = open.begin(); it != open.end(); ++it)
        delete *it;


    /// Retire le premier noeud.
    if (foundPath)
        res.pop_back();

    /// Si un chemin a été trouvé, le renvoie. Sinon renvoie une liste vide.
    //map.test_graph = foundPath? res : std::vector<vec2i>();
    return foundPath? res : std::vector<vec2i>();
}

