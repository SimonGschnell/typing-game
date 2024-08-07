#include "../include/PokeApi.hpp"
#include <fstream>

namespace PokeApi{
    std::pair<std::string,std::string> getPokemon(int id){
        try
        {
            // That's all that is needed to do cleanup of used resources (RAII style).
            curlpp::Cleanup myCleanup;

            // Our request to be sent.
            curlpp::Easy myRequest;

            // Set the URL.
            myRequest.setOpt<Url>("https://pokeapi.co/api/v2/pokemon/"+std::to_string(id));

            // Send request and get a result.
            // By default the result goes to standard output.

            std::ostringstream os;
            curlpp::options::WriteStream ws(&os);
            myRequest.setOpt(ws);
            myRequest.perform();

            auto data = json::parse(os.str());
            std::string pokeName{data["name"]};
            std::cout << "fetched pokemon: Name => " << pokeName << " | id => " << data["id"] << std::endl;

            // get the pokemon sprite image
            std::string imgUrl= data["sprites"]["other"]["official-artwork"]["front_default"] ;
            myRequest.setOpt<Url>(imgUrl);
            std::ostringstream imgData;
            curlpp::options::WriteStream imgDataWs(&imgData);
            myRequest.setOpt(imgDataWs);
            myRequest.perform();

            std::string imageData = imgData.str();

            sf::Image image;
            if (!image.loadFromMemory(imageData.data(), imageData.size())) {
                std::cerr << "Failed to load image from memory\n";
            }

            std::string texturePath{"./textures/"+pokeName+".png"};
            std::ifstream file{texturePath};
            if(!file.good()){
                if(!image.saveToFile(texturePath)){
                    std::cerr << "Failed to save image to File\n";
                }
            }

            return {pokeName,texturePath};

        }

        catch(curlpp::RuntimeError & e)
        {
            std::cout << e.what() << std::endl;
        }

        catch(curlpp::LogicError & e)
        {
            std::cout << e.what() << std::endl;
        }

    }
}