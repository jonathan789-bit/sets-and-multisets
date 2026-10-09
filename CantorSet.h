#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

/**
 * @brief Unordered Cantor set: a collection of unique elements, where each
 *        element is either an atom (a string) or another CantorSet.
 *
 * Text form: {a, b, c, {a, b}, {}, {a, {c}}}.
 * Elements are unordered and never repeated. Internally elements are kept in
 * canonical (sorted) order, so equality does not depend on insertion order.
 * The class performs no console I/O.
 */
class CantorSet {
public:
    /// Creates an empty set.
    CantorSet() = default;

    /**
     * @brief Builds a set from text such as "{a, b, {c}}".
     * @throws std::invalid_argument if the text is malformed.
     */
    explicit CantorSet(const std::string& text);

    CantorSet(const CantorSet&) = default;
    CantorSet(CantorSet&&) noexcept = default;
    CantorSet& operator=(const CantorSet&) = default;
    CantorSet& operator=(CantorSet&&) noexcept = default;
    ~CantorSet() = default;

    /// Parses a set from text. @throws std::invalid_argument on bad input.
    static CantorSet fromString(const std::string& text);

    /// Canonical text form, e.g. "{a, b, {a, b}}".
    std::string toString() const;

    /// Adds an atom. @return true if it was not already present.
    bool addAtom(const std::string& atom);
    /// Adds a nested set. @return true if it was not already present.
    bool addSet(const CantorSet& set);
    /// Removes an atom. @return true if it was present.
    bool removeAtom(const std::string& atom);
    /// Removes a nested set. @return true if it was present.
    bool removeSet(const CantorSet& set);
    /// @return true if the atom is a direct element.
    bool containsAtom(const std::string& atom) const;
    /// @return true if the nested set is a direct element.
    bool containsSet(const CantorSet& set) const;

    /// Number of direct elements (atoms + nested sets).
    std::size_t size() const;
    /// @return true if there are no elements.
    bool empty() const;
    /// Removes all elements.
    void clear();

    /// Union of two sets.
    CantorSet unionWith(const CantorSet& other) const;
    /// Intersection of two sets.
    CantorSet intersection(const CantorSet& other) const;
    /// Elements of this set that are not in @p other.
    CantorSet difference(const CantorSet& other) const;
    /**
     * @brief Set of all subsets (2^n elements).
     * @throws std::length_error if size() > 20.
     */
    CantorSet powerSet() const;

    CantorSet operator+(const CantorSet& rhs) const;  ///< union
    CantorSet operator*(const CantorSet& rhs) const;  ///< intersection
    CantorSet operator-(const CantorSet& rhs) const;  ///< difference
    CantorSet& operator+=(const CantorSet& rhs);      ///< in-place union

    bool operator==(const CantorSet& rhs) const;
    bool operator!=(const CantorSet& rhs) const;

private:
    static CantorSet parseSet(const std::string& s, std::size_t& pos);
    void sortSets();

    std::vector<std::string> atoms_;  ///< sorted, unique
    std::vector<CantorSet> sets_;     ///< sorted by text form, unique
};

/// Writes the canonical text form.
std::ostream& operator<<(std::ostream& out, const CantorSet& set);
/// Reads one balanced "{...}" set; sets failbit on malformed input.
std::istream& operator>>(std::istream& in, CantorSet& set);