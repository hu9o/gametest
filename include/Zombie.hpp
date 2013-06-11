#ifndef BOT_H
#define BOT_H

#include "Mob.hpp"
#include "TileMap.hpp"

class Zombie : public Mob
{
    public:
        Zombie(Game& game);
        virtual ~Zombie();

        virtual void update(float frameTime);

        void goTo(vec2i targetPos);
        void wander();
        void seekAir();
        bool isIdle() const;
        void setTarget(const Entity& e);

        static void updateGraphAt(const TileMap& map, int tileX, int tileY);

    protected:
        std::vector<vec2i> m_path;
        sf::Clock changePathClock;
        const Entity* m_target;
        bool m_wandering;
        float m_reloadTime;

        // optimisation (ou pas ?)
        enum FindPathAction { PATH_FIND, PATH_WANDER, PATH_SEEK_AIR };
        struct GraphNode
        {
            int pos; // x + y*width
            std::vector<int> neighboursPos;
        };
        static std::map<int, GraphNode> s_graph;
        static bool s_useGraph;
        static std::vector<vec2i> findPath(const TileMap& map, vec2i sourcePos, vec2i targetPos, FindPathAction action);


        struct Node
        {
            Node(Node* _p, int _x, int _y, Tile* _c)
             : parent(_p), x(_x), y(_y), c(_c)
            {
                if (parent != NULL)
                {
                    target = parent->target;
                    distance = parent->distance + 1;
                }
                else
                {
                    target = NULL;
                    distance = 0;
                }
            }

            int getF() { return g+getH(); }

            int getH()
            {
                return (target != NULL)? abs(x-target->x)+abs(y-target->y) : 0;
            }

            Node* parent;
            Node* target;
            int x, y;
            Tile* c;
            int g;
            int cost;
            int distance;
        };

    private:
};

#endif // BOT_H
