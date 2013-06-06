#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "common.hpp"
#include "Tileset.hpp"
#include "Controls.hpp"
#include "Player.hpp"
#include <list>

namespace rm
{
    /*
    struct GenericKeyValue
    {
        static GenericKeyValue* newGenericKeyValue(js::Value&);
        static GenericKeyValue* newGenericKeyValue(bool);
        static GenericKeyValue* newGenericKeyValue(int);
        static GenericKeyValue* newGenericKeyValue(float);
        static GenericKeyValue* newGenericKeyValue(std::string);

        virtual bool getBool();
        int getInt();
        float getFloat();
        std::string getString();

        protected:
            GenericKeyValue();
    };

    template<typename T>
    struct KeyValue : GenericKeyValue
    {
        T value;
        KeyValue(T val) { value = val; }
        void setValue(T val) { value = val; }
        T getValue() { return value; }
    };
    */

    void init();

    void setTheme(const std::string& name);
    sf::Texture& getThemedTexture(const std::string& path);
    sf::Texture& getTexture(const std::string& path);

    Tileset& getTileset();
    void setTileset(const std::string& name);

    sf::Text getText(const std::string& content, int size=16);

    //std::string getDirectory(std::string name);
    bool hasKeyValue(const std::string& key);
    js::Value& getKeyJsValue(const std::string& key);
    void setKeyJsValue(const std::string& key, js::Value& val);

    template<typename T> T getKeyValue(const std::string& key);
    template<> inline bool getKeyValue<bool>(const std::string& key) { return getKeyJsValue(key).GetBool(); }
    template<> inline int getKeyValue<int>(const std::string& key) { return getKeyJsValue(key).GetInt(); }
    template<> inline float getKeyValue<float>(const std::string& key) { return getKeyJsValue(key).GetDouble(); }
    template<> inline double getKeyValue<double>(const std::string& key) { return getKeyJsValue(key).GetDouble(); }
    template<> inline std::string getKeyValue<std::string>(const std::string& key) { return getKeyJsValue(key).GetString(); }

    Controls& getControlsByName(const std::string& name);
    void createPlayers(Game& game, TileMap& tilemap, std::list<Player*>& players);


    static sf::Texture* _getTexture(const std::string& path);

    static std::map<std::string, sf::Texture*> m_textures;
    static std::map<std::string, js::Value*> m_keys;
    static std::map<std::string, Controls*> m_controls;
    static std::string m_theme = "gfx/default";
    static Tileset m_tileset;
    static js::Document m_settings;
    static sf::Font m_font;
}

#endif // RESOURCEMANAGER_H
