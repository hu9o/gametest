#include "ResourceManager.hpp"
#include "TileMap.hpp"

using namespace std;



GenericKeyValue::GenericKeyValue() {}

bool GenericKeyValue::getBool() { return dynamic_cast<KeyValue<bool>*>(this)->getValue(); }
int GenericKeyValue::getInt() { return dynamic_cast<KeyValue<int>*>(this)->getValue(); }
float GenericKeyValue::getFloat() { return dynamic_cast<KeyValue<float>*>(this)->getValue(); }
string GenericKeyValue::getString() { return dynamic_cast<KeyValue<string>*>(this)->getValue(); }

GenericKeyValue* GenericKeyValue::newGenericKeyValue(js::Value& val)
{
    if (val.IsBool())
        return newGenericKeyValue(val.GetBool());
    else if (val.IsInt())
        return newGenericKeyValue(val.GetInt());
    else if (val.IsDouble())
        return newGenericKeyValue((float)val.GetDouble());
    else if (val.IsString())
        return newGenericKeyValue(val.GetString());
    else
        assert(0);
}
GenericKeyValue* GenericKeyValue::newGenericKeyValue(bool val) { return new KeyValue<bool>(val); }
GenericKeyValue* GenericKeyValue::newGenericKeyValue(int val) { return new KeyValue<int>(val); }
GenericKeyValue* GenericKeyValue::newGenericKeyValue(float val) { return new KeyValue<float>(val); }
GenericKeyValue* GenericKeyValue::newGenericKeyValue(string val) { return new KeyValue<string>(val); }



ResourceManager* ResourceManager::s_instance = NULL;

ResourceManager::ResourceManager() : m_theme("gfx/default")
{
    //ctor
    cout << "ResourceManager créé" << endl;

    loadSettings();
}
ResourceManager::ResourceManager(ResourceManager const& rm)
{
}
ResourceManager::~ResourceManager()
{
}

ResourceManager& ResourceManager::getInstance()
{
    if (s_instance == NULL)
        s_instance = new ResourceManager();

    return *s_instance;
}

void ResourceManager::setTheme(std::string name)
{
    m_theme = name;
}

sf::Texture& ResourceManager::getThemedTexture(string path)
{
    // on va chercher le fichier dans le thème sélectionné
    sf::Texture* tex;
    string gfxDir = getKeyValueString("d-graphics");

    string filepath(gfxDir);
    filepath += m_theme + "/" + path;

    tex = _getTexture(filepath);

    if (tex == NULL)
    {
        // fichier introuvable, on va chercher dans 'default'
        string filepath = gfxDir;
        filepath += "default/" + path;

        // le fichier doit être présent dans 'default'
        tex = _getTexture(filepath);
        assert(tex != NULL);
    }

    return *tex;
}

sf::Texture& ResourceManager::getTexture(string path)
{
    // le chargement doit réussir
    sf::Texture* tex = _getTexture(path);
    assert(tex != NULL);

    return *tex;
}

sf::Texture* ResourceManager::_getTexture(string path)
{
    cout << "Demande l'image \"" << path << "\"" << endl;

    map<string, sf::Texture*>::iterator it = m_textures.find(path);

    if (it == m_textures.end())
    {
        cout << "Premier chargement" << endl;

        sf::Texture* tex = new sf::Texture();

        if (!tex->loadFromFile(path))
        {
            cout << "Introuvable, retourne NULL" << endl;
            delete tex;
            tex = NULL;
        }
        else
        {
            cout << "Image chargée" << endl;
            m_textures[path] = tex;
            tex->setSmooth(false);
        }

        return tex;
    }
    else
    {
        cout << "L'image est déjà chargée" << endl;
        return it->second;
    }
}

Tileset& ResourceManager::getTileset()
{
    return m_tileset;
}

void ResourceManager::setTileset(std::string name)
{
    cout << "Chargement du tileset \"" << name << "\"" << endl;
    m_tileset.loadFromFile(string("map/tileset/") + name + ".json");
}

void ResourceManager::loadSettings()
{
    string path = "conf.json";

    // On charge le fichier json

    // TODO: fclose?
    FILE* handle = fopen(path.c_str(), "r");
    assert(handle != NULL);

    js::FileStream fstream(handle);

	assert(!m_settings.ParseStream<0>(fstream).HasParseError());
	assert(m_settings.IsObject());

	//assert(m_settings.HasMember("directories") && m_settings["directories"].IsObject());

	// Clés
	assert(m_settings.HasMember("keys") && m_settings["keys"].IsObject());

	js::Value keys;
	keys = m_settings["keys"];

    js::Value::MemberIterator itr = keys.MemberBegin();
    while(itr != keys.MemberEnd())
    {
        setKeyValue(itr->name.GetString(), itr->value);
        ++itr;
    }

    // Touches
    assert(m_settings.HasMember("keylayouts") && m_settings["keylayouts"].IsObject());
//	assert(m_settings.HasMember("keylayouts") && m_settings["keylayouts"].IsArray());

//	js::Value keylayouts;
//	keylayouts = m_settings["keylayouts"];
//
//    for (js::SizeType i = 0; i < keylayouts.Size(); i++)
//    {
//        KeySet keyset = {keylayouts[i]["up"].GetInt(),
//                         keylayouts[i]["down"].GetInt(),
//                         keylayouts[i]["left"].GetInt(),
//                         keylayouts[i]["right"].GetInt(),
//                         keylayouts[i]["jump"].GetInt(),
//                         keylayouts[i]["act1"].GetInt(),
//                         keylayouts[i]["act2"].GetInt()};
//
//        m_keylayouts[] = keyset;
//
//        ++itr;
//
//    }

}

//string ResourceManager::getDirectory(string name)
//{
//    const js::Value& dirs = m_settings["directories"];
//    const char* cname = name.c_str();
//
//    assert(dirs.HasMember(cname) && dirs[cname].IsString());
//
//    return dirs[cname].GetString();
//}

bool ResourceManager::hasKeyValue(string key)
{
    return m_keys.find(key) != m_keys.end();
}

js::Value& ResourceManager::getKeyValue(string key)
{
    map<string, js::Value*>::iterator it = m_keys.find(key);
    assert(it != m_keys.end());

    return *it->second;
}

bool ResourceManager::getKeyValueBool(std::string key)           { return getKeyValue(key).GetBool(); }
int ResourceManager::getKeyValueInt(std::string key)             { return getKeyValue(key).GetInt(); }
float ResourceManager::getKeyValueFloat(std::string key)         { return getKeyValue(key).GetDouble(); }
std::string ResourceManager::getKeyValueString(std::string key)  { return getKeyValue(key).GetString(); }

// TODO: ATTENTION! Est-ce raisonnable de passer un pointeur sur la valeur passée? Si ça devient NULL?
void ResourceManager::setKeyValue(string key, js::Value& val)
{
    m_keys[key] = &val;
}

Controls& ResourceManager::getControlsByName(std::string name)
{
    // pas déjà chargé
    if (m_controls.find(name) == m_controls.end())
    {
        const js::Value& layouts = m_settings["keylayouts"];

        assert(layouts.HasMember(name.c_str()));
        const js::Value& layout = layouts[name.c_str()];

        assert(layout.HasMember("up") && layout["up"].IsInt());
        assert(layout.HasMember("down") && layout["down"].IsInt());
        assert(layout.HasMember("left") && layout["left"].IsInt());
        assert(layout.HasMember("right") && layout["right"].IsInt());
        assert(layout.HasMember("jump") && layout["jump"].IsInt());
        assert(layout.HasMember("act1") && layout["act1"].IsInt());
        assert(layout.HasMember("act2") && layout["act2"].IsInt());

        KeySet keyset = {static_cast<key>(layout["up"].GetInt()),
                         static_cast<key>(layout["down"].GetInt()),
                         static_cast<key>(layout["left"].GetInt()),
                         static_cast<key>(layout["right"].GetInt()),
                         static_cast<key>(layout["jump"].GetInt()),
                         static_cast<key>(layout["act1"].GetInt()),
                         static_cast<key>(layout["act2"].GetInt())};

        m_controls[name] = new Controls(keyset);
    }

    return *(m_controls[name]);
}

void ResourceManager::createPlayers(Game& game, TileMap& tilemap, std::list<Player*>& players)
{
    const js::Value& pls = m_settings["players"];

    cout << "Il y a " << pls.Size() << " joueurs :" << endl;

    for (js::SizeType i=0; i<pls.Size(); ++i)
    {
        const js::Value& pl = pls[i];
        std::string type = pl["control"].GetString();
        Player* p;

        if (type == "keyboard")
        {
            p = new Player();
        }
        else if (type == "bot")
        {
            // à implémenter...
            continue;
        }
        else
        {
            continue;
        }

        Human* h = new Human(game);

        h->setSkin(pl["skin"].GetString());
        h->setPosition(tilemap.getStartPosition());

        p->setHuman(h);
        p->setControls(getControlsByName(pl["keys"].GetString()));

        players.push_back(p);
    }
}
