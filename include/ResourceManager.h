#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "common.h"
#include "Tileset.h"
#include "Controls.h"


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
    KeyValue(T val) { value = val; };
    void setValue(T val) { value = val; };
    T getValue() { return value; };
};



class ResourceManager
{
    public:
        virtual ~ResourceManager();

        static ResourceManager& getInstance();

        void setTheme(std::string name);
        sf::Texture& getThemedTexture(std::string path);
        sf::Texture& getTexture(std::string path);

        Tileset& getTileset();
        void setTileset(std::string name);

        //std::string getDirectory(std::string name);
        bool hasKeyValue(std::string key);
        js::Value& getKeyValue(std::string key);
        void setKeyValue(std::string key, js::Value& val);

        bool getKeyValueBool(std::string key);
        int getKeyValueInt(std::string key);
        float getKeyValueFloat(std::string key);
        std::string getKeyValueString(std::string key);

        Controls& getControlsByName(std::string name);

    protected:
    private:
        static ResourceManager* s_instance;
        std::map<std::string, sf::Texture*> m_textures;
        std::map<std::string, js::Value*> m_keys;
        std::map<std::string, Controls*> m_controls;
        std::string m_theme;
        Tileset m_tileset;
        js::Document m_settings;


        ResourceManager();
        ResourceManager(ResourceManager const&);

        sf::Texture* _getTexture(std::string path);
        void loadSettings();
};

#endif // RESOURCEMANAGER_H
