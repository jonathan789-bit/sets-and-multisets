#include "CantorSet.h"

#include <algorithm>
#include <cctype>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace {

void skipWs(const std::string& s, std::size_t& p) {
    while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p]))) ++p;
}

std::string trim(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

}  // namespace

CantorSet::CantorSet(const std::string& text) : CantorSet(fromString(text)) {}

CantorSet CantorSet::fromString(const std::string& text) {
    std::size_t pos = 0;
    CantorSet result = parseSet(text, pos);
    skipWs(text, pos);
    if (pos != text.size()) {
        throw std::invalid_argument("unexpected characters after set at position " +
                                    std::to_string(pos));
    }
    return result;
}

CantorSet CantorSet::parseSet(const std::string& s, std::size_t& p) {
    skipWs(s, p);
    if (p >= s.size() || s[p] != '{') {
        throw std::invalid_argument("expected '{' at position " + std::to_string(p));
    }
    ++p;
    CantorSet result;
    skipWs(s, p);
    if (p < s.size() && s[p] == '}') {
        ++p;
        return result;
    }
    while (true) {
        skipWs(s, p);
        if (p >= s.size()) throw std::invalid_argument("unexpected end of input");
        if (s[p] == '{') {
            result.addSet(parseSet(s, p));
        } else {
            std::size_t start = p;
            while (p < s.size() && s[p] != ',' && s[p] != '}' && s[p] != '{') ++p;
            std::string atom = trim(s.substr(start, p - start));
            if (atom.empty()) {
                throw std::invalid_argument("empty element at position " + std::to_string(start));
            }
            result.addAtom(atom);
        }
        skipWs(s, p);
        if (p >= s.size()) throw std::invalid_argument("unexpected end of input");
        if (s[p] == ',') {
            ++p;
        } else if (s[p] == '}') {
            ++p;
            break;
        } else {
            throw std::invalid_argument("expected ',' or '}' at position " + std::to_string(p));
        }
    }
    return result;
}

std::string CantorSet::toString() const {
    std::string out = "{";
    bool first = true;
    for (const auto& a : atoms_) {
        if (!first) out += ", ";
        out += a;
        first = false;
    }
    for (const auto& s : sets_) {
        if (!first) out += ", ";
        out += s.toString();
        first = false;
    }
    return out + "}";
}

void CantorSet::sortSets() {
    std::sort(sets_.begin(), sets_.end(), [](const CantorSet& a, const CantorSet& b) {
        return a.toString() < b.toString();
    });
}

bool CantorSet::addAtom(const std::string& atom) {
    auto it = std::lower_bound(atoms_.begin(), atoms_.end(), atom);
    if (it != atoms_.end() && *it == atom) return false;
    atoms_.insert(it, atom);
    return true;
}

bool CantorSet::addSet(const CantorSet& set) {
    if (containsSet(set)) return false;
    sets_.push_back(set);
    sortSets();
    return true;
}

bool CantorSet::removeAtom(const std::string& atom) {
    auto it = std::lower_bound(atoms_.begin(), atoms_.end(), atom);
    if (it == atoms_.end() || *it != atom) return false;
    atoms_.erase(it);
    return true;
}

bool CantorSet::removeSet(const CantorSet& set) {
    auto it = std::find(sets_.begin(), sets_.end(), set);
    if (it == sets_.end()) return false;
    sets_.erase(it);
    return true;
}

bool CantorSet::containsAtom(const std::string& atom) const {
    return std::binary_search(atoms_.begin(), atoms_.end(), atom);
}

bool CantorSet::containsSet(const CantorSet& set) const {
    return std::find(sets_.begin(), sets_.end(), set) != sets_.end();
}

std::size_t CantorSet::size() const { return atoms_.size() + sets_.size(); }
bool CantorSet::empty() const { return size() == 0; }

void CantorSet::clear() {
    atoms_.clear();
    sets_.clear();
}

CantorSet CantorSet::unionWith(const CantorSet& other) const {
    CantorSet r = *this;
    for (const auto& a : other.atoms_) r.addAtom(a);
    for (const auto& s : other.sets_) r.addSet(s);
    return r;
}

CantorSet CantorSet::intersection(const CantorSet& other) const {
    CantorSet r;
    for (const auto& a : atoms_)
        if (other.containsAtom(a)) r.addAtom(a);
    for (const auto& s : sets_)
        if (other.containsSet(s)) r.addSet(s);
    return r;
}

CantorSet CantorSet::difference(const CantorSet& other) const {
    CantorSet r;
    for (const auto& a : atoms_)
        if (!other.containsAtom(a)) r.addAtom(a);
    for (const auto& s : sets_)
        if (!other.containsSet(s)) r.addSet(s);
    return r;
}

CantorSet CantorSet::powerSet() const {
    const std::size_t n = size();
    if (n > 20) throw std::length_error("power set is too large (size > 20)");
    CantorSet result;
    for (std::size_t mask = 0; mask < (std::size_t{1} << n); ++mask) {
        CantorSet sub;
        for (std::size_t i = 0; i < n; ++i) {
            if (!((mask >> i) & 1)) continue;
            if (i < atoms_.size()) sub.addAtom(atoms_[i]);
            else sub.addSet(sets_[i - atoms_.size()]);
        }
        result.addSet(sub);
    }
    return result;
}

CantorSet CantorSet::operator+(const CantorSet& rhs) const { return unionWith(rhs); }
CantorSet CantorSet::operator*(const CantorSet& rhs) const { return intersection(rhs); }
CantorSet CantorSet::operator-(const CantorSet& rhs) const { return difference(rhs); }

CantorSet& CantorSet::operator+=(const CantorSet& rhs) {
    *this = unionWith(rhs);
    return *this;
}

// Both vectors are kept in canonical order, so element-wise comparison is
// enough (nested sets are compared recursively).
bool CantorSet::operator==(const CantorSet& rhs) const {
    return atoms_ == rhs.atoms_ && sets_ == rhs.sets_;
}

bool CantorSet::operator!=(const CantorSet& rhs) const { return !(*this == rhs); }

std::ostream& operator<<(std::ostream& out, const CantorSet& set) {
    return out << set.toString();
}

std::istream& operator>>(std::istream& in, CantorSet& set) {
    in >> std::ws;
    std::string text;
    int depth = 0;
    char c;
    while (in.get(c)) {
        text.push_back(c);
        if (c == '{') {
            ++depth;
        } else if (c == '}') {
            if (--depth <= 0) break;
        } else if (depth == 0) {
            break;  // text that does not start with '{'
        }
    }
    try {
        set = CantorSet::fromString(text);
    } catch (const std::invalid_argument&) {
        in.setstate(std::ios::failbit);
    }
    return in;
}