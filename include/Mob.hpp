#ifndef MOB_H
#define MOB_H

#include "Entity.hpp"
#include "Controllable.hpp"

class Mob : public Entity, public Controllable
{
    public:
        enum State {ST_STAND, ST_JUMP, ST_CLIMB, ST_HANG, ST_DEAD};

        Mob(Game& game);
        virtual ~Mob();

        virtual void draw(sf::RenderTarget& win, int elapsedTime);
        virtual void update(float frameTime);

        void setSkin(std::string name);
        void setPosition(vec2i pos);
        void damage(int pv);

    protected:
        vec2i m_size;

        int m_life;
        int m_maxLife;
        int m_oxygen;
        int m_maxOxygen;
        int m_drowningDamage;
        bool m_canSwim;
        bool m_canHang;
        bool m_debug;

        float m_speedX;
        float m_waterSpeedCoeff;
        float m_accelX;
        float m_maxSpeedX;
        //float m_waterMaxSpeedX;
        float m_speedY;
        float m_accelY;
        float m_maxSpeedY;
        //float m_waterMaxSpeedY;
        float m_jumpStr;
        float m_waterJumpStr;
        float m_leaveWaterJumpStr;

        int m_time;

        // STANDING / FLYING / CLIMBING / SWIMMING / HANGING
        State m_state;
        //bool m_onTheGround;
        //bool m_ladderState;
        bool m_inWater;
        bool m_canBreathe;
        bool m_ignoreLadder;
        //bool m_hanging;
        int m_dir;
        //weapon
        //controllable
        //holdable

        virtual void actionPressed(Action act, bool pressed);
        sf::IntRect getBoundingBox();
        void die();

    private:
};

#endif // MOB_H
