#include <cassert>
#include <utility>
#include <iterator>

#include "src/trie.hpp"

const auto& concat = [](std::string& Seq, char C)
-> std::string& {
    Seq.push_back(C);
    return Seq;
};

using namespace ltr;
using default_trie = trie<char, int, decltype(concat)>;


void TestCtors() {
    static_assert(!std::is_trivially_constructible_v<default_trie>);

    default_trie justConcat(concat);
    assert(justConcat.size() == 0);

    default_trie initList{{{"key1",      31},
                           {"something", 5112},
                           {"fajsjk",    51},
                           {"hjazuwa",   72}}, concat };
    assert(initList.size() == 4);
    assert(initList.at("key1") = 31);
    assert(initList["hjazuwa"] = 72);
    try {
        initList.at("nosuchkey");
        assert(false);
    }
    catch (std::out_of_range ex) {}

    default_trie inputIt(concat, (++++initList.cbegin()), initList.cend());
    assert(inputIt.size() == 2);
    assert(inputIt.count("key1") == 1);
    assert(inputIt.count("fajsjk") == 0);
    assert(inputIt.count("something") == 1);
    assert(inputIt.count("hjazuwa") == 0);
    initList.at("something") = 99;
    // inputIt creates independent copies
    assert(inputIt.at("something") == 5112);

    {
        default_trie copy(initList);
        assert(copy.size() == initList.size());
        copy.at("key1") = 62;
        assert(initList.at("key1") == 31);
    }

    default_trie moved(std::move(initList));
    // initList.size() will cause access violation because of nullptr dereference
    assert(moved.size() == 4);
}

void TestAssign() {
    default_trie trie{{{"key1",      31},
                       {"something", 5112},
                       {"fajsjk",    51},
                       {"hjazuwa",   72}}, concat };
    trie = trie;
    default_trie other(concat);

    other = trie;
    assert(other.size() == 4);
    other["key1"] = 50;
    assert(trie["key1"] == 31);

    default_trie another(concat);
    another = std::move(other);
    // other.size() will crash again
    
    other = {{"abc", 123},
             {"cba", 345}};
    assert(other.size() == 2);
}

void TestAcces() {
    default_trie trie{{{"key1",      31},
                       {"something", 5112},
                       {"fajsjk",    51},
                       {"hjazuwa",   72}}, concat };
    assert(trie.at("key1") == 31);
    trie.at("something") = 46;
    assert(trie.at("something") == 46);
    try {
        trie.at("nosuchkey");
        assert(false);
    }catch(std::out_of_range ex){}

    trie["fajsjk"] = 75;
    assert(trie["fajsjk"] == 75);
    trie["newkey"];
    assert(trie.size() == 5 && trie.at("newkey") == int());
}

void TestIterator() {
    default_trie trie{{{"key1",    31},
                       {"key",     5112},
                       {"fajsjk",  51},
                       {"hjazuwa", 72}}, concat };
    default_trie::iterator start = trie.begin();
    // lexicographical order!
    assert(start->first == "fajsjk" && start->second == 51);
    ++start;
    assert(start->first == "hjazuwa");
    ++start;
    assert(start->first == "key");
    ++start;
    assert(start->first == "key1");
    ++start;
    assert(start == trie.end());
    const default_trie ctrie = trie;
    static_assert(std::is_same_v<decltype(ctrie.begin()), default_trie::const_iterator>);
    auto rstart = trie.rbegin();
    assert(rstart->first == "key1");
    ++rstart;
    assert(rstart->first == "key");
    ++rstart;
    assert(rstart->first == "hjazuwa");
    ++rstart;
    assert(rstart->first == "fajsjk");
    ++rstart;
    assert(rstart == trie.rend());
}

void TestCapacity() {
    default_trie empty(concat);
    assert(empty.empty());
    default_trie trie{{{"key1",   31},
                       {"key",    5112},
                       {"fajsjk", 51},
                       {"hjazuwa",72}}, concat };
    assert(!trie.empty());
    empty = trie;
    assert(!empty.empty());
    assert(trie.size() == 4);
    trie["new"];
    assert(trie.size() == 5);
    trie["fajsjk"];
    assert(trie.size() == 5);
    trie["faj"];
    assert(trie.size() == 6);
}

void TestClear() {
    default_trie trie{{{"key1",   31},
                       {"key",    5112},
                       {"fajsjk", 51},
                       {"hjazuwa",72}}, concat };
    auto end = trie.end();
    trie.clear();
    assert(trie.empty());
    assert(end == trie.end());
}

void TestInsert() {
    default_trie trie(concat);
    // const T&
    const std::pair<const std::basic_string<char>, int>& toInsert = std::make_pair("key1", 50);
    auto result = trie.insert(toInsert);
    assert(result.second && result.first->first == "key1" && result.first->second == 50);

    // P&& from which T is constructible
    result = trie.insert(std::make_pair<std::string, long>("key2", 60));
    assert(result.second);

    // P&& from which T is NOT constructible - won't compile
    // result = trie.insert(std::make_pair<std::string, std::string>("not", "valid"));

    // T&&, also key is already present
    result = trie.insert({ "key2", 70 });
    assert(!result.second);
    assert(result.first->second == 60);

    // inputIt
    default_trie other(concat);
    other.insert(std::begin(trie), std::end(trie));
    assert(other.size() == 2);

    // initializer list, with 1 key already present
    other.insert({ {"key3", 80}, {"key1", 80 }, {"key5", 80} });
    assert(other.size() == 4);
    assert(other.at("key1") == 50);

    // insert_or_assign with const key_type& key, with assignment
    const std::string key = "key1";
    result = trie.insert_or_assign("key1", 80);
    assert(!result.second && result.first->second == 80 && trie.at("key1") == 80);

    // insert_or_assign with key_type&& key, with insert
    result = trie.insert_or_assign("newkey", 5);
    assert(result.second);
}

void TestEmplace() {
    default_trie trie(concat);
    // Args...
    auto result = trie.emplace("key", 50);
    assert(result.second && result.first->first == "key" && result.first->second == 50);

    result = trie.emplace(std::make_pair(std::basic_string<char>("key"), long(80)));
    assert(!result.second && result.first->first == "key" && result.first->second == 50);

    result = trie.emplace(std::make_pair("key", long(90)));
    assert(!result.second && result.first->first == "key" && result.first->second == 50);

    result = trie.emplace(std::piecewise_construct,
                          std::forward_as_tuple("key"),
                          std::forward_as_tuple(60));
    assert(!result.second && result.first->first == "key" && result.first->second == 50);

    // const key_type&, Args...
    const std::basic_string<char> key("key2");
    result = trie.try_emplace(key, 90);
    assert(result.second && result.first->first == "key2" && result.first->second == 90);

    // key_type&&, Args...
    result = trie.try_emplace("key3", 80);
    assert(result.second && result.first->first == "key3" && result.first->second == 80);
}

void TestErase() {
    default_trie trie{{{"abc",   31},
                       {"abcd",   5112},
                       {"abcde",  51},
                       {"bcd",    72},
                       {"bcde",   72},
                       {"hgasha", 80}}, concat};
    assert(trie.size() == 6);
    auto& eraseAbcd = ++trie.begin();
    eraseAbcd = trie.erase(eraseAbcd);
    assert(trie.size() == 5);
    try {
        trie.at("abcd");
        assert(false);
    }
    catch (std::out_of_range) {}
    assert(eraseAbcd->first == "abcde");

    default_trie::size_type size = trie.erase("bcd");
    assert(trie.size() == 4);
    // not removed
    assert(trie.at("bcde") == 72);

    auto eraseMost = trie.erase(trie.cbegin(), --trie.cend());
    assert(trie.size() == 1);
    assert(trie.at("hgasha") == 80);

    trie.erase("hgasha");
    assert(trie.empty());
}

void TestSwap() {
    default_trie trie{{{"abc",   31},
                       {"abcd",  5112},
                       {"abcde", 51}}, concat };
    default_trie other(concat);

    trie.swap(other);
    assert(trie.empty());
    assert(other.size() == 3);

    trie = { {"abc", 2}, {"key2", 3} };

    trie.swap(other);
    assert(trie.at("abc") == 31);
    assert(other.at("abc") == 2);
}

// comparison for transparent lookup using string length
// T will be substituted for char
// that given that trie compares based on key-parts at a time
// making it required for comp to also provide operator< for 
// the sequence type for transparent comparisons to be used
template<typename T>
struct transparent_compare{
    using is_transparent = void;

    bool operator()(const std::string& lhs, std::size_t rhs) const {
        return lhs.size() < rhs;
    }

    bool operator()(std::size_t lhs, const std::string& rhs) const {
        return lhs < rhs.size();
    }

    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return lhs < rhs;
    }

    bool operator()(const T& lhs, const T& rhs) const {
        return lhs < rhs;
    }
};

void TestLookup() {
    using allow_transparent = trie<char, int, decltype(concat), transparent_compare>;
    allow_transparent trie{{{"abc",   31},
                            {"abcd",   5112},
                            {"abcde",  51},
                            {"bcd",    72},
                            {"bcde",   72},
                            {"hgasha", 80}}, concat };
    // even for multiple possible matches, count should only return 0 or 1 which might
    // seem weird but it's due to string.length() hardly being a unique key

    assert(trie.count("abc") == 1);
    assert(trie.count("abf") == 0);
    // search using transparent key which is string.length in our case
    assert(trie.count(3) == 1);

    default_trie def {{{"abc",    31},
                       {"abcd",   5112},
                       {"abcde",  51},
                       {"bcd",    72},
                       {"bcde",   72},
                       {"hgasha", 80}}, concat };

    // for transparent keys custom comparators can be passed
    std::size_t res = def.count<std::size_t, transparent_compare<char>>(3);
    assert(res == 1);

    allow_transparent::iterator result = trie.find("bcd");
    assert(result != trie.end() && result->second == 72);
    result->second = 58;
    assert(trie.find("bcd")->second == 58);
    const allow_transparent ctrie = trie;

    allow_transparent::const_iterator cresult = ctrie.find("nope");
    assert(cresult == ctrie.end());
    // transparent lookup based on string.length will find the first key with matching length
    // called on both const and regular trie for different template instantiation
    assert(trie.find(4)->first == "abcd");
    assert(ctrie.find(5)->first == "abcde");

    assert(trie.contains("bcd") && !trie.contains("nope"));
    assert(trie.contains(3) && !trie.contains(2));

    // called on both const and regular for reasons same as before
    assert(trie.lower_bound("bcd")->first == "bcd");
    assert(trie.lower_bound("x") == trie.end());
    assert(trie.lower_bound(1)->first == "abc");
    assert(ctrie.lower_bound("bcd")->first == "bcd");
    assert(ctrie.lower_bound("x") == ctrie.end());
    assert(ctrie.lower_bound(1)->first == "abc");

    // similar to lower_bound, but returns the first greater key instead of not less
    assert(trie.upper_bound("bcd")->first == "bcde");
    assert(trie.upper_bound("x") == trie.end());
    assert(trie.upper_bound(1)->first == "abc");
    assert(ctrie.upper_bound("bcd")->first == "bcde");
    assert(ctrie.upper_bound("x") == ctrie.end());
    assert(ctrie.upper_bound(1)->first == "abc");

    std::pair<allow_transparent::iterator, allow_transparent::iterator> resultPair = trie.equal_range("bcd");
    assert(resultPair.first->first == "bcd" && resultPair.second->first == "bcde");

    resultPair = trie.equal_range("x");
    assert(resultPair.first == trie.end() && resultPair.second == trie.end());

    resultPair = trie.equal_range(3);
    assert(resultPair.first->first == "abc" && resultPair.second->first == "abcd");

    std::pair<allow_transparent::const_iterator, allow_transparent::const_iterator> cresultPair = ctrie.equal_range("bcd");
    assert(cresultPair.first->first == "bcd" && cresultPair.second->first == "bcde");

    cresultPair = ctrie.equal_range("x");
    assert(cresultPair.first == ctrie.end() && cresultPair.second == ctrie.end());

    cresultPair = ctrie.equal_range(3);
    assert(cresultPair.first->first == "abc" && cresultPair.second->first == "abcd");
}

void TestObservers() {
    using allow_transparent = trie<char, int, decltype(concat), transparent_compare>;
    default_trie dtrie(concat);
    allow_transparent ttrie(concat);

    static_assert(std::is_same_v<decltype(dtrie.key_comp()), std::less<char>>);
    static_assert(std::is_same_v<decltype(ttrie.key_comp()), transparent_compare<char>>);
}

void TestNonmembers() {
    default_trie trie{{{"abc",    31},
                       {"abcd",   5112},
                       {"abcde",  51},
                       {"bcd",    72},
                       {"bcde",   72},
                       {"hgasha", 80}}, concat };
    default_trie other = trie;
    assert(trie == other);
    other.at("abc") = 20;
    assert(trie != other);
    assert(other < trie);
    other.erase("abc");
    assert(trie < other);
    swap(trie, other);
    assert(other <= other);
}

// Not intended to be a thorough test, main goal is to try out all overloads
int main() {
    TestCtors();
    TestAssign();
    TestIterator();
    TestCapacity();
    TestClear();
    TestInsert();
    TestEmplace();
    TestErase();
    TestSwap();
    TestLookup();
    TestObservers();
    TestNonmembers();
    return 0;
}