#include <SFML/Graphics.hpp>
#include <iostream>
#include <exception>
#include <vector>
#include <memory>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <cstdlib>
#include <thread>
#include <future>

#include "../include/Observer.hpp"
#include "../include/GameText.hpp"
#include "../include/PokeApi.hpp"

// constants
const int DEFAULT_CHARACTER_SIZE = 30;
const int MAX_NR_ROWS = 5;


void loadRecourse(sf::Texture &t, std::string file) {
    if(!t.loadFromFile(file)){
        throw std::runtime_error("was not able to load file " + file);
    }
}
void loadRecourse(sf::Font &t, std::string file) {
    if(!t.loadFromFile(file)){
        throw std::runtime_error("was not able to load file " + file);
    }
}

float generateRow(int nrRow, const sf::Window &window){
    auto [width, height] = window.getSize();
    float offset = height / (MAX_NR_ROWS+1);
    std::cout << "height: " << height << " - offset: "<< offset << std::endl;
    return (offset*nrRow)-DEFAULT_CHARACTER_SIZE;
}

void prepareSubscriberForPublisher(Publisher &pub, std::string pokemonName, sf::Font &font, const sf::Window &window, int row){
    // create GameText / set its position and subscribe it to the Observer
    try{
        std::shared_ptr<GameText> gt {std::make_shared<GameText>(GameText{pokemonName, font, DEFAULT_CHARACTER_SIZE,row})};
        gt->setPosition({50,generateRow(gt->getRow(),window)});
        pub.subscribe(gt->getString(),gt);
    }catch(std::exception &e){
        std::cerr << "Failed to create GameText: " << e.what();
    }
}

int checkForEmptyRow(Publisher &pub){
    // all rows empty
    if(pub.getSubscribers().size() == 0){
        return 1;
    }
    // searching for empty row
    std::vector<short> rows {1,2,3,4,5};
    std::vector<short> filtered_rows(10);
    for(auto &sub : pub.getSubscribers()){
        std::shared_ptr<GameText> gt = std::dynamic_pointer_cast<GameText>(sub.second);
        filtered_rows.push_back(gt->getRow());
    }
    bool found=false;
    for(short &r : rows){
        found = false;
        for(short &fr: filtered_rows){
            if(fr==r){
                found = true;
                break;
            }
        }
        if(!found){
            return r;
        }
    }

    // searching for a row with enough space
    for(auto &sub : pub.getSubscribers()){
        std::shared_ptr<GameText> gt = std::dynamic_pointer_cast<GameText>(sub.second);
        sf::FloatRect rect = gt->getGlobalBounds();
        if(rect.left > rect.width*2){
            return gt->getRow();
        }
    }
    return 0;
}

int main()
{
    // fetching pokemon names
    //std::string test{PokeApi::getPokemon(PokeApi::generatePokemonID()).first};

    //background
    sf::Texture oldPaper;
    loadRecourse(oldPaper,"./textures/oldPaper.png");
    oldPaper.setSmooth(true);

    sf::Sprite sprite;
    sprite.setTexture(oldPaper);

    // player
    sf::Texture avatarTexture;
    loadRecourse(avatarTexture,"./textures/guy.png");

    sf::Sprite avatarSprite;
    avatarSprite.setTexture(avatarTexture);
    sf::Vector2f originalAvatarScale{avatarSprite.getScale()};
    avatarSprite.setOrigin(avatarTexture.getSize().x/2,avatarTexture.getSize().y/2);

    // font
    sf::Font font{};
    loadRecourse(font,"./fonts/roboto.tff");

    // text
    sf::Text text{"hello world", font, DEFAULT_CHARACTER_SIZE};
    text.setColor(sf::Color::Red);

    auto window = sf::RenderWindow{ { 300, 300 }, "Typing Game", sf::Style::Fullscreen | sf::Style::Resize | sf::Style::Close};

    window.setFramerateLimit(144);

    // enlarge the sprite to fit the window size
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u textureSize = oldPaper.getSize();
    sprite.setScale((float) windowSize.x / textureSize.x, (float) windowSize.y / textureSize.y);

    float rotation{1};
    int rounds{0};
    int index{0};

    sf::Clock clock;
    Publisher pub;

    std::future_status f_status;
    std::future<std::pair<std::string,int>> future = std::async(PokeApi::getPokemon,PokeApi::generatePokemonID());

    while (window.isOpen())
    {
        sf::Time elapsed = clock.getElapsedTime();

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            switch(event.type){
                case sf::Event::Closed:
                    window.close(); break;
                case sf::Event::TextEntered:
                    pub.notify_subscribers(event.text.unicode);
                    break;
                case sf::Event::Resized:
                    for(auto pair : pub.getSubscribers()){
                        std::shared_ptr<GameText> t {std::dynamic_pointer_cast<GameText>(pair.second)};
                        if(!t) throw std::bad_cast();
                        t->setPosition(t->getPosition().x,generateRow(t->getRow(),window));
                    }
                    break;
            }

        }

        window.clear();

        //drawing happens here
        /* window.draw(sprite);
        avatarSprite.move(sf::Vector2f(1.f,1.f));

        // do one 360 spin

        if(avatarSprite.getRotation() == 0.f){
            if(rounds > 0){
                rotation = 0.f;
            }
            rounds++;
        }

        if(avatarSprite.getScale().x < originalAvatarScale.x*1.5 && avatarSprite.getScale().y < originalAvatarScale.y*1.5 ){
            avatarSprite.scale(1.001f,1.001f);
        }

        avatarSprite.rotate(rotation);
        window.draw(avatarSprite); */

        if(elapsed.asSeconds() >= 2.f && future.valid()){

            switch (f_status = future.wait_for(std::chrono::seconds(0)); f_status)
            {
                case std::future_status::deferred:
                    break;
                case std::future_status::timeout:
                    break;
                case std::future_status::ready:
                    if(int row = checkForEmptyRow(pub); row){
                        std::cout << row << " this is the row"<< std::endl;
                        prepareSubscriberForPublisher(pub, future.get().first, font, window, row);
                        // fetch another pokemon
                        future = std::async(PokeApi::getPokemon,PokeApi::generatePokemonID());
                    }
                    break;
                default:
                    break;
            }

            clock.restart();
        }

        for(auto pair : pub.getSubscribers()){
            std::shared_ptr<GameText> t {std::dynamic_pointer_cast<GameText>(pair.second)};
            if(!t) throw std::bad_cast();

            if(t->isCompleted()){
                pub.unsubscribe(t->getString().toAnsiString());
            }else{
                window.draw(*t);
                t->move({0.5f,0.f});

                // draws outline
                auto bounds = t->getGlobalBounds();
                // Create a RectangleShape with the same position and size
                sf::RectangleShape rectangle(sf::Vector2f(bounds.width, bounds.height));
                rectangle.setPosition(bounds.left, bounds.top);

                // Set the outline color and thickness
                rectangle.setFillColor(sf::Color::Transparent);
                rectangle.setOutlineColor(sf::Color::Red);
                rectangle.setOutlineThickness(1.f);

                window.draw(rectangle);
                rectangle.move({0.5f,0.f});
            }
        }


        window.display();
    }
}
