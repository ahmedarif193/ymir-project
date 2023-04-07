#include "string.h"
#include "vector.h"
#include "map.h"
#include "uuid.h"
#include "json.h"

#include <cassert>
#include <iostream>


int main() {
    {
        
        // Generate a UUID
        lxcd::string uuid = lxcd::UUIDGenerator::generate();
        
        // Verify that the UUID is in the correct format (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)
        if (uuid.length() != 36 ||
                uuid[8] != '-' ||
                uuid[13] != '-' ||
                uuid[18] != '-' ||
                uuid[23] != '-') {
            std::cout << "Invalid UUID format: " << uuid << std::endl;
            return 1;
        }
        
        // Check that all characters in the UUID are hexadecimal digits or hyphens
        const char* validChars = "0123456789abcdefABCDEF-";
        for (lxcd::size_t i = 0; i < uuid.length(); ++i) {
            if (std::strchr(validChars, uuid[i]) == nullptr) {
                std::cout << "Invalid UUID character: " << uuid[i] << std::endl;
                return 1;
            }
        }
        
        // Generate 500 UUIDs and store them in a vector
        lxcd::vector<lxcd::string> uuids;
        for (int i = 0; i < 500; ++i) {
            uuids.push_back(lxcd::UUIDGenerator::generate());
        }
        
        // Verify that all UUIDs are unique
        for (lxcd::size_t i = 0; i < uuids.size(); ++i) {
            for (lxcd::size_t j = i + 1; j < uuids.size(); ++j) {
                if (uuids[i] == uuids[j]) {
                    std::cout << "Duplicate UUID: " << uuids[i] << std::endl;
                    return 1;
                }
            }
        }
        
    }
    int arr[] = {1, 2, 3, 4, 5};
    lxcd::vector<int> vec(arr, 5);
    
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;    
    for (int i = 0; i < 500; ++i) {
        {
            lxcd::string json = "{\"name\": \"John\", \"age\": 30}";
            lxcd::JsonParser parser(json);
            lxcd::JsonValue root = parser.parse();
            lxcd::string name = root["name"].asString();  // "John"
            int age = root["age"].asInt();          // 30
        }
        // Test lxcd::string class
        lxcd::string s1 = "hello";
        assert(s1.size() == 5);
        assert(s1[0] == 'h');
        assert(s1[1] == 'e');
        assert(s1[2] == 'l');
        assert(s1[3] == 'l');
        assert(s1[4] == 'o');
        auto s22 = s1 + " world";
        
        // Test split function
        lxcd::string s2 = "foo,bar,baz";
        lxcd::vector<lxcd::string> v = s2.split(',');
        assert(v.size() == 3);
        assert(v[0] == "foo");
        assert(v[1] == "bar");
        assert(v[2] == "baz");
        
        {
            // test insert at beginning of string
            lxcd::string s("world");
            s.insert(0, "hello ");
            assert(s == "hello world");
            
            // test insert in middle of string
            lxcd::string s2("the fox jumps");
            s2.insert(4, "quick brown ");
            assert(s2 == "the quick brown fox jumps");//the fox quick brown jumps
            
            // test insert at end of string
            lxcd::string s3("hello");
            s3.insert(s3.size(), " world");
            assert(s3 == "hello world");
            
        }
        {
            // test insert with string argument
            lxcd::string s1("world");
            s1.insert(0, "hello ");
            assert(s1 == "hello world");
        }
        {
            // Test methos of string class
            lxcd::string s1("hello");
            lxcd::string s2("world");
            lxcd::string s3 = s1 + " " + s2;
            assert(s3 == "hello world");
            s3.append(", how are you?");
            assert(s3 == "hello world, how are you?");
            s3.erase(5, 6);
            
            assert(s3 == "hello, how are you?");
            assert(s3.find("how") == 7);
            assert(s3.find("xyz") == lxcd::string::npos);
            assert(!s3.empty());
            assert(s3.length() == 19);
        }
        
        {
            // Test rfind
            lxcd::string s1("hello, world");
            assert(s1.rfind("world") == 7);
            assert(s1.rfind("o") == 8);
            assert(s1.rfind("foo") == lxcd::string::npos);
            
            // Test substr
            lxcd::string s2("hello, world");
            assert(s2.substr(0, 5) == "hello");
            assert(s2.substr(7) == "world");
            assert(s2.substr(0, 0).empty());
            assert(s2.substr(0, lxcd::string::npos) == s2);
            
            // Test append with char* and length
            lxcd::string s3("hello");
            s3.append(", world!", 7);
            assert(s3 == "hello, world");
            
            // Test append with lxcd::string
            lxcd::string s4("hello");
            s4.append(lxcd::string(", world!"));
            assert(s4 == "hello, world!");
            
        }
        
        // Test lxcd::vector class
        lxcd::vector<int> v1;
        assert(v1.empty());
        v1.push_back(1);
        v1.push_back(2);
        v1.push_back(3);
        assert(v1.size() == 3);
        assert(v1[0] == 1);
        assert(v1[1] == 2);
        assert(v1[2] == 3);
        v1.resize(2);
        assert(v1.size() == 2);
        assert(v1[0] == 1);
        assert(v1[1] == 2);
        v1.resize(3);
        assert(v1.size() == 3);
        assert(v1[0] == 1);
        assert(v1[1] == 2);
        assert(v1[2] == 0);
        v1.reserve(10);
        assert(v1.capacity() == 10);
        
        // Test with Valgrind
        lxcd::string* s3 = new lxcd::string("test");
        assert(s3->size() == 4);
        delete s3;
        lxcd::vector<int>* v2 = new lxcd::vector<int>;
        v2->push_back(1);
        v2->push_back(2);
        v2->push_back(3);
        assert(v2->size() == 3);
        delete v2;
        {
            // Test vector insert method
            lxcd::vector<int> vec;
            vec.push_back(1);
            vec.push_back(2);
            vec.push_back(3);
            

        }
        
        {
            lxcd::vector<int> vec;
            vec.push_back(1);
            vec.push_back(2);
            vec.push_back(3);
            
            // Iterate over the vector using iterators
            for (lxcd::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it) {
                //std::cout << *it << " "<<std::endl;
            }
        }
        
        {
            // Test map class
            lxcd::map<int,lxcd::string> m;
            m.insert({1, lxcd::string("one")});
            m.insert({2, lxcd::string("two")});
            m.insert({3, lxcd::string("three")});
            m.insert({4, lxcd::string("four")});
            assert(m.size() == 4);
            assert(m[1] == "one");
            assert(m[2] == "two");
            assert(m[3] == "three");
            assert(m[4] == "four");
            m[1] = lxcd::string("uno");
            m[2] = lxcd::string("dos");
            m[3] = lxcd::string("tres");
            m[4] = lxcd::string("cuatro");
            assert(m.size() == 4);
            assert(m[1] == "uno");
            assert(m[2] == "dos");
            assert(m[3] == "tres");
            assert(m[4] == "cuatro");
        }
        {
            lxcd::string s = "world";
            const char* cstr = "Hello, ";
            lxcd::string result = cstr + s;
        }
    }
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
