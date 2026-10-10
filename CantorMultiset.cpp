#include "CantorMultiset.h"

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

CantorMultiset::CantorMultiset(const std::string& text) : CantorMultiset(fromString(text)) {}

CantorMultiset CantorMultiset::fromString(const std::string& text) {
    std::size_t pos = 0;
    CantorMultiset result = parseSet(text, pos);
    skipWs(text, pos);
    if (pos != text.size()) {
        throw std::invalid_argument("unexpected characters after set at position " +
                                    std::to_string(pos));
    }
    return result;
}

CantorMultiset CantorMultiset::parseSet(const std::string& s, std::size_t& p) {
    skipWs(s, p);
    if (p >= s.size() || s[p] != '{') {
        throw std::invalid_argument("expected '{' at position " + std::to_string(p));
    }
    ++p;
    CantorMultiset result;
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

std::string CantorMultiset::toString() const {
    std::string out = "{";
    bool first = true;
    for (const auto& [atom, n] : atoms_) {
        for (std::size_t i = 0; i < n; ++i) {
            if (!first) out += ", ";
            out += atom;
            first = false;
        }
    }
    for (std::size_t k = 0; k < sets_.size(); ++k) {
        const std::string text = sets_[k].toString();
        for (std::size_t i = 0; i < setCounts_[k]; ++i) {
            if (!first) out += ", ";
            out += text;
            first = false;
        }
    }
    return out + "}";
}

std::size_t CantorMultiset::findSet(const CantorMultiset& set) const {
    for (std::size_t i = 0; i < sets_.size(); ++i) {
        if (sets_[i] == set) return i;
    }
    return sets_.size();
}

void CantorMultiset::addAtom(const std::string& atom, std::size_t count) {
    if (count == 0) return;
    atoms_[atom] += count;
}

void CantorMultiset::addSet(const CantorMultiset& set, std::size_t count) {
    if (count == 0) return;
    const std::string key = set.toString();
    std::size_t i = 0;
    for (; i < sets_.size(); ++i) {
        const std::string k = sets_[i].toString();
        if (k == key) {
            setCounts_[i] += count;
            return;
        }
        if (k > key) break;
    }
    sets_.insert(sets_.begin() + static_cast<std::ptrdiff_t>(i), set);
    setCounts_.insert(setCounts_.begin() + static_cast<std::ptrdiff_t>(i), count);
}

std::size_t CantorMultiset::removeAtom(const std::string& atom, std::size_t count) {
    auto it = atoms_.find(atom);
    if (it == atoms_.end()) return 0;
    std::size_t removed = std::min(count, it->second);
    it->second -= removed;
    if (it->second == 0) atoms_.erase(it);
    return removed;
}

std::size_t CantorMultiset::removeSet(const CantorMultiset& set, std::size_t count) {
    std::size_t i = findSet(set);
    if (i == sets_.size()) return 0;
    std::size_t removed = std::min(count, setCounts_[i]);
    setCounts_[i] -= removed;
    if (setCounts_[i] == 0) {
        sets_.erase(sets_.begin() + static_cast<std::ptrdiff_t>(i));
        setCounts_.erase(setCounts_.begin() + static_cast<std::ptrdiff_t>(i));
    }
    return removed;
}

std::size_t CantorMultiset::countAtom(const std::string& atom) const {
    auto it = atoms_.find(atom);
    return it == atoms_.end() ? 0 : it->second;
}

std::size_t CantorMultiset::countSet(const CantorMultiset& set) const {
    std::size_t i = findSet(set);
    return i == sets_.size() ? 0 : setCounts_[i];
}

bool CantorMultiset::containsAtom(const std::string& atom) const { return countAtom(atom) > 0; }
bool CantorMultiset::containsSet(const CantorMultiset& set) const { return countSet(set) > 0; }

std::size_t CantorMultiset::size() const {
    std::size_t total = 0;
    for (const auto& kv : atoms_) total += kv.second;
    for (std::size_t c : setCounts_) total += c;
    return total;
}

std::size_t CantorMultiset::distinctSize() const { return atoms_.size() + sets_.size(); }
bool CantorMultiset::empty() const { return atoms_.empty() && sets_.empty(); }

void CantorMultiset::clear() {
    atoms_.clear();
    sets_.clear();
    setCounts_.clear();
}

CantorMultiset CantorMultiset::unionWith(const CantorMultiset& other) const {
    CantorMultiset r = *this;
    for (const auto& [atom, n] : other.atoms_) {
        std::size_t have = r.countAtom(atom);
        if (n > have) r.addAtom(atom, n - have);
    }
    for (std::size_t k = 0; k < other.sets_.size(); ++k) {
        std::size_t have = r.countSet(other.sets_[k]);
        if (other.setCounts_[k] > have) r.addSet(other.sets_[k], other.setCounts_[k] - have);
    }
    return r;
}

CantorMultiset CantorMultiset::sum(const CantorMultiset& other) const {
    CantorMultiset r = *this;
    for (const auto& [atom, n] : other.atoms_) r.addAtom(atom, n);
    for (std::size_t k = 0; k < other.sets_.size(); ++k) r.addSet(other.sets_[k], other.setCounts_[k]);
    return r;
}

CantorMultiset CantorMultiset::intersection(const CantorMultiset& other) const {
    CantorMultiset r;
    for (const auto& [atom, n] : atoms_) r.addAtom(atom, std::min(n, other.countAtom(atom)));
    for (std::size_t k = 0; k < sets_.size(); ++k) {
        r.addSet(sets_[k], std::min(setCounts_[k], other.countSet(sets_[k])));
    }
    return r;
}

CantorMultiset CantorMultiset::difference(const CantorMultiset& other) const {
    CantorMultiset r;
    for (const auto& [atom, n] : atoms_) {
        std::size_t sub = other.countAtom(atom);
        if (n > sub) r.addAtom(atom, n - sub);
    }
    for (std::size_t k = 0; k < sets_.size(); ++k) {
        std::size_t sub = other.countSet(sets_[k]);
        if (setCounts_[k] > sub) r.addSet(sets_[k], setCounts_[k] - sub);
    }
    return r;
}

CantorMultiset CantorMultiset::powerSet() const {
    // Distinct elements with their multiplicities: choosing 0..count copies of
    // each one gives every sub-multiset exactly once (mixed-radix counting).
    std::vector<std::size_t> maxCounts;
    for (const auto& kv : atoms_) maxCounts.push_back(kv.second);
    for (std::size_t c : setCounts_) maxCounts.push_back(c);

    const std::size_t limit = 65536;
    std::size_t total = 1;
    for (std::size_t c : maxCounts) {
        total *= (c + 1);
        if (total > limit) throw std::length_error("power set is too large (more than 65536 subsets)");
    }

    CantorMultiset result;
    for (std::size_t idx = 0; idx < total; ++idx) {
        CantorMultiset sub;
        std::size_t rest = idx, e = 0;
        for (const auto& kv : atoms_) {
            sub.addAtom(kv.first, rest % (maxCounts[e] + 1));
            rest /= (maxCounts[e] + 1);
            ++e;
        }
        for (std::size_t k = 0; k < sets_.size(); ++k, ++e) {
            sub.addSet(sets_[k], rest % (maxCounts[e] + 1));
            rest /= (maxCounts[e] + 1);
        }
        result.addSet(sub);
    }
    return result;
}

CantorMultiset CantorMultiset::operator+(const CantorMultiset& rhs) const { return sum(rhs); }
CantorMultiset CantorMultiset::operator*(const CantorMultiset& rhs) const { return intersection(rhs); }
CantorMultiset CantorMultiset::operator-(const CantorMultiset& rhs) const { return difference(rhs); }

CantorMultiset& CantorMultiset::operator+=(const CantorMultiset& rhs) {
    *this = sum(rhs);
    return *this;
}

// Atoms are in a sorted map and nested sets are sorted by text form with
// unique entries, so comparing the containers (recursively) is enough.
bool CantorMultiset::operator==(const CantorMultiset& rhs) const {
    return atoms_ == rhs.atoms_ && setCounts_ == rhs.setCounts_ && sets_ == rhs.sets_;
}

bool CantorMultiset::operator!=(const CantorMultiset& rhs) const { return !(*this == rhs); }

std::ostream& operator<<(std::ostream& out, const CantorMultiset& set) {
    return out << set.toString();
}

std::istream& operator>>(std::istream& in, CantorMultiset& set) {
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
        set = CantorMultiset::fromString(text);
    } catch (const std::invalid_argument&) {
        in.setstate(std::ios::failbit);
    }
    return in;
}