#include <string>

#include "../config.h"
#include "rating.hpp"

Rating::Rating() {}

Rating::Rating(size_t user, size_t movie, VIP_ENCDOUBLE mark) {
    this->user = user;
    this->movie = movie;
    this->mark = mark;
}

Rating::~Rating() {}

size_t Rating::getUser() {
    return this->user;
}

size_t Rating::getMovie() {
    return this->movie;
}

VIP_ENCDOUBLE Rating::getMark() {
    return this->mark;
}

void Rating::setUser(size_t user) {
    this->user = user;
}

void Rating::setMovie(size_t movie) {
    this->movie = movie;
}

void Rating::setMark(VIP_ENCDOUBLE mark) {
    this->mark = mark;
}

std::string Rating::toString() {
    return "[ "
            + std::to_string(this->user) + " , "
            + std::to_string(this->movie) + " , "
            + std::to_string(VIP_DEC(this->mark)) + " ]";
}
