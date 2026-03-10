#include <iostream>
#include <bitset>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <sstream>
#include <ostream>
#include <fstream>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/integer/common_factor.hpp>
#include "util.h"
#include "save-file.h"
#include <openssl/applink.c>


using namespace std;
using boost::multiprecision::uint1024_t;
using boost::multiprecision::cpp_int;
using boost::integer::gcd;



typedef boost::multiprecision::number<
    boost::multiprecision::cpp_int_backend<
        2048, 
        2048, 
        boost::multiprecision::unsigned_magnitude, 
        boost::multiprecision::unchecked, 
        void
    >
> uint2048_t;

struct DiceType {
    int sides;
    double multiplier;
};

struct skip_result {
    bool skip;
    uint8_t value;
};


uint1024_t find_nearest_prime(const string& input) {

    uint1024_t num(input);
    while (true) {
        if (miller_rabin_test(num, 25)) {
            return num;
        }
        ++num;
    }
}

skip_result random_skip(std::mt19937& gen, std::uniform_int_distribution<int>& dis) {
    int chance_skip = dis(gen);
    if (chance_skip < 100) {
        return {true, static_cast<uint8_t>(dis(gen))};
    }
    return {false, 0};
}

int roll_dice(int num_dice, const vector<DiceType>& selected_dice, vector<int>& rolls, vector<uint8_t>& buffer, int& byte_index, mt19937& gen, int BUFFER_SIZE) {
        
        bool TESTING = false; // Set to true to enable testing mode with fixed rolls
        if (TESTING) {
            for (int j = 0; j < num_dice; ++j) {
                rolls[j] = 1; // Fixed roll of 1 for testing
            }
            return 0; // Skip the rest of the function in testing mode
        } 
    
        for (int j = 0; j < num_dice; ++j) {
            cout << "Rolling for die with " << selected_dice[j].sides << " sides (die " << (j + 1) << "):\n";
            
            int input;
            if (!(cin >> input)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            rolls[j] = input;
            if (rolls[j] < 1 || rolls[j] > selected_dice[j].sides) {
                cerr << "Invalid roll for die " << (j + 1) << ". Must be between 1 and " << selected_dice[j].sides << ".\n";
                return 1;
            }
        }

        //randomize order of rolls to fill the buffer byte
        shuffle(rolls.begin(), rolls.end(), gen);
        for (int j = 0; j < num_dice && byte_index < BUFFER_SIZE; ++j) {
            int scaled = static_cast<int>((rolls[j] - 1) * selected_dice[j].multiplier + 0.5);
            scaled = std::clamp(scaled, 0, 255);
            buffer[byte_index++] = static_cast<uint8_t>(scaled);
        }
        cout << "Current buffer byte" << byte_index / 8 << ": " << bitset<8>(buffer[byte_index-1]) << "\n\n";
    
    return 0;

}

uint1024_t start_rolls(int num_dice, vector<DiceType>& selected_dice, int BUFFER_SIZE, vector<uint8_t>& buffer, std::mt19937& gen, std::uniform_int_distribution<int>& dis) {
    
    stringstream ss = stringstream();
    int byte_index = 0;
    while (byte_index < BUFFER_SIZE) {

        vector<int> rolls(num_dice);

        skip_result skip_result = random_skip(gen, dis);
        if (skip_result.skip) {
            buffer[byte_index++] = skip_result.value;
            continue;
        }

        roll_dice(num_dice, selected_dice, rolls, buffer, byte_index, gen, BUFFER_SIZE);

    }

    cout << "\nBuffer bytes in binary (one blob):\n";
    for (int byte_index = 0; byte_index < BUFFER_SIZE; ++byte_index) {
        ss << bitset<8>(buffer[byte_index]);
        cout << bitset<8>(buffer[byte_index]) << " ";

    }
    
    // Sanity check: verify exactly 1024 bits of 0s and 1s
    string binary_str = ss.str();
    if (binary_str.length() != 1024) {
        cerr << "Error: Expected 1024 bits, got " << binary_str.length() << "\n";
        return 1;
    }
    for (int counter = 0; counter < (int)binary_str.length(); ++counter) {
        char c = binary_str[counter];
        if (c != '0' && c != '1') {
            cerr << "Error: Invalid character '" << c << "' at position " << counter << " in binary string. Only 0s and 1s allowed.\n";
            return 1;
        }
    }
    
    try
    {
        uint1024_t prime = find_nearest_prime(binary_str);
        cout << "\n\nNearest prime: " << prime << "\n";
        return prime;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

int initialize(int BUFFER_SIZE, vector<DiceType>& selected_dice, const map<string, DiceType>& dice_catalog) {
    cout << "How many dice?";
    int num_dice;
    cin >> num_dice;

    if (num_dice <= 0 || num_dice > BUFFER_SIZE) {
        cerr << "Invalid number of dice. Must be between 1 and " << BUFFER_SIZE << ".\n";
        return 1;
    }

    int byte_size = (num_dice*8) + 1;
    cout << "You have " << num_dice << " dice. Each die can be rolled to fill one byte (0-255) in the buffer.\n";

    cout << "\n\n";
    cout << "now enter the dice type (d4, d6, d8, d10, d12, d20)" << "\n\n";

    for (int byte_index = 0; byte_index < num_dice; ++byte_index) {
        cout << "Die " << (byte_index + 1) << " type: ";
        string die_name;
        if (!(cin >> die_name)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            std::cerr << "Invalid input for die " << (byte_index + 1) << ".\n";
            return 1;
        }

        auto it = dice_catalog.find(die_name);
        if (it == dice_catalog.end()) {
            std::cerr << "Invalid die type for die " << (byte_index + 1) << ". Use d4, d6, d8, d10, d12, or d20.\n";
            return 1;
        }

        selected_dice.push_back(it->second);
    }

    return num_dice;
}

int main() {
    const int BUFFER_SIZE = 128;
    vector<uint8_t> buffer(BUFFER_SIZE);
    vector<DiceType> selected_dice;


    random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);


    const map<string, DiceType> dice_catalog = {
        {"d4", {4, 255.0 / (4 - 1)}},
        {"d6", {6, 255.0 / (6 - 1)}},
        {"d8", {8, 255.0 / (8 - 1)}},
        {"d10", {10, 255.0 / (10 - 1)}},
        {"d12", {12, 255.0 / (12 - 1)}},
        {"d20", {20, 255.0 / (20 - 1)}}
    };

    int num_dice = initialize(BUFFER_SIZE, selected_dice, dice_catalog);
    if (num_dice <= 0) {
        return 1;
    }

    uint1024_t primeq = start_rolls(num_dice, selected_dice, BUFFER_SIZE, buffer, gen, dis);
    uint1024_t primep = start_rolls(num_dice, selected_dice, BUFFER_SIZE, buffer, gen, dis);

    if (primeq == 1 || primep == 1) {
        cerr << "Error during prime generation.\n";
        return 1;
    }

    cpp_int mod = primep * primeq;
    cout << "\n\nModulus (p*q): " << mod << "\n";

    cpp_int lambda = carmichael_fixed(primep, primeq);
    cout << "\n\nCarmichael Lambda: " << lambda << "\n";

    cpp_int e=65537; // Common choice for public exponent

    // Ensure e and lambda are coprime
    if (gcd(e, lambda) != 1) {
        cerr << "Error: e and lambda are not coprime. Choose different primes.\n";
        return 1;
    }

    // Compute d, the modular inverse of e mod lambda
    cpp_int d = mod_inverse(e, lambda);
    cout << "\n\nPrivate exponent d: " << d << "\n";


    //convert to base64 and save to file as the private key
    stringstream ss;
    ss << hex << d;
    string private_key_hex = ss.str();

    try {
        save_pem(mod, e, d, primep, primeq);
    } catch (const std::exception& ex) {
        cerr << "Error saving PEM file: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}