#include "json.hpp"
#include <fstream>
#include "TileMap.hpp"
#include "TileList.hpp"
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <iostream>
#include <sstream> 

namespace ns {
    struct MapForJsonStruct {
        std::vector<int> map;
        //ToDo
        std::vector<int> tileSize, mapSize; //what structure would suit these. Note SFML types wont work here.
        std::string textureForMap;
        const int* get_level()
        {
            //since vector must store data contiguously, the following works for array
            int* a = &map[0];
            return a;
        }
    };

    void to_json(nlohmann::json& j, const MapForJsonStruct& ms) {
        j = nlohmann::json{
            {"map",ms.map},
            {"tilesize",ms.tileSize},
            {"mapsize",ms.mapSize},
            {"maptexture",ms.textureForMap}
        };
    }
    void from_json(const nlohmann::json& j, MapForJsonStruct& ms) {
        j.at("map").get_to(ms.map);
        j.at("tilesize").get_to(ms.tileSize);
        j.at("mapsize").get_to(ms.mapSize);
        j.at("maptexture").get_to(ms.textureForMap);
    }


}
using namespace ns;
using nlohmann::json;

// Enumerators for managing transitions between home and map menu.
enum menuState { Home, Map };
enum mapState { Create, Load, None };

// Method declerations.
int DrawMap(ns::MapForJsonStruct ms, tgui::Gui& gui);
void updateTextSize(tgui::Gui& gui);
void homeScreen(tgui::Gui& gui);

//Vectors used to represent map width and height and tile width and height.
sf::Vector2u widthAndHeight(16, 8), tileSize(32, 32);

// Where is the mouse in relation to world coordinates
sf::Vector2f mouseWorldPosition;
// Where is the mouse in relation to screen coordinates
sf::Vector2i mouseScreenPosition;

// Used to save tile map to json file.
std::ofstream outJson;

int window_width = (int)sf::VideoMode::getDesktopMode().width / 2;
int window_height = (int)sf::VideoMode::getDesktopMode().height / 2;
sf::RenderWindow window(sf::VideoMode(window_width, window_width), "Tilemap");

// Keeps track of the current tile type we're painting to the map.
unsigned int paintingTile = 0;

// Map File being loaded e.g. map.json
std::string loadingMapName;

std::string savingMapName;
// Tileset File being loaded e.g. tileset.png
std::string textureForMap;

menuState menuS = Home;
mapState mapS = None;
tgui::Gui gui;

// Used to restart when there is an exception.
bool restart = false;

int main()
{

    do {
        //Restting variables.
        menuS = Home;
        mapS = None;
        restart = false;
        window.setVerticalSyncEnabled(true);
        gui.setWindow(window);

        //Load home screen.
        homeScreen(gui);
        json j;
        std::ifstream i;

        //When load button is pressed
        if (mapS == Load) {
            try {
                i.open(loadingMapName);
                i >> j;
            }
            catch (...) {
                restart = true;
                std::cout << "Map Name Provided Failed" << std::endl;
            }


        }
        //When Create Map button is pressed
        else if (mapS == Create && restart == false) {
            try {
                std::vector<int> map;
                int mapSize = widthAndHeight.x * widthAndHeight.y;
                map.reserve(mapSize);

                for (int i = 0; i < mapSize; i++) {
                    map.push_back(0);
                }

                json jsonMap = {
                    {"map", map},
                    {"tilesize",{tileSize.x,tileSize.y}},
                    {"mapsize",{widthAndHeight.x,widthAndHeight.y}},
                    {"maptexture", textureForMap}
                };

                outJson.open(savingMapName);
                //write json object:
                outJson << jsonMap << std::endl;

                i.open(savingMapName);
                i >> j;

                outJson.close();
            }
            catch (...) {
                restart = true;
                std::cout << "Malformed Input Try Again" << std::endl;
            }

        }
        //output map as json object

        if (restart == false && mapS != None) {
            std::cout << j << std::endl;
            try {
                auto p2 = j.get<ns::MapForJsonStruct>();
                if (restart == false) {
                    widthAndHeight.x = p2.mapSize[0];
                    widthAndHeight.y = p2.mapSize[1];
                    tileSize.x = p2.tileSize[0];
                    tileSize.y = p2.tileSize[1];

                    std::cout << " p2 =" << std::endl;
                    for (auto i : p2.map)
                        std::cout << i << ' ';
                    std::cout << ", mapsize = [" << p2.mapSize[0] << "," << p2.mapSize[1] << "]" << std::endl;
                    if (p2.mapSize[0] > 0 || p2.mapSize[1] > 0 || p2.tileSize[0] > 0 || p2.tileSize[1] > 0 || p2.textureForMap.size() > 0) {
                        int i = DrawMap(p2, gui);
                        if (i == -1) {
                            restart = true;
                            std::cout << "Failed to Draw Given Map" << std::endl;
                        }
                    }
                    else {
                        restart = true;
                        std::cout << "File Provided Was Unacceptable." << std::endl;
                    }
                }
            }
            catch (...) {
                std::cout << "Badly Formatted Map Try Again" << std::endl;
                restart = true;
            }
        }
    } while (restart);

    return EXIT_SUCCESS;

};

int DrawMap(ns::MapForJsonStruct ms, tgui::Gui& gui)
{
    //Resets GUI for moving between windows.
    gui.removeAllWidgets();
    // define the level with an array of tile indices
    auto level = ms.get_level();//I know return type is const int*.

    // create the tilemap from the level definition
    TileMap map;

    if (!map.load(ms.textureForMap, tileSize, level, widthAndHeight))
        return -1;




    TileList tileList;
    
    if (!tileList.load(ms.textureForMap, tileSize, 4))
        return -1;
    auto sideBar = tgui::ScrollablePanel::create();
    sideBar->setPosition("82%", "0%");
    sideBar->setSize(320, "100%");
    gui.add(sideBar);
    std::vector<Tile>& tiles = tileList.tiles;
    // Loops through tilelist then each tile to sidebar as a sprite on an SFML canvas.
    for (unsigned int i = 0; i < tiles.size(); ++i)
    {
        sf::Sprite& sprite = tiles[i].m_tile_sprite;
        sprite.setScale(10, 10);
        auto canvas = tgui::CanvasSFML::create();
        canvas->setSize({ 320, 320 });
        canvas->setPosition(0, i * 350);
        sideBar->add(canvas);

        canvas->draw(sprite);
        canvas->display();
    }

    sideBar->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);

    /*
    Describes Map Menu UI Layout.
    */
    auto panel = tgui::Panel::create();
    panel->setSize("70%", "100%");
    gui.add(panel);


    auto scrollCanvas = tgui::ScrollablePanel::create();
    panel->setSize("70%", "70%");
    panel->add(scrollCanvas);

    auto canvas = tgui::CanvasSFML::create();
    canvas->setSize(tileSize.x * widthAndHeight.x, tileSize.y * widthAndHeight.y);
    canvas->setOrigin(0.5f, 0.5f);
    canvas->setPosition("50%", "50%");
    scrollCanvas->add(canvas);

    auto buttonPanel = tgui::Panel::create();
    buttonPanel->setSize("70%", "25%");
    buttonPanel->setPosition("0%", "75%");
    buttonPanel->getRenderer()->setOpacity(0.0f);
    gui.add(buttonPanel);

    auto saveButton = tgui::Button::create("Save Map");
    saveButton->setSize("30%", "5%");
    saveButton->setOrigin(0.5f, 0.5f);
    saveButton->setPosition("35%", "75%");
    gui.add(saveButton);


    auto homeButton = tgui::Button::create("Home Menu");
    homeButton->setSize("30%", "5%");
    homeButton->setOrigin(0.5f, 0.5f);
    homeButton->setPosition("35%", "85%");
    gui.add(homeButton);

    // run the main loop
    while (window.isOpen())
    {
        mouseScreenPosition = sf::Mouse::getPosition(window);
        mouseWorldPosition = window.mapPixelToCoords(mouseScreenPosition);

        // handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            gui.handleEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Handles user clicking on one of the tiles in the sidebar, then assigns this tiles type to the painting tile.
            if (sideBar->isMouseDown()) {
                auto widgets = sideBar->getWidgets();
                for (int j = 0; j < 4; j++) {
                    if (widgets[j]->isMouseDown()) {
                        paintingTile = tiles[j].m_type;
                    }
                }
            }
            // Saves current state of map to output file.
            else if (saveButton->isMouseDown()) {
                ms.map = map.convertToMap(level, tileSize, widthAndHeight);
                json saveFile = ms;
                std::ofstream outJson(savingMapName);
                outJson << saveFile << std::endl;
                outJson.close();
            }
            // Returns to the home menu.
            else if (homeButton->isMouseDown()) {
                restart = true;
                return 1;
            }
            // Handles user clicking on the map canvas.
            else if (canvas->isMouseDown()) {
                int mapWidth = widthAndHeight.x;
                int mapHeight = widthAndHeight.y;

                int tileWidth = tileSize.x;
                int tileHeight = tileSize.y;

                int canvasWidth = tileWidth * mapWidth;
                int canvasHeight = tileHeight * mapHeight;

                auto canvasPos = canvas->getAbsolutePosition();

                int scaledMousePosX = mouseScreenPosition.x - canvasPos.x;
                int scaledMousePosY = mouseScreenPosition.y - canvasPos.y;
                if (scaledMousePosX >= 0 && scaledMousePosY >= 0 && scaledMousePosX < canvasWidth && scaledMousePosY < canvasHeight) {
                    int tileX = (scaledMousePosX / tileWidth);
                    int tileY = (scaledMousePosY / tileHeight);

                    int tileNumber = tileX + tileY * mapWidth;
                    map.changeTile(tileNumber, paintingTile);
                }

            }

        }

        // draw the map
        window.clear();

        canvas->clear();
        canvas->draw(map);
        canvas->display();
        // window.draw(map);
        gui.draw();
        window.display();
    }

    return 0;

    return EXIT_SUCCESS;
}

void updateTextSize(tgui::Gui& gui)
{
    // Update the text size of all widgets in the gui, based on the current window height
    const float windowHeight = gui.getView().getRect().height;
    gui.setTextSize(static_cast<unsigned int>(0.04f * windowHeight)); // 7% of height
}

void loadMap(tgui::EditBox::Ptr mapName)
{
    std::stringstream ss;
    ss << mapName->getText();
    std::string name;
    ss >> name;
    loadingMapName = name;
    menuS = Map;
    mapS = Load;
}

int createMap(tgui::EditBox::Ptr mapName, tgui::EditBox::Ptr mapWidth, tgui::EditBox::Ptr mapHeight, tgui::EditBox::Ptr tileWidth,
    tgui::EditBox::Ptr tileHeight, tgui::EditBox::Ptr mapTexture)
{
    try {
        unsigned int width;
        unsigned int height;
        unsigned int createTileWidth;
        unsigned int createTileHeight;
        std::string name;
        std::string tilesetName;

        std::stringstream ss;
        ss << mapName->getText() << ' ' << mapWidth->getText() << ' ' << mapHeight->getText() << ' ' << tileWidth->getText() << ' ' << tileHeight->getText() << ' ' << mapTexture->getText();
        try {
            ss >> name >>  width >> height >> createTileWidth >> createTileHeight >> tilesetName;
        }
        catch (...) {
            std::cout << "Malformed Input" << std::endl;
            restart = true;
            menuS = Map;
            mapS = Create;
            return 0;
        }
        
        savingMapName = name;

        widthAndHeight.x = width;
        widthAndHeight.y = height;

        tileSize.x = createTileWidth;
        tileSize.y = createTileHeight;

        textureForMap = tilesetName;

        menuS = Map;
        mapS = Create;

        if (!(width > 0 && height > 0 && createTileWidth > 0 && createTileHeight > 0 && name.size() != 0 && tilesetName.size() != 0)) {
            std::cout << "Malformed Input" << std::endl;
            restart = true;
        }
    }
    catch (...) {
        restart = true;
        return 0;
    }

    return 1;
}

void homeScreen(tgui::Gui& gui)
{
    gui.removeAllWidgets();
    // Specify an initial text size instead of using the default value
    updateTextSize(gui);
    // We want the text size to be updated when the window is resized
    gui.onViewChange([&gui] { updateTextSize(gui); });

    auto picture = tgui::Picture::create("background.png");
    picture->setSize({ "100%", "100%" });
    gui.add(picture);

    auto editBoxMapName = tgui::EditBox::create();
    editBoxMapName->setSize({ "60%", "7%" });
    editBoxMapName->setPosition({ "20%", "2%" });
    editBoxMapName->setDefaultText("Map File Name");
    gui.add(editBoxMapName);

    auto loadButton = tgui::Button::create("Load Map");
    loadButton->setSize({ "40%", "10%" });
    loadButton->setPosition({ "30%", "12%" });
    gui.add(loadButton);

    loadButton->onPress(&loadMap, editBoxMapName);

    auto editMapName = tgui::EditBox::create();
    editMapName->setSize({ "60%", "7%" });
    editMapName->setPosition({ "20%", "25%" });
    editMapName->setDefaultText("Map Name");
    gui.add(editMapName);

    auto editMapWidth = tgui::EditBox::create();
    editMapWidth->setSize({ "60%", "7%" });
    editMapWidth->setPosition({ "20%", "35%" });
    editMapWidth->setDefaultText("Map Width");
    gui.add(editMapWidth);

    auto editMapHeight = tgui::EditBox::create();
    editMapHeight->setSize({ "60%", "7%" });
    editMapHeight->setPosition({ "20%", "45%" });
    editMapHeight->setDefaultText("Map Height");
    gui.add(editMapHeight);

    auto editTileWidth = tgui::EditBox::create();
    editTileWidth->setSize({ "60%", "7%" });
    editTileWidth->setPosition({ "20%", "55%" });
    editTileWidth->setDefaultText("Tile Width");
    gui.add(editTileWidth);

    auto editTileHeight = tgui::EditBox::create();
    editTileHeight->setSize({ "60%", "7%" });
    editTileHeight->setPosition({ "20%", "65%" });
    editTileHeight->setDefaultText("Tile Height");
    gui.add(editTileHeight);

    auto editMapTexture = tgui::EditBox::create();
    editMapTexture->setSize({ "60%", "7%" });
    editMapTexture->setPosition({ "20%", "75%" });
    editMapTexture->setDefaultText("Map Texture File Name");
    gui.add(editMapTexture);

    auto createButton = tgui::Button::create("Create Map");
    createButton->setSize({ "40%", "10%" });
    createButton->setPosition({ "30%", "85%" });
    gui.add(createButton);

    createButton->onPress(&createMap,editMapName, editMapWidth, editMapHeight, editTileWidth, editTileWidth, editMapTexture);

    bool finished = false;
    while ((window.isOpen()) && (menuS == Home))
    {
        // handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            gui.handleEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // draw the map
        window.clear();
        // window.draw(map);
        gui.draw();
        window.display();
    }
}
