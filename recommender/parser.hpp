#ifndef PARSER_HPP
#define PARSER_HPP

#include "rating.hpp"
#include "predictor.hpp"

#include <string>
#include <vector>

class Parser {

public:
    Parser(char *filename, char delimiter);
    ~Parser();

    size_t getUserNb();
    size_t getMovieNb();

    std::vector<Rating*> getRatings();
    std::vector<Rating*> getTrainRatings();
    std::vector<Rating*> getTestRatings();

    size_t getTrainRatingsNb();
    size_t getTestRatingsNb();

    std::string toString();

    void readCsv();
    void splitTrainTestRatings(double ratio, int slice);
    VIP_ENCDOUBLE** ratingsMatrix();

    void writeResultsFile(const char *filename, Predictor *predictor);

private:
    char *filename;
    char delimiter;

    std::vector<Rating*> ratings;
    std::vector<Rating*> trainRatings;
    std::vector<Rating*> testRatings;

    size_t userNb;
    size_t movieNb;
};

#endif
