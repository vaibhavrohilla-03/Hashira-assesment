#include <iostream>
#include <vector>
#include <complex>
#include <fstream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

std::vector<std::complex<double>> computeCoefficientsFromRoots(
    const std::vector<std::complex<double>>& roots) {

    int ndeg = roots.size();
    if (ndeg == 0) {
        return { 1.0 };
    }

    std::vector<std::complex<double>> coeffs(ndeg + 1, 0.0);
    coeffs[0] = 1.0;

    for (int i = 0; i < ndeg; ++i) {
        std::complex<double> z_i = roots[i];
        for (int j = i; j >= 0; --j) {
            coeffs[j + 1] = coeffs[j + 1] - coeffs[j] * z_i;
        }
    }

    return coeffs;
}

long long stringToLong(const std::string& str, int base) {
    long long result = 0;
    long long power = 1;
    for (int i = str.length() - 1; i >= 0; i--) {
        int digit = (str[i] >= '0' && str[i] <= '9') ? (str[i] - '0') : 0;
        if (digit >= base) {
            std::cerr << "Error: Digit '" << str[i] << "' invalid for base " << base << std::endl;
            return 0;
        }
        result += digit * power;
        power *= base;
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.json>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    try {
        json data = json::parse(file);
        int k = data["keys"]["k"];
        int degree_m = k - 1;

        if (degree_m <= 0) {
            std::cerr << "Error: Degree of polynomial must be > 0." << std::endl;
            return 1;
        }

        std::vector<std::complex<double>> roots;
        std::cout << "Parsing roots for a polynomial of degree " << degree_m << "..." << std::endl;

        for (int i = 1; i <= degree_m; ++i) {
            std::string key = std::to_string(i);
            if (!data.contains(key)) {
                std::cerr << "Error: Missing root with key '" << key << "'." << std::endl;
                return 1;
            }
            int base = std::stoi(data[key]["base"].get<std::string>());
            std::string value_str = data[key]["value"];
            long long root_val = stringToLong(value_str, base);
            roots.push_back({ (double)root_val, 0.0 });
            std::cout << "  - Root " << i << ": " << root_val << std::endl;
        }

        std::vector<std::complex<double>> coefficients = computeCoefficientsFromRoots(roots);

        std::cout << "\nComputed Polynomial Coefficients:" << std::endl;
        for (size_t i = 0; i < coefficients.size(); ++i) {
            std::cout << "  - c[" << i << "] (for z^" << degree_m - i << "): " << coefficients[i] << std::endl;
        }

        std::complex<double> c = coefficients.back();

        std::cout << "\n----------------------------------------" << std::endl;
        std::cout << "The calculated constant term 'c' is: " << c.real() << std::endl;
        std::cout << "----------------------------------------" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}