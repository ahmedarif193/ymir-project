#include "string.h"
#include "map.h"

namespace lxcd {
class JsonValue {
public:
enum class Type { Null, Object, Array, String, Number, Boolean };

JsonValue() : type_(Type::Null) {
}
explicit JsonValue(Type type) : type_(type) {
}
explicit JsonValue(const char* str) : type_(Type::String), str_(str) {
}
explicit JsonValue(const string& str) : type_(Type::String), str_(str) {
}
explicit JsonValue(int value) : type_(Type::Number), str_(to_string(value)) {
}
explicit JsonValue(double value) : type_(Type::Number), str_(to_string(value)) {
}
explicit JsonValue(bool value) : type_(Type::Boolean), str_(value ? "true" : "false") {
}

// Methods for manipulating objects and arrays
void append(const JsonValue& value) {
    if(type_ == Type::Array) {
        arr_.push_back(value);
    }
}

void remove(size_t index) {
    if((type_ == Type::Array) && (index < arr_.size())) {
        arr_.erase(arr_.begin() + index);
    }
}

void set(const string& key, const JsonValue& value) {
    if(type_ == Type::Object) {
        obj_[key] = value;
    }
}

void remove(const string& key) {
    if(type_ == Type::Object) {
        obj_.erase(key);
    }
}

// Methods for checking the type of a value
bool isNull() const {
    return type_ == Type::Null;
}
bool isObject() const {
    return type_ == Type::Object;
}
bool isArray() const {
    return type_ == Type::Array;
}
bool isString() const {
    return type_ == Type::String;
}
bool isNumber() const {
    return type_ == Type::Number;
}
bool isBoolean() const {
    return type_ == Type::Boolean;
}

// Methods for accessing the data
const JsonValue& operator[](size_t index) const {
    static JsonValue nullValue;
    if((type_ == Type::Array) && (index < arr_.size())) {
        return arr_[index];
    }
    return nullValue;
}

JsonValue& operator[](size_t index) {
    static JsonValue nullValue;
    if((type_ == Type::Array) && (index < arr_.size())) {
        return arr_[index];
    }
    return nullValue;
}

const JsonValue& operator[](const string& key) const {
    static JsonValue nullValue;
    if((type_ == Type::Object) && obj_.count(key)) {
        return obj_.at(key);
    }
    return nullValue;
}

JsonValue& operator[](const string& key) {
    static JsonValue nullValue;
    if((type_ == Type::Object) && obj_[key]) {
        return obj_[key];
    }
    return nullValue;
}

const string& asString() const {
    static string emptyString;
    return type_ == Type::String ? str_ : emptyString;
}

int asInt() const {
    return type_ == Type::Number ? stoi(str_) : 0;
}

double asDouble() const {
    return type_ == Type::Number ? stod(str_) : 0.0;
}

bool asBool() const {
    return type_ == Type::Boolean && str_ == "true";
}
const map<string, JsonValue>& asObject() const {
    return type_ == Type::Object ? obj_ : empty_obj_;
}

const vector<JsonValue>& asArray() const {
    return type_ == Type::Array ? arr_ : empty_arr_;
}
Type getType() const;
private:
Type type_;
string str_;
vector<JsonValue> arr_;
map<string, JsonValue> obj_;
static const map<string, JsonValue> empty_obj_;
static const vector<JsonValue> empty_arr_;
};

class JsonParser {
public:
explicit JsonParser(const string& json);
JsonValue parse();

private:
string json_;
size_t pos_;

JsonValue parseValue();
JsonValue parseString();
JsonValue parseNumber();
JsonValue parseObject();
JsonValue parseArray();
JsonValue parseBoolean();
JsonValue parseNull();
};

class JsonSerializer {
public:
explicit JsonSerializer(const JsonValue& value) : value_(value) {
}
string serialize() {
    return serializeValue(value_);
}

private:
const JsonValue& value_;

string serializeValue(const JsonValue& value) {
    switch(value.getType()) {
    case JsonValue::Type::String:
        return serializeString(value.asString());
    case JsonValue::Type::Number:
        return serializeNumber(value.asInt());
    case JsonValue::Type::Object:
        return serializeObject(value.asObject());
    case JsonValue::Type::Array:
        return serializeArray(value.asArray());
    case JsonValue::Type::Boolean:
        return serializeBoolean(value.asBool());
    case JsonValue::Type::Null:
        return serializeNull();
    }
    return "";
}

string serializeString(const string& str) {
    string result;
    result += '"';
    for(char ch : str) {
        if((ch == '"') || (ch == '\\')) {
            result += '\\';
        }
        result += ch;
    }
    result += '"';
    return result;
}

string serializeNumber(double number) {
    char buffer[32];
    snprintf(buffer, 32, "%.17g", number);
    return buffer;
}

string serializeObject(const map<string, JsonValue>& obj) {
    string result;
    result += '{';
    bool first = true;
    for(const auto& pair : obj) {
        if(!first) {
            result += ',';
        }
        result += serializeString(pair.first);
        result += ':';
        result += serializeValue(pair.second);
        first = false;
    }
    result += '}';
    return result;
}

string serializeArray(const vector<JsonValue>& arr) {
    string result;
    result += '[';
    bool first = true;
    for(const auto& value : arr) {
        if(!first) {
            result += ',';
        }
        result += serializeValue(value);
        first = false;
    }
    result += ']';
    return result;
}

string serializeBoolean(bool value) {
    return value ? "true" : "false";
}

string serializeNull() {
    return "null";
}
};


JsonParser::JsonParser(const string& json) : json_(json), pos_(0) {
}

JsonValue JsonParser::parse() {
    return parseValue();
}

JsonValue JsonParser::parseValue() {
    switch(json_[pos_]) {
    case '{':
        return parseObject();
    case '[':
        return parseArray();
    case '\"':
        return parseString();
    case 't':
    case 'f':
        return parseBoolean();
    case 'n':
        return parseNull();
    default:
        return parseNumber();
    }
}

JsonValue JsonParser::parseString() {
    size_t start = ++pos_;
    while(json_[pos_] != '\"') {
        if((json_[pos_] == '\\') && (json_[pos_ + 1] == '\"')) {
            json_.erase(pos_, 1);
        }
        pos_++;
    }
    string str = json_.substr(start, pos_ - start);
    pos_++;  // skip ending quote
    return JsonValue(str);
}

JsonValue JsonParser::parseNumber() {
    size_t start = pos_;
    while(isdigit(json_[pos_]) || json_[pos_] == '-' || json_[pos_] == '.' || json_[pos_] == 'e' || json_[pos_] == 'E') {
        pos_++;
    }
    string num = json_.substr(start, pos_ - start);
    return JsonValue(stod(num));
}

JsonValue JsonParser::parseObject() {
    JsonValue obj(JsonValue::Type::Object);
    pos_++;  // skip opening brace
    while(json_[pos_] != '}') {
        string key = parseString().asString();
        pos_++;  // skip colon
        JsonValue val = parseValue();
        obj.set(key, val);
        if(json_[pos_] == ',') {
            pos_++;  // skip comma
        }
    }
    pos_++;  // skip closing brace
    return obj;
}

JsonValue JsonParser::parseArray() {
    JsonValue arr(JsonValue::Type::Array);
    pos_++;  // skip opening bracket
    while(json_[pos_] != ']') {
        JsonValue val = parseValue();
        arr.append(val);
        if(json_[pos_] == ',') {
            pos_++;  // skip comma
        }
    }
    pos_++;  // skip closing bracket
    return arr;
}

JsonValue JsonParser::parseBoolean() {
    if(json_[pos_] == 't') {
        pos_ += 4;
        return JsonValue(true);
    } else {
        pos_ += 5;
        return JsonValue(false);
    }
}

JsonValue JsonParser::parseNull() {
    pos_ += 4;
    return JsonValue(JsonValue::Type::Null);
}
JsonValue::Type JsonValue::getType() const {
    return type_;
}
}
