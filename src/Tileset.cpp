#include "Tileset.hpp"
#include "ResourceManager.hpp"

using namespace std;

Tileset::Tileset() : m_texture(NULL)
{
    cout << "Tileset créé." << endl;
}

Tileset::~Tileset()
{
    //dtor
}

void Tileset::loadFromFile(string path)
{
    // On charge le fichier json
    js::Document json;

    FILE* handle = fopen(path.c_str(), "r");
    assert(handle != NULL);

    js::FileStream fstream(handle);

	assert(!json.ParseStream<0>(fstream).HasParseError());
	assert(json.IsObject());

	// On charge la texture du tileset
    string texturePath(rm::getKeyValue<std::string>("d-tilesets"));
    texturePath += json["texture"].GetString();
    m_texture = &rm::getTexture(texturePath);

    // Taille des tuiles
    m_tileSize = json["tilesize"].GetInt();

	// Fichier chargé, on remplit le dico

	// D'abord la correspondance char/nom
    js::Value& chars = json["chars"];
	assert(chars.IsArray());

    for (js::SizeType i = 0; i < chars.Size(); i++)
    {
        const js::Value& tileValue = chars[i];

        assert(tileValue.IsObject());
        assert(tileValue.HasMember("char") && tileValue["char"].IsString());
        assert(tileValue.HasMember("tile") && tileValue["tile"].IsString());

        string ch = tileValue["char"].GetString();

        assert(ch.size() == 1);
        m_namesByChar[ch[0]] = tileValue["tile"].GetString();

        cout << " (" << ch[0] << ")";
    }

	// Les tuiles normales
    js::Value& tiles = json["normaltiles"];
	assert(tiles.IsArray());

    for (js::SizeType i = 0; i < tiles.Size(); i++)
    {
        const js::Value& tileValue = tiles[i];
        assert(tileValue.IsObject());

        TileInfo* tileData = new TileInfo;

        assert(tileValue.HasMember("name") && tileValue["name"].IsString());
        assert(tileValue.HasMember("type") && tileValue["type"].IsString());
        assert(tileValue.HasMember("x") && tileValue["x"].IsInt());
        assert(tileValue.HasMember("y") && tileValue["y"].IsInt());

        tileData->name = tileValue["name"].GetString();
        tileData->type = Tile::getTypeByString(tileValue["type"].GetString());
        tileData->x = tileValue["x"].GetInt();
        tileData->y = tileValue["y"].GetInt();

        // Émet de la lumière?
        tileData->light = (tileValue.HasMember("light"))? tileValue["light"].GetInt() : 0;

        // animation?
        tileData->nbframes = 1;
        tileData->delay = 200;
        if (tileValue.HasMember("nbframes"))
        {
            assert(tileValue["nbframes"].IsInt());
            tileData->nbframes = tileValue["nbframes"].GetInt();

            if (tileValue.HasMember("delay"))
            {
                assert(tileValue["delay"].IsInt());
                tileData->delay = tileValue["delay"].GetInt();
            }
        }


        cout << " Tuile : " << tileData->name;

        // Ajout aux dictionnaires

        if (tileData->name.size())
            m_normalTiles[tileData->name] = tileData;

        cout << endl;
    }


    // On passe aux randomtiles
    js::Value& randtiles = json["randomtiles"];
	assert(randtiles.IsArray());

    for (js::SizeType i = 0; i < randtiles.Size(); i++)
    {
        const js::Value& tileValue = randtiles[i];
        assert(tileValue.IsObject());

        RandomTileData* tileData = new RandomTileData;

        assert(tileValue.HasMember("name") && tileValue["name"].IsString());
        assert(tileValue.HasMember("tiles") && tileValue["tiles"].IsArray());

        tileData->name = tileValue["name"].GetString();
        tileData->totalProbas = 0;

        int proba;
        bool isProbas = tileValue.HasMember("probas")
                         && tileValue["probas"].Size() == tileValue["tiles"].Size();

        for (js::SizeType j = 0; j < tileValue["tiles"].Size(); j++)
        {
            assert(tileValue["tiles"][j].IsString());

            tileData->tiles.push_back(getExistingTileInfoFromName(tileValue["tiles"][j].GetString()));


            if (isProbas)
                proba = tileValue["probas"][j].GetInt();
            else
                proba = 1;

            tileData->probas.push_back(proba);
            tileData->totalProbas += proba;
        }

        cout << " Tuile aléa : " << tileData->name;

        // Ajout aux dictionnaires

        if (tileData->name.size())
            m_randomTiles[tileData->name] = tileData;

        cout << endl;
    }


    // Et enfin aux tuiles automatiques!
    js::Value& autotiles = json["autotiles"];
	assert(autotiles.IsArray());

    for (js::SizeType i = 0; i < autotiles.Size(); i++)
    {
        const js::Value& tileValue = autotiles[i];
        assert(tileValue.IsObject());

        string voidTile = "void";

        AutoTileData* tileData = new AutoTileData;

        assert(tileValue.HasMember("name") && tileValue["name"].IsString());

        tileData->name = tileValue["name"].GetString();

        if (tileValue.HasMember("mid"))
            tileData->mid = tileValue["mid"].GetString();
        else
            tileData->mid = voidTile;

        if (tileValue.HasMember("top"))
            tileData->top = tileValue["top"].GetString();
        else
            tileData->top = tileData->mid;

        if (tileValue.HasMember("bot"))
            tileData->bot = tileValue["bot"].GetString();
        else
            tileData->bot = tileData->mid;

        if (tileValue.HasMember("one"))
            tileData->one = tileValue["one"].GetString();
        else
            tileData->one = tileData->mid;

        if (tileValue.HasMember("over"))
            tileData->over = tileValue["over"].GetString();
        else
            tileData->over = voidTile;

        if (tileValue.HasMember("under"))
            tileData->under = tileValue["under"].GetString();
        else
            tileData->under = voidTile;



        cout << " Tuile auto : " << tileData->name;

        // Ajout aux dictionnaires

        if (tileData->name.size())
            m_autoTiles[tileData->name] = tileData;

        cout << endl;
    }

    cout << "Tileset chargé" << endl;
}

TileInfo* Tileset::getTileInfoFromName(string s)
{
    // trouve la tuile correspondant au nom

    std::map<string, TileInfo*>::iterator it;
    std::map<string, RandomTileData*>::iterator it2;

    // Cherche dans les tuiles normales
    it = m_normalTiles.find(s);
    if (it != m_normalTiles.end())
        return it->second;

    // Cherche dans les tuiles aléatoires
    it2 = m_randomTiles.find(s);
    if (it2 != m_randomTiles.end())
    {
        // Trouvé! Sélectionne tuile au hasard, tenant compte des probas

        RandomTileData* r = it2->second;
        int n = (rand() % r->totalProbas) + 1;
        int i = -1;

        do
        {
            i++;
            n -= r->probas[i];
        }
        while (n > 0);

        // ATTENTION! Pas sûr de pouvoir passer un pointeur sur une valeur retournée par std::vector::operator[]
        return & r->tiles[i];
    }

    return NULL;
}

TileInfo& Tileset::getExistingTileInfoFromName(string s)
{
    TileInfo* tileInfo = getTileInfoFromName(s);
    assert(tileInfo != NULL);

    return *tileInfo;
}

string Tileset::getTileNameFromChar(char c)
{
    // il faut que le caractère soit dans la map

    // trouve le nom de la tuile
    std::map<char, string>::iterator it = m_namesByChar.find(c);
    if (it == m_namesByChar.end())
    {
        cout << "ERREUR: Tuile '" << c << "' introuvable." << endl;
        assert(false);
    }

    return it->second;
}

Tile* Tileset::makeTileFromChar(TileMap& map, char c)
{
    Tile* tile;

    // On sait que l'espace ne correspond à rien
    if (c == ' ')
        return NULL;

    string name = getTileNameFromChar(c);

    // Cherche dans les tuiles normales/aléatoires

    TileInfo* tileInfo = getTileInfoFromName(name);

    if (tileInfo != NULL)
    {
        tile = new Tile(map, *tileInfo);
        //tile->loadFromTileInfo(*tileInfo);

        return tile;
    }

    // Si ça n'y est pas...

    std::map<std::string, AutoTileData*>::iterator it;
    it = m_autoTiles.find(name);
    if (it != m_autoTiles.end())
    {
        AutoTileInfo tileInfo;
        tileInfo.name = it->second->name;
        tileInfo.type = tileInfo.top.type;
        tileInfo.top = getExistingTileInfoFromName(it->second->top);
        tileInfo.bot = getExistingTileInfoFromName(it->second->bot);
        tileInfo.mid = getExistingTileInfoFromName(it->second->mid);
        tileInfo.one = getExistingTileInfoFromName(it->second->one);
        tileInfo.over = getExistingTileInfoFromName(it->second->over);
        tileInfo.under = getExistingTileInfoFromName(it->second->under);

        tile = new AutoTile(map, tileInfo);
        return tile;
    }

    cout << "Attention, retourne NULL" << endl;

    return NULL;
}

sf::Texture& Tileset::getTexture()
{
    assert(m_texture != NULL);

    return *m_texture;
}

int Tileset::getTileSize()
{
    return m_tileSize;
}
