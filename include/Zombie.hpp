#ifndef BOT_H
#define BOT_H

#include "Mob.hpp"
#include "TileMap.hpp"

class Zombie : public Mob
{
    public:
        Zombie(Game& game);
        virtual ~Zombie();

        void goTo(vec2i targetPos);
        virtual void update(float frameTime);
        bool isIdle() const;
        void setTarget(const Entity& e);

    protected:
        std::vector<vec2i> m_path;
        sf::Clock changePathClock;
        const Entity* m_target;

        std::vector<vec2i> findPath(vec2i sourcePos, vec2i targetPos) const;

        struct Node
        {
            Node(Node* _p, int _x, int _y, Tile* _c)
             : parent(_p), x(_x), y(_y), c(_c)
            {
                if (parent != NULL)
                    target = parent->target;
                else
                    target = NULL;
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
        };

    private:
};

#endif // BOT_H
