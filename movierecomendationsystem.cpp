#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype> // for std::tolower

// Define a simple Movie class
struct Movie {
    std::string title;
    int year;

    bool operator==(const Movie& other) const {
        return title == other.title && year == other.year;
    }
};

// Custom hash function for Movie class
struct MovieHash {
    size_t operator()(const Movie& movie) const {
        return std::hash<std::string>()(movie.title) ^ std::hash<int>()(movie.year);
    }
};

// Define a User class
class User {
public:
    int id; // Optional user ID
    std::string name;
    std::unordered_map<Movie, int, MovieHash> ratings;

    // Function to add a rating for a movie
    void addRating(const Movie& movie, int rating) {
        ratings[movie] = rating;
    }
};

// Function to calculate similarity between two users
double calculateSimilarity(const User& user1, const User& user2) {
    double similarity = 0.0;
    double user1_magnitude = 0.0, user2_magnitude = 0.0;

    for (const auto& entry : user1.ratings) {
        Movie movie = entry.first;
        int rating1 = entry.second;

        auto it = user2.ratings.find(movie);
        if (it != user2.ratings.end()) {
            int rating2 = it->second;
            similarity += rating1 * rating2;
        }

        user1_magnitude += pow(rating1, 2);
    }

    for (const auto& entry : user2.ratings) {
        user2_magnitude += pow(entry.second, 2);
    }

    return similarity / (std::sqrt(user1_magnitude) * std::sqrt(user2_magnitude));
}

// Function to recommend movies for a user
std::vector<Movie> recommendMovies(const User& user, const std::unordered_map<Movie, int, MovieHash>& movies) {
    std::vector<Movie> recommendations;
    std::vector<std::pair<Movie, double>> similarities; // Movie and its similarity score

    // Calculate similarity with all movies
    for (const auto& movie_entry : movies) {
        Movie movie = movie_entry.first;
        if (user.ratings.count(movie) == 0) { // Don't recommend already rated movies
            User tempUser;
            tempUser.addRating(movie, 0);
            double similarity = calculateSimilarity(user, tempUser); // Create a temporary user with the movie and a rating of 0
            similarities.push_back({movie, similarity});
        }
    }

    // Sort movies by similarity in descending order
    std::sort(similarities.begin(), similarities.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    // Select top N recommendations (modify N as needed)
    size_t num_recommendations = 5;
    for (size_t i = 0; i < num_recommendations && i < similarities.size(); ++i) {
        recommendations.push_back(similarities[i].first);
    }

    return recommendations;
}

// Function to load movie data from a CSV file
void loadMovies(std::unordered_map<Movie, int, MovieHash>& movies, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::cout << "Loading movies from file: " << filename << std::endl;

    std::string line;
    // Skip header line (assuming there's a header)
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string title, year_str;

        // Extract movie title and year (assuming comma separated values)
        std::getline(ss, title, ',');
        std::getline(ss, year_str, ',');

        int year = std::stoi(year_str);
        movies.insert({{title, year}, 0});
    }
}

// Function to load user data from a CSV file
void loadUsers(std::vector<User>& users, const std::unordered_map<Movie, int, MovieHash>& movies, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::cout << "Loading users from file: " << filename << std::endl;

    std::string line;
    // Skip header line (assuming there's a header)
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string name, movie_title, rating_str;

        // Extract user name, movie title, and rating (assuming comma separated values)
        std::getline(ss, name, ',');
        std::getline(ss, movie_title, ',');
        std::getline(ss, rating_str, ',');

        int rating = std::stoi(rating_str);
        auto it = std::find_if(movies.begin(), movies.end(), [&movie_title](const auto& pair) {
            return pair.first.title == movie_title;
        });

        if (it != movies.end()) {
            Movie movie = it->first;

            // Find or create user
            auto it_user = std::find_if(users.begin(), users.end(), [&name](const User& user) {
                return user.name == name;
            });

            if (it_user != users.end()) {
                it_user->addRating(movie, rating);
            } else {
                User new_user;
                new_user.name = name;
                new_user.addRating(movie, rating);
                users.push_back(new_user);
            }
        }
    }
}

// Convert string to lowercase and trim whitespace
std::string toLowerCaseAndTrim(const std::string& str) {
    std::string lower_str = str;
    lower_str.erase(lower_str.begin(), std::find_if(lower_str.begin(), lower_str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    lower_str.erase(std::find_if(lower_str.rbegin(), lower_str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), lower_str.end());
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

int main() {
    std::unordered_map<Movie, int, MovieHash> movies;
    std::vector<User> users;

    // Use absolute paths or ensure files are in the correct directory
    std::string movies_file = "C:\\Users\\anant\\Desktop\\MovieRecommendationSystemusingcpp\\movies.csv";
    std::string ratings_file = "C:\\Users\\anant\\Desktop\\MovieRecommendationSystemusingcpp\\ratings.csv";

    // Load movies and user ratings from CSV files
    loadMovies(movies, movies_file);
    loadUsers(users, movies, ratings_file);

    // User interaction loop
    while (true) {
        std::string username;
        std::cout << "Enter username (or 'exit' to quit): ";
        std::getline(std::cin, username);
        if (username == "exit") {
            break;
        }

        std::string normalized_username = toLowerCaseAndTrim(username);

        // Find user by name (case insensitive)
        auto it = std::find_if(users.begin(), users.end(), [&normalized_username](const User& user) {
            return toLowerCaseAndTrim(user.name) == normalized_username;
        });

        if (it == users.end()) {
            std::cout << "User not found.\n";
            continue;
        }

        // Show recommendations
        std::vector<Movie> recommendations = recommendMovies(*it, movies);
        if (recommendations.empty()) {
            std::cout << "No recommendations available.\n";
        } else {
            std::cout << "Recommended movies for " << username << ":\n";
            for (const Movie& movie : recommendations) {
                std::cout << movie.title << std::endl;
            }
        }
    }

    return 0;
}
