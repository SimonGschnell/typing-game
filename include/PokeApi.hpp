#ifndef HTTP_REQ_HPP
#define HTTP_REQ_HPP

#include <utility>


#include <SFML/Graphics.hpp>

// curl
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

// json
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace curlpp::options;

namespace PokeApi{
    const int MAX_POKEMON_ID = 1026;
    inline int generatePokemonID(){
        // range from 0 - 1025
        return rand() % MAX_POKEMON_ID;
    }
    std::pair<std::string,std::string> getPokemon(int id);
}

#endif
