#ifndef HUMAN_H
#define HUMAN_H

#include "Mob.hpp"


class Human : public Mob
{
    public:
        static const int MAX_LIFE = 100;

        Human(Game& game);
        virtual ~Human();

    protected:

    private:
};


#endif // HUMAN_H
