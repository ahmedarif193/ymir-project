#include "string.h"
#include "map.h"

namespace lxcd {
class JsonValue {
public:
enum class Type { Null, Object, Array, String, Number, Boolean };

JsonValue();
explicit JsonValue(Type type);
explicit JsonValue(const char* str);
explicit JsonValue(const string& str);
explicit JsonValue(int value);
explicit JsonValue(double value);
explicit JsonValue(bool value);

// Methods for manipulating objects and arrays
void append(const JsonValue& value);

void remove(size_t index);

void set(const string& key, const JsonValue& value);

void remove(const string& key);

// Methods for checking the type of a value
bool isNull() const;
bool isObject() const;
bool isArray() const;
bool isString() const;
bool isNumber() const;
bool isBoolean() const;

// Methods for accessing the data
const JsonValue& operator[](size_t index) const;

JsonValue& operator[](size_t index);

const JsonValue& operator[](const string& key) const;

JsonValue& operator[](const string& key);

const string& asString() const;

int asInt() const;

double asDouble() const;

bool asBool() const;
const map<string, JsonValue>& asObject() const;

const vector<JsonValue>& asArray() const;
Type getType() const;
private:
Type type_;
string str_;
vector<JsonValue> arr_;
map<string, JsonValue> obj_;
static map<string, JsonValue> empty_obj_;
static vector<JsonValue> empty_arr_;
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
explicit JsonSerializer(const JsonValue& value);
string serialize();

private:
const JsonValue& value_;

string serializeValue(const JsonValue& value);

string serializeString(const string& str);

string serializeNumber(double number);

string serializeObject(const map<string, JsonValue>& obj);

string serializeArray(const vector<JsonValue>& arr);

string serializeBoolean(bool value);

string serializeNull();
};

}
