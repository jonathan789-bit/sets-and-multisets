#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

/**
 * @brief Unordered Cantor multiset: elements may repeat. Each element is
 *        either an atom (a string) or another CantorMultiset.
 *
 * Text form: {a, a, c, {a, b, b}, {}, {a, {c, c}}}.
 * Elements are stored as (element, multiplicity) pairs in canonical order, so
 * equality does not depend on the order elements were given.
 * The class performs no console I/O.
 */
class CantorMultiset {
public:
    /// Creates an empty multiset.
    CantorMultiset() = default;

    /**
     * @brief Builds a multiset from text such as "{a, a, {b, b}}".
     * @throws std::invalid_argument if the text is malformed.
     */
    explicit CantorMultiset(const std::string& text);

    CantorMultiset(const CantorMultiset&) = default;
    CantorMultiset(CantorMultiset&&) noexcept = default;
    CantorMultiset& operator=(const CantorMultiset&) = default;
    CantorMultiset& operator=(CantorMultiset&&) noexcept = default;
    ~CantorMultiset() = default;

    /// Parses a multiset from text. @throws std::invalid_argument on bad input.
    static CantorMultiset fromString(const std::string& text);

    /// Canonical text form; repeated elements are printed repeatedly.
    std::string toString() const;

    /// Adds @p count copies of an atom (no-op if count is 0).
    void addAtom(const std::string& atom, std::size_t count = 1);
    /// Adds @p count copies of a nested multiset (no-op if count is 0).
    void addSet(const CantorMultiset& set, std::size_t count = 1);
    /// Removes up to @p count copies of an atom. @return how many were removed.
    std::size_t removeAtom(const std::string& atom, std::size_t count = 1);
    /// Removes up to @p count copies of a nested multiset. @return how many were removed.
    std::size_t removeSet(const CantorMultiset& set, std::size_t count = 1);

    /// Multiplicity of an atom (0 if absent).
    std::size_t countAtom(const std::string& atom) const;
    /// Multiplicity of a nested multiset (0 if absent).
    std::size_t countSet(const CantorMultiset& set) const;
    /// @return true if the atom occurs at least once.
    bool containsAtom(const std::string& atom) const;
    /// @return true if the nested multiset occurs at least once.
    bool containsSet(const CantorMultiset& set) const;

    /// Total number of elements, counting repeats.
    std::size_t size() const;
    /// Number of distinct elements.
    std::size_t distinctSize() const;
    /// @return true if there are no elements.
    bool empty() const;
    /// Removes all elements.
    void clear();

    /// Union: each element takes the maximum multiplicity.
    CantorMultiset unionWith(const CantorMultiset& other) const;
    /// Sum: multiplicities are added.
    CantorMultiset sum(const CantorMultiset& other) const;
    /// Intersection: each element takes the minimum multiplicity.
    CantorMultiset intersection(const CantorMultiset& other) const;
    /// Difference: multiplicities are subtracted, never below zero.
    CantorMultiset difference(const CantorMultiset& other) const;
    /**
     * @brief Multiset of all sub-multisets.
     * @throws std::length_error if there would be more than 65536 of them.
     */
    CantorMultiset powerSet() const;

    CantorMultiset operator+(const CantorMultiset& rhs) const;  ///< sum
    CantorMultiset operator*(const CantorMultiset& rhs) const;  ///< intersection
    CantorMultiset operator-(const CantorMultiset& rhs) const;  ///< difference
    CantorMultiset& operator+=(const CantorMultiset& rhs);      ///< in-place sum

    bool operator==(const CantorMultiset& rhs) const;
    bool operator!=(const CantorMultiset& rhs) const;

private:
    static CantorMultiset parseSet(const std::string& s, std::size_t& pos);
    std::size_t findSet(const CantorMultiset& set) const;

    std::map<std::string, std::size_t> atoms_;  ///< atom -> multiplicity (> 0)
    std::vector<CantorMultiset> sets_;          ///< distinct nested sets, sorted by text form
    std::vector<std::size_t> setCounts_;        ///< multiplicity of sets_[i] (> 0)
};

/// Writes the canonical text form.
std::ostream& operator<<(std::ostream& out, const CantorMultiset& set);
/// Reads one balanced "{...}" multiset; sets failbit on malformed input.
std::istream& operator>>(std::istream& in, CantorMultiset& set);