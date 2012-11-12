#include "Mob.h"
#include "ResourceManager.h"
#include "Game.h"

using namespace std;

Mob::Mob(Game& game) : Entity(game)
{
    ResourceManager& rm = ResourceManager::getInstance();

    m_canSwim = rm.getKeyValueBool("p-can-swim");
    m_canHang = rm.getKeyValueBool("p-can-hang");
    m_size.x = rm.getKeyValueInt("p-size-w");
    m_size.y = rm.getKeyValueInt("p-size-h");
    m_maxLife = rm.getKeyValueInt("p-life");
    m_maxOxygen = rm.getKeyValueInt("p-oxygen");
    m_drowningDamage = rm.getKeyValueInt("p-drown-damage");
    m_life = m_maxLife;
    m_oxygen = m_maxOxygen;
    m_speedX = 0;
    m_accelX = rm.getKeyValueFloat("p-accel-x");
    m_maxSpeedX = rm.getKeyValueFloat("p-speed-x");
    m_maxSpeedY = rm.getKeyValueFloat("p-speed-y");
    m_waterSpeedCoeff = rm.getKeyValueFloat("p-speed-water-coeff");
    //m_waterMaxSpeedX = rm.getKeyValueFloat("p-speed-water-x");
    //m_waterMaxSpeedY = rm.getKeyValueFloat("p-speed-water-y");
    m_speedY = 0;
    m_accelY = rm.getKeyValueFloat("p-accel-y");
    m_jumpStr = -rm.getKeyValueFloat("p-jump-str");
    m_waterJumpStr = -rm.getKeyValueFloat("p-jump-str-water");
    m_leaveWaterJumpStr = -rm.getKeyValueFloat("p-jump-str-leave-water");
    m_debug = rm.getKeyValueBool("g-debug");
    //m_onTheGround = false;
    //m_ladderState = false;
    //m_inWater = false;
    //m_ignoreLadder = false;
    //m_hanging = false;
    m_time = 0;
    state = ST_JUMP;
    m_dir = 1;
}

Mob::~Mob()
{
    //dtor
}

void Mob::draw(sf::RenderWindow& win, int elapsedTime)
{
    int image = 0;
    int t = (elapsedTime % 400) / 200;

    if (state == ST_DEAD)
    {
        image = 8;
        m_sprite.setRotation(90);
    }
    if (state == ST_CLIMB)
        image = 4 + t;
    else if (state == ST_HANG)
        image = 6;
    else
    {
        if (state == ST_STAND)
        {
            image = 0 + (m_speedX)? t : 0;
        }
        else if (!m_canBreathe)
        {
            image = 7;
        }
        else if (m_speedY <= 0)
            image = 2;
        else if (m_speedY > 0)
            image = 3;
    }

    m_sprite.setTextureRect(sf::IntRect(image * m_size.x,
                                        0 * m_size.y,
                                        m_size.x,
                                        m_size.y));

    m_sprite.scale((m_sprite.getScale().x == m_dir)? 1 : -1, 1.f);

    m_sprite.setOrigin(m_size.x/2, m_size.y);

    m_sprite.setPosition((int)m_pos.x, (int)m_pos.y);

    drawSprite(win);
}

void Mob::update(float frameTime)
{
    if (state == ST_DEAD)
    {/*
        m_speedY = ((m_inWater)?  m_waterJumpStr : m_jumpStr) / 2;
        m_pos.y += m_speedY * frameTime;

        m_speedY += m_accelY * (m_inWater? 0.02 : 1) * frameTime;*/
    }
    else if (state == ST_STAND || state == ST_JUMP)
    {
        // dans l'eau?
        bool wasInWater = m_inWater;
        m_inWater = m_game.isCollision(m_pos.x, m_pos.y - 8, TILE_WATER);
        m_canBreathe = !m_inWater || !m_game.isCollision(m_pos.x, m_pos.y - 14, TILE_WATER);

        // la vitesse baisse quand on arrive dans l'eau
        if (m_inWater && !wasInWater)
            m_speedY *= 0.2;

        if (m_canBreathe)
            m_oxygen = m_maxOxygen;

        // à chaque seconde
        m_time += frameTime*40;
        if (m_time >= 1000)
        {
            if (!m_canBreathe)
            {
                m_oxygen -= 1;
                if (m_oxygen < 0)
                {
                    m_oxygen = 0;
                    damage(m_drowningDamage);
                    if (state == ST_DEAD)
                        return;
                }
            }

            m_time -= 1000;
        }

        // pics?
        if (m_game.isCollision(getBoundingBox(), TILE_SPIKE))
        {
            damage(m_maxLife);
            if (state == ST_DEAD)
                return;
        }

        // grimpe?
        if ((isActionPressed(ACT_UP)
             && m_game.isCollision(m_pos.x - m_size.x/2, m_pos.y - 1, TILE_LADDER)
             && m_game.isCollision(m_pos.x + m_size.x/2, m_pos.y - 1, TILE_LADDER))
          || (isActionPressed(ACT_DOWN)
              && m_game.isCollision(m_pos.x - m_size.x/2, m_pos.y + 1, TILE_LADDER)
              && m_game.isCollision(m_pos.x + m_size.x/2 , m_pos.y + 1, TILE_LADDER)))
        {
            state = ST_CLIMB;
            return;
        }

        // Mouvement X
        if (isActionPressed(ACT_LEFT))
        {
            m_speedX -= m_accelX * frameTime;
            m_dir = -1;
        }
        if (isActionPressed(ACT_RIGHT))
        {
            m_speedX += m_accelX * frameTime;
            m_dir = 1;
        }

        if (m_speedX > m_maxSpeedX)
            m_speedX = m_maxSpeedX;
        if (m_speedX < -m_maxSpeedX)
            m_speedX = -m_maxSpeedX;

        m_pos.x += m_speedX * (m_inWater? m_waterSpeedCoeff : 1) * frameTime;

        // collisions X
        if (m_game.isCollision(getBoundingBox()))
        {
            while (m_game.isCollision(getBoundingBox()))
            {
                m_pos.x -= (m_speedX > 0)? 0.1 : -0.1;
            }

            m_speedX = 0;
        }

        // décélération
        m_speedX -= m_speedX * 0.3 * frameTime;
        if (m_speedX < 0.2 * frameTime && m_speedX > -0.2 * frameTime)
            m_speedX = 0;

        // Mouvement Y
        m_speedY += m_accelY * (m_inWater? 0.02 : 1) * frameTime;
        if (m_speedY > m_maxSpeedY)
            m_speedY = m_maxSpeedY;

        {/*
        m_pos.y += m_speedY;

        m_onTheGround = false;

        if (m_game.isCollision(getBoundingBox()))
        {
            m_onTheGround = true;
            m_ignoreLadder = false;

            m_pos.y -= m_speedY;
            bool positiveSpeed = m_speedY > 0;
            short signe = positiveSpeed? 1 : -1;

            float i;
            bool addRest = true;
            for (i = m_speedY; positiveSpeed? (i >= 1) : (i <= -1); i -= signe)
            {
                if (m_game.isCollision(getBoundingBox()))
                {
                    addRest = false;
                    break;
                }
                m_pos.y += signe;
            }
            if (addRest)
                m_pos.y += i * signe;

            m_pos.y -= signe;
            m_speedY = 0;
        }
        */}

        state = ST_JUMP;
        if (m_speedY > 0)
        {
            for (float i = m_speedY * frameTime; i > 0; i -= 0.1)
            {
                m_pos.y += 0.1;

                if (m_game.isCollision(getBoundingBox())
                    || (m_game.isCollision(m_pos.x, m_pos.y-1, TILE_LADDER)
                        && !m_game.isCollision(m_pos.x, m_pos.y-2, TILE_LADDER)))
                {
                    m_pos.y -= 0.1;
                    state = ST_STAND;
                    m_ignoreLadder = false;
                    m_speedY = 0;
                    break;
                }
                else if (m_canHang)
                {
                    if ((m_game.isCollision(m_pos.x - m_size.x/2 - 1, m_pos.y - m_size.y + 2)
                        && !m_game.isCollision(m_pos.x - m_size.x/2 - 1, m_pos.y - m_size.y + 1)
                        && !m_game.isCollision(m_pos.x, m_pos.y + 3)
                        && m_dir == -1
                        && isActionPressed(ACT_LEFT))
                        ||
                        (m_game.isCollision(m_pos.x + m_size.x/2, m_pos.y - m_size.y + 2)
                        && !m_game.isCollision(m_pos.x + m_size.x/2, m_pos.y - m_size.y + 1)
                        && !m_game.isCollision(m_pos.x, m_pos.y + 3)
                        && m_dir == 1
                        && isActionPressed(ACT_RIGHT)))
                    {
                        state = ST_HANG;
                        m_speedY = 0;
                        break;
                    }
                }
                else if (m_game.isCollision(m_pos.x + m_size.x/2 + 1, m_pos.y-1)
                        )
                {
                    m_speedY = 0;
                    break;
                }
            }
        }
        else
        {
            for (float i = m_speedY * frameTime; i < 0; i += 0.1)
            {
                m_pos.y -= 0.1;

                if (m_game.isCollision(getBoundingBox()))
                {
                    m_pos.y += 0.1;
                    state = ST_STAND;
                    m_ignoreLadder = false;
                    m_speedY = 0;
                    break;
                }
            }
        }

        // collision échelles
        //if (state != ST_HANG && !m_ignoreLadder && m_game.isCollision(m_pos.x, m_pos.y, TILE_LADDER) && m_speedY > 0)
        //{
        //    state = ST_STAND;
        //    m_pos.y -= m_speedY;
        //    m_speedY = 0;
        //}
    }
    else if (state == ST_CLIMB)
    {
        m_speedY = 0;
        m_speedX = 0;
        vec2f lastpos = m_pos;

        if (isActionPressed(ACT_UP))
        {
            m_pos.y -= 1.5 * frameTime;
        }
        if (isActionPressed(ACT_DOWN))
        {
            m_pos.y += 1.5 * frameTime;
        }

        if (m_game.isCollision(getBoundingBox()))
            m_pos = lastpos;

        if (!m_game.isCollision(m_pos.x, m_pos.y + 1, TILE_LADDER))
            state = ST_JUMP;
    }

    Entity::update(frameTime);
}

void Mob::actionPressed(Action act, bool pressed)
{
    if (pressed && state != ST_DEAD)
    {
        if (act == ACT_JUMP && (state == ST_STAND || state == ST_HANG || (m_inWater && m_canSwim)))
        {
            m_speedY = (m_inWater)? (m_canBreathe? m_leaveWaterJumpStr : m_waterJumpStr) : m_jumpStr;
            state = ST_JUMP;
        }
        if (act == ACT_JUMP && state == ST_CLIMB)
        {
            state = ST_JUMP;
            m_ignoreLadder = true;
        }
        if (act == ACT_DOWN && state == ST_HANG)
            state = ST_JUMP;

        if (act == ACT_JUMP && state == ST_HANG)
            state = ST_JUMP;

        if (m_debug && act == ACT_1)
            m_game.destroyTileAt(m_pos.x, m_pos.y + 8);
    }
}

void Mob::setSkin(string name)
{
    // charge une nouvelle texture pour le sprite
    ResourceManager& rm = ResourceManager::getInstance();

    string path = rm.getKeyValueString("d-skins") + name + ".png";

    m_sprite.setTexture(rm.getTexture(path));
}

void Mob::setPosition(vec2i pos)
{
    pos.x += m_size.x/2;
    pos.y += m_size.y;

    Entity::setPosition(pos);
};

void Mob::damage(int pv)
{
    m_life -= pv;

    if (m_life <= 0)
    {
        m_life = 0;
        die();
    }
    cout << "ouch! " << m_life << endl;
}

void Mob::die()
{
    state = ST_DEAD;
    cout << "aargh" << endl;
}

sf::IntRect Mob::getBoundingBox()
{
    return sf::IntRect(m_pos.x - m_size.x/2, m_pos.y - m_size.y, m_size.x - 1, m_size.y - 1);
}
