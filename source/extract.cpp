#include "../include/extract.hpp"
#include "unzipper.h"

bool extract(std::string filename){
    chdir("/");
    zipper::Unzipper unzipper(filename);
    bool res = unzipper.extract();
    unzipper.close();
    return res;
}