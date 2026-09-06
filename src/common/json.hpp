#pragma once
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <variant>
#include <memory>
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace vmh {

class Json {
public:
    enum Type { Null, Bool, Int, Double, String, Array, Object };

    using ObjectMap = std::map<std::string, Json>;
    using ArrayVec = std::vector<Json>;
    using Value = std::variant<
        std::nullptr_t,
        bool,
        int64_t,
        double,
        std::string,
        std::shared_ptr<ArrayVec>,
        std::shared_ptr<ObjectMap>
    >;

    Json() : m_value(nullptr), m_type(Null) {}
    Json(std::nullptr_t) : m_value(nullptr), m_type(Null) {}
    Json(bool v) : m_value(v), m_type(Bool) {}
    Json(int v) : m_value(static_cast<int64_t>(v)), m_type(Int) {}
    Json(int64_t v) : m_value(v), m_type(Int) {}
    Json(size_t v) : m_value(static_cast<int64_t>(v)), m_type(Int) {}
    Json(double v) : m_value(v), m_type(Double) {}
    Json(const char* v) : m_value(std::string(v)), m_type(String) {}
    Json(const std::string& v) : m_value(v), m_type(String) {}
    Json(std::string&& v) : m_value(std::move(v)), m_type(String) {}

    static Json object() {
        Json j;
        j.m_type = Object;
        j.m_value = std::make_shared<ObjectMap>();
        return j;
    }

    static Json array() {
        Json j;
        j.m_type = Array;
        j.m_value = std::make_shared<ArrayVec>();
        return j;
    }

    Type type() const { return m_type; }
    bool isNull() const { return m_type == Null; }
    bool isObject() const { return m_type == Object; }
    bool isArray() const { return m_type == Array; }
    bool isString() const { return m_type == String; }
    bool isNumber() const { return m_type == Int || m_type == Double; }
    bool isBool() const { return m_type == Bool; }

    bool getBool(bool def = false) const {
        if (m_type == Bool) return std::get<bool>(m_value);
        return def;
    }

    int64_t getInt(int64_t def = 0) const {
        if (m_type == Int) return std::get<int64_t>(m_value);
        if (m_type == Double) return static_cast<int64_t>(std::get<double>(m_value));
        return def;
    }

    double getDouble(double def = 0.0) const {
        if (m_type == Double) return std::get<double>(m_value);
        if (m_type == Int) return static_cast<double>(std::get<int64_t>(m_value));
        return def;
    }

    std::string getString(const std::string& def = "") const {
        if (m_type == String) return std::get<std::string>(m_value);
        return def;
    }

    size_t size() const {
        if (m_type == Array) return getArr()->size();
        if (m_type == Object) return getObj()->size();
        return 0;
    }

    bool contains(const std::string& key) const {
        if (m_type != Object) return false;
        return getObj()->count(key) > 0;
    }

    Json& operator[](const std::string& key) {
        if (m_type != Object) {
            m_type = Object;
            m_value = std::make_shared<ObjectMap>();
        }
        return (*std::get<std::shared_ptr<ObjectMap>>(m_value))[key];
    }

    const Json& operator[](const std::string& key) const {
        static const Json null_json;
        if (m_type != Object) return null_json;
        auto& obj = *getObj();
        auto it = obj.find(key);
        if (it == obj.end()) return null_json;
        return it->second;
    }

    Json& operator[](size_t idx) {
        if (m_type != Array) {
            m_type = Array;
            m_value = std::make_shared<ArrayVec>();
        }
        auto& arr = *std::get<std::shared_ptr<ArrayVec>>(m_value);
        if (idx >= arr.size()) arr.resize(idx + 1);
        return arr[idx];
    }

    const Json& operator[](size_t idx) const {
        static const Json null_json;
        if (m_type != Array) return null_json;
        auto& arr = *getArr();
        if (idx >= arr.size()) return null_json;
        return arr[idx];
    }

    void push_back(const Json& val) {
        if (m_type != Array) {
            m_type = Array;
            m_value = std::make_shared<ArrayVec>();
        }
        std::get<std::shared_ptr<ArrayVec>>(m_value)->push_back(val);
    }

    const ObjectMap& items() const {
        static const ObjectMap empty;
        if (m_type != Object) return empty;
        return *getObj();
    }

    const ArrayVec& elements() const {
        static const ArrayVec empty;
        if (m_type != Array) return empty;
        return *getArr();
    }

    // value() with default
    template<typename T>
    T value(const std::string& key, const T& def) const {
        if (!contains(key)) return def;
        const auto& v = (*this)[key];
        if constexpr (std::is_same_v<T, bool>) return v.getBool(def);
        else if constexpr (std::is_same_v<T, int>) return static_cast<int>(v.getInt(def));
        else if constexpr (std::is_same_v<T, int64_t>) return v.getInt(def);
        else if constexpr (std::is_same_v<T, double>) return v.getDouble(def);
        else if constexpr (std::is_same_v<T, std::string>) return v.getString(def);
        else return def;
    }

    std::string dump(int indent = -1) const {
        std::ostringstream ss;
        dumpImpl(ss, indent, 0);
        return ss.str();
    }

    static Json parse(const std::string& text) {
        size_t pos = 0;
        return parseValue(text, pos);
    }

    static Json loadFromFile(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) {
            throw std::runtime_error("Cannot open JSON file: " + path);
        }
        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        return parse(content);
    }

    void saveToFile(const std::string& path) const {
        std::ofstream f(path);
        if (!f.is_open()) {
            throw std::runtime_error("Cannot write JSON file: " + path);
        }
        f << dump(2);
    }

private:
    Value m_value;
    Type m_type;

    ObjectMap* getObj() const {
        return std::get<std::shared_ptr<ObjectMap>>(m_value).get();
    }

    ArrayVec* getArr() const {
        return std::get<std::shared_ptr<ArrayVec>>(m_value).get();
    }

    void dumpImpl(std::ostringstream& ss, int indent, int depth) const {
        std::string pad = (indent >= 0) ? std::string(depth * indent, ' ') : "";
        std::string padInner = (indent >= 0) ? std::string((depth + 1) * indent, ' ') : "";
        std::string nl = (indent >= 0) ? "\n" : "";
        std::string sep = (indent >= 0) ? " " : "";

        switch (m_type) {
        case Null:   ss << "null"; break;
        case Bool:   ss << (std::get<bool>(m_value) ? "true" : "false"); break;
        case Int:    ss << std::get<int64_t>(m_value); break;
        case Double: {
            double d = std::get<double>(m_value);
            ss << d;
            std::string s = ss.str();
            if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
                ss << ".0";
            break;
        }
        case String: ss << "\"" << escapeString(std::get<std::string>(m_value)) << "\""; break;
        case Array: {
            auto& arr = *getArr();
            if (arr.empty()) { ss << "[]"; break; }
            ss << "[" << nl;
            for (size_t i = 0; i < arr.size(); ++i) {
                ss << padInner;
                arr[i].dumpImpl(ss, indent, depth + 1);
                if (i + 1 < arr.size()) ss << ",";
                ss << nl;
            }
            ss << pad << "]";
            break;
        }
        case Object: {
            auto& obj = *getObj();
            if (obj.empty()) { ss << "{}"; break; }
            ss << "{" << nl;
            size_t i = 0;
            for (auto& [k, v] : obj) {
                ss << padInner << "\"" << escapeString(k) << "\":" << sep;
                v.dumpImpl(ss, indent, depth + 1);
                if (++i < obj.size()) ss << ",";
                ss << nl;
            }
            ss << pad << "}";
            break;
        }
        }
    }

    static std::string escapeString(const std::string& s) {
        std::string r;
        r.reserve(s.size());
        for (char c : s) {
            switch (c) {
            case '"':  r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n"; break;
            case '\r': r += "\\r"; break;
            case '\t': r += "\\t"; break;
            default:   r += c;
            }
        }
        return r;
    }

    static void skipWhitespace(const std::string& s, size_t& pos) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\n' || s[pos] == '\r' || s[pos] == '\t'))
            ++pos;
    }

    static Json parseValue(const std::string& s, size_t& pos) {
        skipWhitespace(s, pos);
        if (pos >= s.size()) return Json();

        switch (s[pos]) {
        case '{': return parseObject(s, pos);
        case '[': return parseArray(s, pos);
        case '"': return parseString(s, pos);
        case 't': case 'f': return parseBool(s, pos);
        case 'n': return parseNull(s, pos);
        default:  return parseNumber(s, pos);
        }
    }

    static Json parseObject(const std::string& s, size_t& pos) {
        Json j = Json::object();
        ++pos; // skip '{'
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == '}') { ++pos; return j; }

        while (pos < s.size()) {
            skipWhitespace(s, pos);
            if (s[pos] != '"') throw std::runtime_error("Expected string key in JSON object");
            std::string key = parseString(s, pos).getString();
            skipWhitespace(s, pos);
            if (pos >= s.size() || s[pos] != ':') throw std::runtime_error("Expected ':' in JSON object");
            ++pos;
            j[key] = parseValue(s, pos);
            skipWhitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; continue; }
            if (pos < s.size() && s[pos] == '}') { ++pos; return j; }
            throw std::runtime_error("Expected ',' or '}' in JSON object");
        }
        return j;
    }

    static Json parseArray(const std::string& s, size_t& pos) {
        Json j = Json::array();
        ++pos; // skip '['
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') { ++pos; return j; }

        while (pos < s.size()) {
            j.push_back(parseValue(s, pos));
            skipWhitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; continue; }
            if (pos < s.size() && s[pos] == ']') { ++pos; return j; }
            throw std::runtime_error("Expected ',' or ']' in JSON array");
        }
        return j;
    }

    static Json parseString(const std::string& s, size_t& pos) {
        ++pos; // skip opening '"'
        std::string result;
        while (pos < s.size() && s[pos] != '"') {
            if (s[pos] == '\\') {
                ++pos;
                if (pos >= s.size()) break;
                switch (s[pos]) {
                case '"':  result += '"'; break;
                case '\\': result += '\\'; break;
                case '/':  result += '/'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    if (pos + 4 < s.size()) {
                        std::string hex = s.substr(pos + 1, 4);
                        unsigned long cp = std::strtoul(hex.c_str(), nullptr, 16);
                        if (cp < 0x80) {
                            result += static_cast<char>(cp);
                        } else if (cp < 0x800) {
                            result += static_cast<char>(0xC0 | (cp >> 6));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (cp >> 12));
                            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        }
                        pos += 4;
                    }
                    break;
                }
                default: result += s[pos];
                }
            } else {
                result += s[pos];
            }
            ++pos;
        }
        if (pos < s.size()) ++pos; // skip closing '"'
        return Json(result);
    }

    static Json parseNumber(const std::string& s, size_t& pos) {
        size_t start = pos;
        bool isFloat = false;
        if (s[pos] == '-') ++pos;
        while (pos < s.size() && std::isdigit(s[pos])) ++pos;
        if (pos < s.size() && s[pos] == '.') { isFloat = true; ++pos; }
        while (pos < s.size() && std::isdigit(s[pos])) ++pos;
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
            isFloat = true; ++pos;
            if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
            while (pos < s.size() && std::isdigit(s[pos])) ++pos;
        }
        std::string numStr = s.substr(start, pos - start);
        if (isFloat) return Json(std::stod(numStr));
        return Json(static_cast<int64_t>(std::stoll(numStr)));
    }

    static Json parseBool(const std::string& s, size_t& pos) {
        if (s.compare(pos, 4, "true") == 0) { pos += 4; return Json(true); }
        if (s.compare(pos, 5, "false") == 0) { pos += 5; return Json(false); }
        throw std::runtime_error("Invalid JSON boolean");
    }

    static Json parseNull(const std::string& s, size_t& pos) {
        if (s.compare(pos, 4, "null") == 0) { pos += 4; return Json(); }
        throw std::runtime_error("Invalid JSON null");
    }
};

using json = Json;

} // namespace vmh
