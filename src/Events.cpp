#include "Events.hpp"
#include "Zombie.hpp"

namespace evt
{
    void wakeTheDead(Game& game)
    {
        const TileMap& map = game.getTileMap();
        const std::list<Tile*>& positions = map.test_tombstones;

        for (std::list<Tile*>::const_iterator it=positions.begin(); it != positions.end(); ++it)
        {
            Zombie& z = * new Zombie(game);
            z.setSkin("zombie");
            z.setPosition(map.fromTileCoords( (*it)->getPosition() ));
            z.setTarget(*game.getPlayers().front()->getHuman());
        }
    }

    void quakeTheEarth(Game& game)
    {
        const TileMap& map = game.getTileMap();


    }
}
