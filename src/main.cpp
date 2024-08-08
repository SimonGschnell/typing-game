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

enum class GameState{
    Normal,
    Paused,
    GameOver,
};


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

void prepareSubscriberForPublisher(Publisher &pub, std::pair<std::string,std::string> pokemon, sf::Font &font, const sf::Window &window, int row){
    // create GameText / set its position and subscribe it to the Observer
    try{
        std::shared_ptr<GameText> gt {std::make_shared<GameText>(GameText{pokemon.first, font, DEFAULT_CHARACTER_SIZE,pokemon.second,row})};
        gt->setPosition({50,generateRow(gt->getRow(),window)});
        pub.subscribe(gt->getString(),gt);
    }catch(std::exception &e){
        std::cerr << "Failed to create GameText: " << e.what();
    }
}

//todo: refactor
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


    // player
    sf::Texture avatarTexture;
    loadRecourse(avatarTexture,"./textures/litleo.png");

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
    window.setVerticalSyncEnabled( true );


    float rotation{1};
    int rounds{0};
    int index{0};

    sf::Clock clock;
    Publisher pub;

    std::future_status f_status;
    std::future<std::pair<std::string,std::string>> future = std::async(PokeApi::getPokemon,PokeApi::generatePokemonID());

    sf::Time elapsed;

    sf::Clock dtClock;
    sf::Time time;
    float deltaTime=0;

    GameState game_state{GameState::Normal};

    // PAUSE - Graphics
    sf::RectangleShape pause_bar{{window.getSize().x,window.getSize().y/2}};
    pause_bar.setOrigin({pause_bar.getSize().x/2,pause_bar.getSize().y/2});
    pause_bar.setPosition({window.getSize().x/2,window.getSize().y/2});
    pause_bar.setFillColor(sf::Color{0,0,0,125});

    sf::Text pause_text{"Game Paused",font,100};
    pause_text.setOrigin({pause_text.getGlobalBounds().width/2,pause_text.getGlobalBounds().height/2});
    pause_text.setPosition({window.getSize().x/2,window.getSize().y/2});
    pause_text.setFillColor(sf::Color::White);

    //GAME OVER - Graphics
    sf::Text game_over_continue{"continue",font,70};
    game_over_continue.setPosition({pause_text.getGlobalBounds().left - 50,pause_text.getGlobalBounds().top + 100});
    sf::Text game_over_quit{"quit",font,70};
    game_over_quit.setPosition({pause_text.getGlobalBounds().left + 380,pause_text.getGlobalBounds().top + 100});


    //background
    sf::Texture bg_texture;
    loadRecourse(bg_texture,"./textures/grass.jpg");

    sf::Sprite bg_sprite;
    bg_sprite.setTexture(bg_texture);
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u textureSize = bg_texture.getSize();
    bg_sprite.setScale((float) windowSize.x / textureSize.x, (float) windowSize.y / textureSize.y);

    while (window.isOpen())
    {
        elapsed = clock.getElapsedTime();

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
                case sf::Event::LostFocus:
                    game_state = GameState::Paused;
                    break;
                case sf::Event::GainedFocus:
                    if(game_state != GameState::GameOver){
                        game_state = GameState::Normal;
                    }
                    break;
                case sf::Event::MouseButtonPressed:
                    if(game_over_continue.getGlobalBounds().contains({event.mouseButton.x,event.mouseButton.y})){
                        game_state = GameState::Normal;
                    }
                    if(game_over_quit.getGlobalBounds().contains({event.mouseButton.x,event.mouseButton.y})){
                        window.close();
                    }
                    break;
            }

        }

        // Restart the clock and get the delta time
        time = dtClock.restart();
        deltaTime = time.asSeconds();

        window.clear();
        window.draw(bg_sprite);


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
        */

        if(game_state == GameState::Normal && elapsed.asSeconds() >= 2.f && future.valid()){

            switch (f_status = future.wait_for(std::chrono::seconds(0)); f_status)
            {
                case std::future_status::deferred:
                    break;
                case std::future_status::timeout:
                    break;
                case std::future_status::ready:
                    if(int row = checkForEmptyRow(pub); row){
                        std::cout << row << " this is the row"<< std::endl;
                        prepareSubscriberForPublisher(pub, future.get(), font, window, row);
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

                switch (game_state)
                {
                    case GameState::Normal:
                        t->move({400*deltaTime,0.f});
                        break;
                    case GameState::Paused:
                        window.draw(pause_bar);
                        window.draw(pause_text);
                        break;
                }

                if(window.getSize().x <= t->getPosition().x){
                    game_state = GameState::GameOver;
                    break;
                }

                window.draw(*t);
            }
        }

        if(game_state == GameState::GameOver){
            if(pub.getSubscribers().size() > 0){
                pub.unsubscribeAll();
            }
            window.draw(pause_bar);
            pause_text.setString("Game Over");
            window.draw(pause_text);
            window.draw(game_over_continue);
            window.draw(game_over_quit);
        }

        window.display();
    }
}
