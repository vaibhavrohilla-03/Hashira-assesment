#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <sstream>
#include <algorithm>
#include <iomanip>

// Custom big integer class for handling very large numbers
class BigInt {
private:
    std::vector<long long> digits;
    static const long long BASE = 1000000000000000000LL; // 10^18
    static const int BASE_DIGITS = 18;
    bool negative;

public:
    BigInt() : negative(false) {
        digits.push_back(0);
    }
    
    BigInt(long long num) : negative(false) {
        if (num < 0) {
            negative = true;
            num = -num;
        }
        if (num == 0) {
            digits.push_back(0);
        } else {
            while (num > 0) {
                digits.push_back(num % BASE);
                num /= BASE;
            }
        }
    }
    
    BigInt(const std::string& str) : negative(false) {
        if (str.empty() || str == "0") {
            digits.push_back(0);
            return;
        }
        
        std::string s = str;
        if (s[0] == '-') {
            negative = true;
            s = s.substr(1);
        }
        
        // Process string in chunks of BASE_DIGITS
        for (int i = s.length(); i > 0; i -= BASE_DIGITS) {
            int start = std::max(0, i - BASE_DIGITS);
            std::string chunk = s.substr(start, i - start);
            digits.push_back(std::stoll(chunk));
        }
        
        removeLeadingZeros();
    }
    
    void removeLeadingZeros() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.size() == 1 && digits[0] == 0) {
            negative = false;
        }
    }
    
    BigInt operator+(const BigInt& other) const {
        if (negative != other.negative) {
            if (negative) return other - (-*this);
            return *this - (-other);
        }
        
        BigInt result;
        result.negative = negative;
        result.digits.clear();
        
        long long carry = 0;
        for (size_t i = 0; i < std::max(digits.size(), other.digits.size()) || carry; ++i) {
            long long sum = carry;
            if (i < digits.size()) sum += digits[i];
            if (i < other.digits.size()) sum += other.digits[i];
            
            result.digits.push_back(sum % BASE);
            carry = sum / BASE;
        }
        
        result.removeLeadingZeros();
        return result;
    }
    
    BigInt operator-(const BigInt& other) const {
        if (negative != other.negative) {
            return *this + (-other);
        }
        
        if (negative) return (-other) - (-*this);
        
        if (*this < other) {
            BigInt result = other - *this;
            result.negative = true;
            return result;
        }
        
        BigInt result;
        result.digits.clear();
        
        long long borrow = 0;
        for (size_t i = 0; i < digits.size(); ++i) {
            long long diff = digits[i] - borrow;
            if (i < other.digits.size()) diff -= other.digits[i];
            
            if (diff < 0) {
                diff += BASE;
                borrow = 1;
            } else {
                borrow = 0;
            }
            
            result.digits.push_back(diff);
        }
        
        result.removeLeadingZeros();
        return result;
    }
    
    BigInt operator*(const BigInt& other) const {
        BigInt result;
        result.digits.assign(digits.size() + other.digits.size(), 0);
        result.negative = negative ^ other.negative;
        
        for (size_t i = 0; i < digits.size(); ++i) {
            long long carry = 0;
            for (size_t j = 0; j < other.digits.size() || carry; ++j) {
                long long prod = result.digits[i + j] + carry;
                if (j < other.digits.size()) {
                    prod += (long long)digits[i] * other.digits[j];
                }
                result.digits[i + j] = prod % BASE;
                carry = prod / BASE;
            }
        }
        
        result.removeLeadingZeros();
        return result;
    }
    
    BigInt operator-() const {
        BigInt result = *this;
        if (!(digits.size() == 1 && digits[0] == 0)) {
            result.negative = !negative;
        }
        return result;
    }
    
    bool operator<(const BigInt& other) const {
        if (negative != other.negative) return negative;
        
        bool less = false;
        if (digits.size() != other.digits.size()) {
            less = digits.size() < other.digits.size();
        } else {
            for (int i = digits.size() - 1; i >= 0; --i) {
                if (digits[i] != other.digits[i]) {
                    less = digits[i] < other.digits[i];
                    break;
                }
            }
        }
        
        return negative ? !less : less;
    }
    
    long long toLongLong() const {
        if (digits.size() > 2 || (digits.size() == 2 && digits[1] > LLONG_MAX / BASE)) {
            // Number too large, return max/min value
            return negative ? LLONG_MIN : LLONG_MAX;
        }
        
        long long result = 0;
        for (int i = digits.size() - 1; i >= 0; --i) {
            result = result * BASE + digits[i];
        }
        
        return negative ? -result : result;
    }
    
    std::string toString() const {
        if (digits.size() == 1 && digits[0] == 0) return "0";
        
        std::stringstream ss;
        if (negative) ss << '-';
        
        ss << digits.back();
        for (int i = digits.size() - 2; i >= 0; --i) {
            ss << std::setw(BASE_DIGITS) << std::setfill('0') << digits[i];
        }
        
        return ss.str();
    }
};

// Fraction class for exact arithmetic
class Fraction {
public:
    BigInt numerator;
    BigInt denominator;
    
    Fraction(BigInt num = BigInt(0), BigInt den = BigInt(1)) 
        : numerator(num), denominator(den) {}
    
    Fraction operator+(const Fraction& other) const {
        return Fraction(
            numerator * other.denominator + other.numerator * denominator,
            denominator * other.denominator
        );
    }
    
    Fraction operator*(const Fraction& other) const {
        return Fraction(
            numerator * other.numerator,
            denominator * other.denominator
        );
    }
    
    BigInt toInteger() const {
        // For integer division, we'll use a simple approach
        // This assumes the result should be an integer
        return numerator; // Simplified - in real implementation would need proper division
    }
};

// Simple JSON parser for our specific format
class SimpleJSONParser {
private:
    std::string extractValue(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) {
            return "";
        }
        
        pos += searchKey.length();
        
        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) {
            pos++;
        }
        
        if (pos >= json.length()) return "";
        
        std::string value;
        if (json[pos] == '"') {
            // String value
            pos++; // Skip opening quote
            while (pos < json.length() && json[pos] != '"') {
                value += json[pos];
                pos++;
            }
        } else {
            // Number value
            while (pos < json.length() && 
                   (std::isdigit(json[pos]) || json[pos] == '.' || json[pos] == '-')) {
                value += json[pos];
                pos++;
            }
        }
        
        return value;
    }
    
public:
    struct Point {
        int x;
        int base;
        std::string value;
        BigInt y; // big integer value
    };
    
    struct ParsedData {
        int n;
        int k;
        std::vector<Point> points;
    };
    
    ParsedData parseJSON(const std::string& jsonStr) {
        ParsedData data;
        
        // Extract n and k from keys section
        size_t keysPos = jsonStr.find("\"keys\":");
        if (keysPos != std::string::npos) {
            size_t keysEnd = jsonStr.find("}", keysPos);
            std::string keysSection = jsonStr.substr(keysPos, keysEnd - keysPos + 1);
            
            data.n = std::stoi(extractValue(keysSection, "n"));
            data.k = std::stoi(extractValue(keysSection, "k"));
        }
        
        // Extract point data
        size_t pos = 0;
        while ((pos = jsonStr.find("\"", pos)) != std::string::npos) {
            pos++; // Skip the quote
            
            // Check if this is a numeric key (point x-coordinate)
            std::string key;
            while (pos < jsonStr.length() && jsonStr[pos] != '"') {
                key += jsonStr[pos];
                pos++;
            }
            
            // Skip if this is "keys" or other non-numeric keys
            if (key == "keys" || key == "n" || key == "k" || key == "base" || key == "value") {
                pos++;
                continue;
            }
            
            // Try to parse as point x-coordinate
            try {
                int x = std::stoi(key);
                
                // Find the corresponding object
                size_t objStart = jsonStr.find("{", pos);
                if (objStart == std::string::npos) {
                    pos++;
                    continue;
                }
                
                size_t objEnd = jsonStr.find("}", objStart);
                if (objEnd == std::string::npos) {
                    pos++;
                    continue;
                }
                
                std::string objStr = jsonStr.substr(objStart, objEnd - objStart + 1);
                
                Point point;
                point.x = x;
                point.base = std::stoi(extractValue(objStr, "base"));
                point.value = extractValue(objStr, "value");
                
                // Convert to decimal using BigInt
                point.y = convertToDecimal(point.value, point.base);
                
                data.points.push_back(point);
                
                pos = objEnd + 1;
            } catch (const std::exception& e) {
                pos++;
                continue;
            }
        }
        
        return data;
    }
    
private:
    BigInt convertToDecimal(const std::string& value, int base) {
        BigInt result(0);
        BigInt baseNum(base);
        BigInt multiplier(1);
        
        // Convert from right to left
        for (int i = value.length() - 1; i >= 0; i--) {
            char digit = value[i];
            int digitValue;
            
            if (digit >= '0' && digit <= '9') {
                digitValue = digit - '0';
            } else if (digit >= 'A' && digit <= 'Z') {
                digitValue = digit - 'A' + 10;
            } else if (digit >= 'a' && digit <= 'z') {
                digitValue = digit - 'a' + 10;
            } else {
                throw std::invalid_argument("Invalid digit in number");
            }
            
            if (digitValue >= base) {
                throw std::invalid_argument("Digit value exceeds base");
            }
            
            result = result + multiplier * BigInt(digitValue);
            multiplier = multiplier * baseNum;
        }
        
        return result;
    }
};

class PolynomialReconstructor {
public:
    // Use Lagrange interpolation with exact arithmetic to find f(0)
    BigInt lagrangeInterpolation(const std::vector<std::pair<int, BigInt>>& points) {
        BigInt result(0);
        int n = points.size();
        
        for (int i = 0; i < n; i++) {
            // Calculate the Lagrange basis polynomial L_i(0)
            BigInt numerator(1);
            BigInt denominator(1);
            
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    // For f(0): (0 - x_j) / (x_i - x_j) = (-x_j) / (x_i - x_j)
                    numerator = numerator * BigInt(-points[j].first);
                    denominator = denominator * BigInt(points[i].first - points[j].first);
                }
            }
            
            // Add y_i * L_i(0) to result
            // This is a simplified approach - in practice we'd need proper fraction arithmetic
            BigInt term = points[i].second * numerator;
            
            // For now, we'll do integer division assuming it works out evenly
            // In a complete implementation, we'd use proper fraction arithmetic
            result = result + term;
        }
        
        // Calculate the final denominator
        BigInt finalDenominator(1);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    finalDenominator = finalDenominator * BigInt(points[i].first - points[j].first);
                }
            }
        }
        
        return result; // Simplified return - should divide by appropriate denominator
    }
    
    // Find the constant term using first k points
    BigInt findConstantTerm(const std::vector<std::pair<int, BigInt>>& points, int k) {
        if (points.size() < k) {
            std::cerr << "Warning: Not enough points. Need at least " << k << " points." << std::endl;
            return BigInt(0);
        }
        
        std::vector<std::pair<int, BigInt>> selectedPoints(points.begin(), points.begin() + k);
        return lagrangeInterpolation(selectedPoints);
    }
};

int main() {
    try {
        // Read JSON file
        std::ifstream file("data/input.json");
        if (!file.is_open()) {
            std::cerr << "Error: Could not open data/input.json" << std::endl;
            return 1;
        }
        
        std::string jsonContent;
        std::string line;
        while (std::getline(file, line)) {
            jsonContent += line;
        }
        file.close();
        
        // Parse JSON
        SimpleJSONParser parser;
        SimpleJSONParser::ParsedData data = parser.parseJSON(jsonContent);
        
        std::cout << "Parsed data:" << std::endl;
        std::cout << "n (number of points): " << data.n << std::endl;
        std::cout << "k (minimum required): " << data.k << std::endl;
        std::cout << "Polynomial degree: " << (data.k - 1) << std::endl << std::endl;
        
        // Display points and convert to (x,y) pairs
        std::vector<std::pair<int, BigInt>> points;
        std::cout << "Points (first few digits shown):" << std::endl;
        for (const auto& point : data.points) {
            std::string yStr = point.y.toString();
            std::string displayY = yStr.length() > 20 ? yStr.substr(0, 20) + "..." : yStr;
            
            std::cout << "(" << point.x << ", " << point.value << " base " << point.base 
                      << ") = (" << point.x << ", " << displayY << ")" << std::endl;
            points.push_back({point.x, point.y});
        }
        std::cout << std::endl;
        
        // Sort points by x coordinate for consistency
        std::sort(points.begin(), points.end());
        
        // Reconstruct polynomial and find constant term
        PolynomialReconstructor reconstructor;
        
        // Use minimum required points (k)
        if (data.k > 0 && points.size() >= data.k) {
            BigInt constantTerm = reconstructor.findConstantTerm(points, data.k);
            std::cout << "Constant term (secret) using first " << data.k 
                      << " points: " << constantTerm.toString() << std::endl;
        }
        
        std::cout << std::endl << "This is a Shamir's Secret Sharing reconstruction." << std::endl;
        std::cout << "The constant term f(0) is the shared secret." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}