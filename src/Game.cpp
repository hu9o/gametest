#include "Game.hpp"
#include "ResourceManager.hpp"
#include "Player.hpp"
#include "Zombie.hpp"

using namespace std;

Game::Game(sf::RenderWindow& win) : m_win(win)
{

}

Game::~Game()
{
    m_debugMode = rm::getKeyValueBool("g-debug");
}

void Game::run()
{
    mainLoop();
}

void Game::loadFromFile(string path)
{
    cout << "Chargement du niveau" << endl;

    FILE* handle;

    string fullpath(rm::getKeyValueString("d-maps"));
    fullpath += path;

    handle = fopen(fullpath.c_str(), "r");
    assert(handle != NULL);

    js::FileStream fstream(handle);

	assert(!json.ParseStream<0>(fstream).HasParseError());

	// TODO: fclose?

	assert(json.HasMember("name") && json["background"].IsString());
    cout << "La carte s'appelle \"" << json["name"].GetString() << "\"" << endl;

    // Clés avant le reste!
	assert(json.HasMember("keys"));// member iterator

	js::Value keys;
	keys = json["keys"];
	string overloadable = rm::getKeyValueString("g-overloadable-keys-map");

    js::Value::MemberIterator itr = keys.MemberBegin();
    while(itr != keys.MemberEnd())
    {
        string name = itr->name.GetString();
        bool authorized = false;

        for (uint i=0; i<overloadable.length(); i++)
        {
            if (overloadable[i] == name[0])
            {
                authorized = true;
                break;
            }
        }

        if (authorized && name[1] == '-')
        {
            if (rm::hasKeyValue(name))
            {
                rm::setKeyValue(name, itr->value);
                cout << " Clé: " << name << endl;
            }
            else
                cout << " Clé ignorée: " << name << " (inexistante dans conf.json)" << endl;
        }
        else
            cout << " Clé ignorée: " << name << " (surcharge illégale)" << endl;

        ++itr;
    }
    // fin clés

    // carte
	assert(json.HasMember("tileset") && json["background"].IsString());
    rm::setTileset(json["tileset"].GetString());

	assert(json.HasMember("background") && json["background"].IsString());
	sf::Texture& bgtex = rm::getTexture(rm::getKeyValueString("d-backgrounds") + json["background"].GetString());
	bgtex.setRepeated(true);
	m_bg.setTexture(bgtex);
	m_bg.setTextureRect(sf::IntRect(0, 0, 400, 300));

	assert(json.HasMember("tilemap"));
	m_tilemap.loadFromJson(json["tilemap"]);

	assert(json.HasMember("entities"));
}

void Game::mainLoop()
{
    cout << "Début de la boucle" << endl;

    // Réglages
    m_win.setFramerateLimit(rm::getKeyValueInt("g-framerate-limit"));
    cout << "Début de la boucle" << endl;
    m_win.setKeyRepeatEnabled(false);
    bool framerateAdjust = rm::getKeyValueBool("g-framerate-adjust");
    // doubletaille
    sf::View view = m_win.getView();
    view.zoom(0.5);
    view.setCenter(200, 150);
    m_win.setView(view);
    m_usePostFX = sf::Shader::isAvailable();
    //sf::Shader lightEffect;
    //sf::Sprite screen;

    /*if (m_usePostFX)
    {
        lightEffect.setParameter("framebuffer", sf::Shader::CurrentTexture);
        lightEffect.setParameter("color", 1.f, 1.f, 1.f);
        bool okay = lightEffect.loadFromFile(rm::getDirectory("post-fx") + "colorize.sfx", sf::Shader::Fragment);
        assert(okay);

        lightEffect.bind();
    }*/

    // Temps
    sf::Clock clock;

    // Map
    loadFromFile(rm::getKeyValueString("g-default-map"));

    // Joueurs et persos
    rm::createPlayers(*this, m_tilemap, m_players);

    // Zombies
    for (int i=rm::getKeyValueInt("m-nb-zombies"); i>0; i--)
    {
        Zombie* z = new Zombie(*this);
        z->setSkin("zombie");
        z->setPosition(m_tilemap.getStartPosition());
        z->setTarget(*m_players.front()->getHuman());
    }

    // Shader
    sf::Shader shader;
    shader.loadFromFile(rm::getKeyValueString("d-resources")+"blur.frag", sf::Shader::Fragment);
    shader.setParameter("texture", sf::Shader::CurrentTexture);
    shader.setParameter("blur_radius", 0.01);


    sf::RenderTexture shadowTex;
    shadowTex.create(m_win.getSize().x, m_win.getSize().y, 32);

    // Boucle
    int timeElapsed = clock.getElapsedTime().asMilliseconds();
    float frameTime;
    sf::Clock zombieClock;

    while (m_win.isOpen())
    {
        sf::Event evt;
        sf::Clock tick;


        view.setCenter(200, 150);
        m_win.setView(view);

        frameTime = (float)(clock.getElapsedTime().asMilliseconds() - timeElapsed) / 40;
        timeElapsed = clock.getElapsedTime().asMilliseconds();

        while (m_win.pollEvent(evt))
        {
            if (evt.type == sf::Event::Closed || (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::Escape))
                return;

            if (evt.type == sf::Event::KeyPressed || evt.type == sf::Event::KeyReleased)
            {
                for (list<Player*>::iterator it = m_players.begin(); it != m_players.end(); ++it)
                {
                    (*it)->pressKey(evt.key.code, evt.type == sf::Event::KeyPressed);
                }
            }

            if (evt.type == sf::Event::MouseButtonPressed)
            {
                //john.goTo(vec2i(evt.mouseButton.x/32, evt.mouseButton.y/32));
            }
        }

        m_win.clear();

        // Ordre d'affichage: background, backtiles, player, fronttiles, water, overlay

        m_win.draw(m_bg);

        // Affiche la carte
        // on la décale de 24px vers le bas
        //view.move(0, -12);

        // couche 1
        m_tilemap.drawLayer(m_win, LAYER_BACK, timeElapsed);

        // affiche les entités
        for (list<Entity*>::iterator it = m_entities.begin(); it != m_entities.end(); it++)
        {
            (*it)->update(framerateAdjust? frameTime : 1);
            (*it)->draw(m_win, timeElapsed);
        }

        // couches 2, 3 et 4
        m_tilemap.drawLayer(m_win, LAYER_FRONT, timeElapsed);
        m_tilemap.drawLayer(m_win, LAYER_OVERLAY, timeElapsed);
        m_tilemap.drawLayer(m_win, LAYER_WATER, timeElapsed);

        sf::RenderStates states;
        states.shader = &shader;
        shadowTex.clear(sf::Color::Transparent);
        m_tilemap.drawLayer(shadowTex, LAYER_SHADOW, timeElapsed);
        shadowTex.display();
        m_win.draw(sf::Sprite(shadowTex.getTexture()), states);

        m_tilemap.drawLayer(m_win, LAYER_TEST, timeElapsed);


        //screen.copyScreen(m_win);
        //m_win.draw(sf::Sprite(screen), lightEffect);


        m_win.display();

        //cout << frameTime << endl;
    }

}

void Game::registerEntity(Entity& e)
{
    m_entities.push_back(&e);
}

const TileMap& Game::getTileMap() const
{
    return m_tilemap;
}

void Game::destroyTileAt(int x, int y)
{
    m_tilemap.destroyTileAt(x, y);
}
