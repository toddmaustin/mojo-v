#ifndef RATING_HPP
#define RATING_HPP

#include <string>

class Rating {

public:
    Rating();
    Rating(size_t user, size_t movie, VIP_ENCDOUBLE mark);
    ~Rating();

    size_t getUser();
    size_t getMovie();
    VIP_ENCDOUBLE getMark();

    void setUser(size_t user);
    void setMovie(size_t movie);
    void setMark(VIP_ENCDOUBLE mark);

    std::string toString();

private:
    size_t user;
    size_t movie;
    VIP_ENCDOUBLE mark;

};

#endif
