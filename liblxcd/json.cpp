#include "utils/json.h"

namespace lxcd {

map<string, JsonValue> JsonValue::empty_obj_ = {};
vector<JsonValue> JsonValue::empty_arr_ = {};

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
    while(string::isdigit(json_[pos_]) || json_[pos_] == '-' || json_[pos_] == '.' || json_[pos_] == 'e' || json_[pos_] == 'E') {
        pos_++;
    }
    string num = json_.substr(start, pos_ - start);
    return JsonValue(string::stod(num));
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

JsonValue::JsonValue() : type_(Type::Null) {
}

JsonValue::JsonValue(JsonValue::Type type) : type_(type) {
}

JsonValue::JsonValue(const char *str) : type_(Type::String), str_(str) {
}

JsonValue::JsonValue(const string &str) : type_(Type::String), str_(str) {
}

JsonValue::JsonValue(int value) : type_(Type::Number), str_(string::to_string(value)) {
}

JsonValue::JsonValue(double value) : type_(Type::Number), str_(string::to_string(value)) {
}

JsonValue::JsonValue(bool value) : type_(Type::Boolean), str_(value ? "true" : "false") {
}

void JsonValue::append(const JsonValue &value) {
    if(type_ == Type::Array) {
        arr_.push_back(value);
    }
}

void JsonValue::remove(size_t index) {
    if((type_ == Type::Array) && (index < arr_.size())) {
        arr_.erase(arr_.begin() + index);
    }
}

void JsonValue::set(const string &key, const JsonValue &value) {
    if(type_ == Type::Object) {
        obj_[key] = value;
    }
}

void JsonValue::remove(const string &key) {
    if(type_ == Type::Object) {
        obj_.erase(key);
    }
}

bool JsonValue::isNull() const {
    return type_ == Type::Null;
}

bool JsonValue::isObject() const {
    return type_ == Type::Object;
}

bool JsonValue::isArray() const {
    return type_ == Type::Array;
}

bool JsonValue::isString() const {
    return type_ == Type::String;
}

bool JsonValue::isNumber() const {
    return type_ == Type::Number;
}

bool JsonValue::isBoolean() const {
    return type_ == Type::Boolean;
}

const JsonValue &JsonValue::operator[](size_t index) const {
    static JsonValue nullValue;
    if((type_ == Type::Array) && (index < arr_.size())) {
        return arr_[index];
    }
    return nullValue;
}

JsonValue &JsonValue::operator[](size_t index) {
    static JsonValue nullValue;
    if((type_ == Type::Array) && (index < arr_.size())) {
        return arr_[index];
    }
    return nullValue;
}

const JsonValue &JsonValue::operator[](const string &key) const {
    static JsonValue nullValue;
    if((type_ == Type::Object) && obj_.count(key)) {
        return obj_.at(key);
    }
    return nullValue;
}

JsonValue &JsonValue::operator[](const string &key) {
    static JsonValue nullValue;
    if((type_ == Type::Object) && !obj_[key].isNull()) {
        return obj_[key];
    }
    return nullValue;
}

const string &JsonValue::asString() const {
    static string emptyString;
    return type_ == Type::String ? str_ : emptyString;
}

int JsonValue::asInt() const {
    return type_ == Type::Number ? string::stoi(str_) : 0;
}

double JsonValue::asDouble() const {
    return type_ == Type::Number ? string::stod(str_) : 0.0;
}

bool JsonValue::asBool() const {
    return type_ == Type::Boolean && str_ == "true";
}

const map<string, JsonValue> &JsonValue::asObject() const {
    return type_ == Type::Object ? obj_ : JsonValue::empty_obj_;
}

const vector<JsonValue> &JsonValue::asArray() const {
    return type_ == Type::Array ? arr_ : JsonValue::empty_arr_;
}

JsonValue::Type JsonValue::getType() const {
    return type_;
}

JsonSerializer::JsonSerializer(const JsonValue &value) : value_(value) {
}

string JsonSerializer::serialize() {
    return serializeValue(value_);
}

string JsonSerializer::serializeValue(const JsonValue &value) {
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

string JsonSerializer::serializeString(const string &str) {
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

string JsonSerializer::serializeNumber(double number) {
    char buffer[32];
    snprintf(buffer, 32, "%.17g", number);
    return buffer;
}

string JsonSerializer::serializeObject(const map<string, JsonValue> &obj) {
    string result;
    result += '{';
    bool first = true;
    for(const auto& pair : obj) {
        if(!first) {
            result += ',';
        }
        result += serializeString(pair.key);
        result += ':';
        result += serializeValue(pair.value);
        first = false;
    }
    result += '}';
    return result;
}

string JsonSerializer::serializeArray(const vector<JsonValue> &arr) {
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

string JsonSerializer::serializeBoolean(bool value) {
    return value ? "true" : "false";
}

string JsonSerializer::serializeNull() {
    return "null";
}

}
