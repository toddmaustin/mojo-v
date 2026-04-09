#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#include "../config.h"
#include "parser.hpp"

Parser::Parser(char *filename, char delimiter) {
    this->filename = filename;
    this->delimiter = delimiter;
}

Parser::~Parser() {
    for (Rating *rating: this->ratings) {
        delete rating;
    }

    this->ratings.clear();
    this->trainRatings.clear();
    this->testRatings.clear();
}

size_t Parser::getUserNb() {
    return this->userNb;
}

size_t Parser::getMovieNb() {
    return this->movieNb;
}

std::vector<Rating*> Parser::getRatings() {
    return this->ratings;
}

std::vector<Rating*> Parser::getTrainRatings() {
    return this->trainRatings;
}

std::vector<Rating*> Parser::getTestRatings() {
    return this->testRatings;
}

size_t Parser::getTrainRatingsNb() {
    return this->trainRatings.size();
}

size_t Parser::getTestRatingsNb() {
    return this->testRatings.size();
}

std::string Parser::toString() {
    std::string result;

    // Ratings to strings
    for (Rating* rating: this->ratings) {
        result += rating->toString();
        result += "\n";
    }

    // User and movie counters
    result += "Users: " + std::to_string(this->userNb)
            + "\nMovies: " + std::to_string(this->movieNb);

    // Return result string
    return result;
}

void Parser::readCsv() {
    // Open CSV file
    std::ifstream file(this->filename);
    std::string line;

    // Initialize user and movie counters
    this->userNb = 0;
    this->movieNb = 0;

    // Skip first line (header)
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;

        Rating *rating = new Rating();

        // User id
        std::getline(ss, value, this->delimiter);
        rating->setUser(atoi(value.c_str()));

        // Movie id
        std::getline(ss, value, this->delimiter);
        rating->setMovie(atoi(value.c_str()));

        // Mark
        std::getline(ss, value, this->delimiter);
        rating->setMark(atof(value.c_str()));

        // Compute max user id
        if (this->userNb < rating->getUser()) {
            this->userNb = rating->getUser();
        }

        // Compute max movie id
        if (this->movieNb < rating->getMovie()) {
            this->movieNb = rating->getMovie();
        }

        // Push rating to data
        this->ratings.push_back(rating);
    }

    // Close the input file
    file.close();
}

void Parser::splitTrainTestRatings(double ratio, int slice) {
    // Count ratings per user
    size_t *userRatingsNb = new size_t[this->userNb]();
    for (Rating *rating: this->ratings) {
        ++userRatingsNb[rating->getUser() - 1];
    }

    // Clear old rating vectors
    this->trainRatings.clear();
    this->testRatings.clear();

    // Current user id
    size_t currentUser = 0;

    // Rating per user counter
    size_t ratingCounter = 0;

    // Slice bound
    int sliceBound = 0;
    for (Rating *rating: this->ratings) {
        // Next user id
        if (currentUser < rating->getUser()) {
            sliceBound = std::round(userRatingsNb[currentUser] * (1 - ratio));
            ++currentUser;
            ratingCounter = 0;
        }

        // Split between training and testing datasets
        if (ratingCounter >= (size_t)(slice * sliceBound)
                && ratingCounter < (size_t)((slice + 1) * sliceBound)) {
            this->testRatings.push_back(rating);
        } else {
            this->trainRatings.push_back(rating);
        }

        // Increase rating per user counter
        ++ratingCounter;
    }

    // Free memory
    delete [] userRatingsNb;
}

VIP_ENCDOUBLE** Parser::ratingsMatrix() {
    VIP_ENCDOUBLE **ratings = new VIP_ENCDOUBLE*[this->userNb];
    for (size_t i = 0; i < this->userNb; ++i) {
        ratings[i] = new VIP_ENCDOUBLE[this->movieNb]();
    }

    // Populate the matrix with ratings
    for (Rating *r: this->trainRatings) {
        ratings[r->getUser() - 1][r->getMovie() - 1] = r->getMark();
    }

    // Return ratings matrix
    return ratings;
}

void Parser::writeResultsFile(const char *filename, Predictor *predictor) {
    // Open output file
    std::ofstream file(filename, std::ofstream::out | std::ofstream::trunc);

    // Iterate through test ratings
    for (Rating *rating: this->testRatings) {
        // Compute prediction
        VIP_ENCDOUBLE expected = rating->getMark();
        VIP_ENCDOUBLE predicted = predictor->predict(
                    rating->getUser() - 1, rating->getMovie() - 1);

        // Write rating result
        std::string result = rating->toString()
                + "\t expected: " + std::to_string(VIP_DEC(expected))
                + "\t predicted: " + std::to_string(VIP_DEC(predicted))
                + "\n";
        file << result;
    }

    // Close output file
    file.close();
}
