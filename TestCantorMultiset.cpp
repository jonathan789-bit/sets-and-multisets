#include <gtest/gtest.h>

#include <sstream>

#include "CantorMultiset.h"

TEST(MsParse, SpecExample) {
    CantorMultiset s("{a, a, c, {a, b, b}, {}, {a, {c, c}}}");
    EXPECT_EQ(s.size(), 6u);
    EXPECT_EQ(s.distinctSize(), 5u);
    EXPECT_EQ(s.countAtom("a"), 2u);
    EXPECT_EQ(s.countAtom("c"), 1u);
    EXPECT_TRUE(s.containsSet(CantorMultiset("{b, a, b}")));
    EXPECT_TRUE(s.containsSet(CantorMultiset("{}")));
    EXPECT_TRUE(s.containsSet(CantorMultiset("{{c, c}, a}")));
    EXPECT_FALSE(s.containsSet(CantorMultiset("{a, b}")));
}

TEST(MsParse, EmptyWhitespaceAndRepeats) {
    EXPECT_TRUE(CantorMultiset("  {  }  ").empty());
    CantorMultiset s(" { a ,a , {b}, {b} } ");
    EXPECT_EQ(s.countAtom("a"), 2u);
    EXPECT_EQ(s.countSet(CantorMultiset("{b}")), 2u);
}

TEST(MsParse, Errors) {
    for (const char* bad : {"", "a", "{", "{a", "{a,}", "{,a}", "{a b", "{a}}", "{a{b}}", "{a,,b}", "{{}"}) {
        EXPECT_THROW(CantorMultiset::fromString(bad), std::invalid_argument) << bad;
    }
}

TEST(MsText, CanonicalOutputAndRoundTrip) {
    CantorMultiset s("{c, {b, a, a}, a, a, {}, {}}");
    EXPECT_EQ(s.toString(), "{a, a, c, {a, a, b}, {}, {}}");
    EXPECT_EQ(CantorMultiset(s.toString()), s);
}

TEST(MsEquality, OrderIndependentAndCountSensitive) {
    EXPECT_EQ(CantorMultiset("{a, b, a, {c, d, c}}"), CantorMultiset("{{c, c, d}, a, a, b}"));
    EXPECT_NE(CantorMultiset("{a, b}"), CantorMultiset("{a, a, b}"));
    EXPECT_NE(CantorMultiset("{{a}}"), CantorMultiset("{{a}, {a}}"));
    EXPECT_NE(CantorMultiset("{a}"), CantorMultiset("{{a}}"));
    EXPECT_FALSE(CantorMultiset("{a}") != CantorMultiset("{a}"));
}

TEST(MsModify, AddRemoveCount) {
    CantorMultiset s;
    s.addAtom("x", 3);
    s.addAtom("x", 0);  // no-op
    s.addSet(CantorMultiset("{x}"), 2);
    s.addSet(CantorMultiset("{x}"), 0);  // no-op
    EXPECT_EQ(s.countAtom("x"), 3u);
    EXPECT_EQ(s.countSet(CantorMultiset("{x}")), 2u);
    EXPECT_EQ(s.size(), 5u);

    EXPECT_EQ(s.removeAtom("x", 2), 2u);
    EXPECT_EQ(s.countAtom("x"), 1u);
    EXPECT_EQ(s.removeAtom("x", 5), 1u);
    EXPECT_FALSE(s.containsAtom("x"));
    EXPECT_EQ(s.removeAtom("x"), 0u);

    EXPECT_EQ(s.removeSet(CantorMultiset("{x}")), 1u);
    EXPECT_EQ(s.removeSet(CantorMultiset("{x}"), 9), 1u);
    EXPECT_EQ(s.removeSet(CantorMultiset("{x}")), 0u);
    EXPECT_TRUE(s.empty());

    s.addAtom("y");
    s.clear();
    EXPECT_TRUE(s.empty());
}

TEST(MsAlgebra, UnionSumIntersectionDifference) {
    CantorMultiset a("{a, a, b, {x}}"), b("{a, b, b, {x}, {x}}");
    EXPECT_EQ(a.unionWith(b), CantorMultiset("{a, a, b, b, {x}, {x}}"));
    EXPECT_EQ(a + b, CantorMultiset("{a, a, a, b, b, b, {x}, {x}, {x}}"));
    EXPECT_EQ(a * b, CantorMultiset("{a, b, {x}}"));
    EXPECT_EQ(a - b, CantorMultiset("{a}"));
    EXPECT_EQ(b - a, CantorMultiset("{b, {x}}"));
    CantorMultiset c = a;
    c += b;
    EXPECT_EQ(c, a + b);
}

TEST(MsPowerSet, SmallAndTooLarge) {
    EXPECT_EQ(CantorMultiset("{}").powerSet(), CantorMultiset("{{}}"));
    EXPECT_EQ(CantorMultiset("{a, a}").powerSet(), CantorMultiset("{{}, {a}, {a, a}}"));
    EXPECT_EQ(CantorMultiset("{a, a, b}").powerSet().size(), 6u);
    EXPECT_EQ(CantorMultiset("{a, {b}}").powerSet().size(), 4u);
    CantorMultiset big;
    for (int i = 0; i < 17; ++i) big.addAtom("e" + std::to_string(i));
    EXPECT_THROW(big.powerSet(), std::length_error);
}

TEST(MsCopy, IndependentCopies) {
    CantorMultiset a("{a, a, {b}}");
    CantorMultiset b(a);
    b.addAtom("z");
    EXPECT_NE(a, b);
    CantorMultiset c;
    c = a;
    EXPECT_EQ(c, a);
    c = c;  // self-assignment
    EXPECT_EQ(c, a);
    CantorMultiset d(std::move(c));
    EXPECT_EQ(d, a);
}

TEST(MsStream, WriteAndRead) {
    CantorMultiset s("{b, a, a, {c, c}}");
    std::ostringstream out;
    out << s;
    EXPECT_EQ(out.str(), "{a, a, b, {c, c}}");

    std::istringstream in("  {x, x, {y, z}}  {q}");
    CantorMultiset r1, r2;
    in >> r1 >> r2;
    EXPECT_EQ(r1, CantorMultiset("{x, x, {y, z}}"));
    EXPECT_EQ(r2, CantorMultiset("{q}"));
}

TEST(MsStream, ReadFailures) {
    for (const char* text : {"", "abc", "{a, ", "{a,}"}) {
        std::istringstream in(text);
        CantorMultiset keep("{keep}"), s = keep;
        in >> s;
        EXPECT_TRUE(in.fail()) << text;
        EXPECT_EQ(s, keep) << text;
    }
}