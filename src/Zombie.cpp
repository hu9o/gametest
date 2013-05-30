#include "Zombie.hpp"
#include "Game.hpp"

Zombie::Zombie(Game& game) : Mob(game), m_target(NULL)
{
    //ctor
}

Zombie::~Zombie()
{
    //dtor
}

void Zombie::goTo(vec2i targetPos)
{
    m_path = findPath(vec2i(m_pos.x/16, m_pos.y/16-.8), targetPos);
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
    if (!m_state != ST_DEAD)
    {
        bool actLeft = false;
        bool actRight = false;
        bool actJump = false;
        bool actUp = false;
        bool actDown = false;

        if (m_target && changePathClock.getElapsedTime().asSeconds() > 1)
        {
            goTo(vec2i(m_target->getPosition().x/16, m_target->getPosition().y/16-.8));
            changePathClock.restart();
        }

        if (!m_path.empty())
        {
            vec2i currPos(m_pos.x/16, m_pos.y/16-.8);
            vec2i nextPos(m_path.back());
            const TileMap& map = m_game.getTileMap();

            // saute si sur une échelle et but hors de l'échelle
            if (map.tileAtHasType(currPos.x, currPos.y, TILE_LADDER) && !map.tileAtHasType(nextPos.x, nextPos.y, TILE_LADDER))
            {
                actJump = true;
            }

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
                    if (nextPos.x*16+8 < m_pos.x)// && map.tileAtHasType(nextPos.x, nextPos.y+1, TILE_SOLID))
                    {
                        actLeft = true;
                    }
                    if (nextPos.x*16+8 > m_pos.x)// && map.tileAtHasType(nextPos.x, nextPos.y+1, TILE_SOLID))
                    {
                        actRight = true;
                    }
                }

                if (nextPos.y == currPos.y-1)
                {
                    if (nextPos.x == currPos.x && map.tileAtHasType(currPos.x, currPos.y, TILE_LADDER) && !map.tileAtHasType(currPos.x, currPos.y, TILE_SOLID))
                        actUp = true;
                    else
                        actJump = true;
                }

                if (nextPos.y == currPos.y+1)
                    actDown = true;
            }

            if (false && m_pos.x < (float)(nextPos.x)*16+12 && m_pos.x > (float)(nextPos.x)*16+4)
            {
                if (m_pos.x < nextPos.x*16+8)
                    m_pos.x += 2;
                if (m_pos.x > nextPos.x*16+8)
                    m_pos.x -= 2;
            }



        }

        pressAction(ACT_LEFT, actLeft);
        pressAction(ACT_RIGHT, actRight);
        pressAction(ACT_JUMP, actJump);
        pressAction(ACT_UP, actUp);
        pressAction(ACT_DOWN, actDown);
    }

    Mob::update(frameTime);
}

std::vector<vec2i> Zombie::findPath(vec2i sourcePos, vec2i targetPos) const
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

    std::vector<Node*> open, closed;
    //Node* openLowest = NULL;

    open.push_back(&source);
    //openLowest = &source;
    source.g = 0;

    bool loop = true;
    while (!open.empty() && loop)
    {
        std::vector<Node*>::iterator it;
        std::vector<Node*>::iterator currentIt;
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
                    if ((*it)->x == current->x+i &&
                        (*it)->y == current->y+j)
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

                if ((i || j) && notInClosed && !currentCase->hasType(TILE_SOLID))
                {
                    // nage
                    if (currentCase->hasType(TILE_WATER) && ( (!i || !j) || (!map.tileAtHasType(current->x, current->y+j, TILE_SOLID)
                                                                          && !map.tileAtHasType(current->x+i, current->y, TILE_SOLID))))
                    {
                        ok = true;
                    }
                    // tombe
                    else if (!map.tileAtHasType(current->x, current->y+1, TILE_SOLID)
                     && !map.tileAtHasType(current->x-1, current->y, TILE_SOLID)
                     && !map.tileAtHasType(current->x+1, current->y, TILE_SOLID))
                    {
                        if (i==0 && j==1)
                            ok = true;
                    }
                    else
                    {
                        // saute à gauche ou à droite
                        if (j==-1 && i!=0 && !map.tileAtHasType(current->x, current->y-1, TILE_SOLID)
                                        && !map.tileAtHasType(current->x+i, current->y-1, TILE_SOLID)
                                        && map.tileAtHasType(current->x+i, current->y, TILE_SOLID))
                            ok = true;

                        // gauche ou droite
                        if (j==0 && i!=0 && (!map.tileAtHasType(current->x, current->y+j, TILE_SOLID)
                                         && !map.tileAtHasType(current->x+i, current->y, TILE_SOLID)) )
                            ok = true;

                        // échelle
                        if (i==0 && j!=0 && currentCase->hasType(TILE_LADDER) && !currentCase->hasType(TILE_SOLID)
                         && map.tileAtHasType(current->x, current->y+j, TILE_LADDER) && !map.tileAtHasType(current->x, current->y+j, TILE_SOLID))
                            ok = true;

                        // saut simple
                        if (i==0 && j==-1 && !map.tileAtHasType(current->x, current->y+j, TILE_SOLID))
                            ok = true;

                        // tombe
                        if (!map.tileAtHasType(current->x, current->y+1, TILE_SOLID))
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

        for (it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            Node* neigh = *it;
            bool found = false;

            std::vector<Node*>::iterator open_n;
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
                if (neigh->g > (*open_n)->g)
                {
                    neigh->g = (*open_n)->g;
                    neigh->parent = current;
                    delete neigh;
                }

            }
            else
            {
                open.push_back(neigh);
            }

        }

        if (current->x == target.x && current->y == target.y)
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

    /// Retire le premier noeud.
    if (foundPath)
        res.pop_back();

    /// Si un chemin a été trouvé, le renvoie. Sinon renvoie une liste vide.
    //map.test_graph = foundPath? res : std::vector<vec2i>();
    return foundPath? res : std::vector<vec2i>();
}

