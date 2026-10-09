#include <gtest/gtest.h>

#include <sstream>

#include "CantorSet.h"

TEST(Parse, SpecExample) {
    CantorSet s("{a, b, c, {a, b}, {}, {a, {c}}}");
    EXPECT_EQ(s.size(), 6u);
    EXPECT_TRUE(s.containsAtom("a"));
    EXPECT_TRUE(s.containsSet(CantorSet("{b, a}")));
    EXPECT_TRUE(s.containsSet(CantorSet("{}")));
    EXPECT_TRUE(s.containsSet(CantorSet("{{c}, a}")));
}

TEST(Parse, EmptyAndWhitespace) {
    EXPECT_TRUE(CantorSet("  {  }  ").empty());
    EXPECT_EQ(CantorSet(" { a ,b } ").size(), 2u);
}

TEST(Parse, DuplicatesCollapse) {
    EXPECT_EQ(CantorSet("{a, a, {b}, {b}}").size(), 2u);
}

TEST(Parse, Errors) {
    for (const char* bad : {"", "a", "{", "{a", "{a,}", "{,a}", "{a b", "{a}}", "{a{b}}", "{a,,b}", "{{}"}) {
        EXPECT_THROW(CantorSet::fromString(bad), std::invalid_argument) << bad;
    }
}

TEST(Text, CanonicalOutputAndRoundTrip) {
    CantorSet s("{c, {b, a}, a, {}}");
    EXPECT_EQ(s.toString(), "{a, c, {}, {a, b}}");
    EXPECT_EQ(CantorSet(s.toString()), s);
}

TEST(Equality, OrderIndependent) {
    EXPECT_EQ(CantorSet("{a, b, {c, d}}"), CantorSet("{{d, c}, b, a}"));
    EXPECT_NE(CantorSet("{a}"), CantorSet("{b}"));
    EXPECT_NE(CantorSet("{a}"), CantorSet("{{a}}"));
    EXPECT_FALSE(CantorSet("{a}") != CantorSet("{a}"));
}

TEST(Modify, AddRemoveContains) {
    CantorSet s;
    EXPECT_TRUE(s.addAtom("x"));
    EXPECT_FALSE(s.addAtom("x"));
    EXPECT_TRUE(s.addSet(CantorSet("{x}")));
    EXPECT_FALSE(s.addSet(CantorSet("{x}")));
    EXPECT_EQ(s.size(), 2u);
    EXPECT_TRUE(s.removeAtom("x"));
    EXPECT_FALSE(s.removeAtom("x"));
    EXPECT_TRUE(s.removeSet(CantorSet("{x}")));
    EXPECT_FALSE(s.removeSet(CantorSet("{x}")));
    EXPECT_TRUE(s.empty());
    s.addAtom("y");
    s.clear();
    EXPECT_TRUE(s.empty());
}

TEST(Algebra, UnionIntersectionDifference) {
    CantorSet a("{a, b, {x}}"), b("{b, c, {x}, {y}}");
    EXPECT_EQ(a + b, CantorSet("{a, b, c, {x}, {y}}"));
    EXPECT_EQ(a * b, CantorSet("{b, {x}}"));
    EXPECT_EQ(a - b, CantorSet("{a}"));
    CantorSet c = a;
    c += b;
    EXPECT_EQ(c, a + b);
}

TEST(PowerSet, SmallAndTooLarge) {
    EXPECT_EQ(CantorSet("{}").powerSet(), CantorSet("{{}}"));
    EXPECT_EQ(CantorSet("{a, {b}}").powerSet().size(), 4u);
    EXPECT_EQ(CantorSet("{a}").powerSet(), CantorSet("{{}, {a}}"));
    CantorSet big;
    for (int i = 0; i < 21; ++i) big.addAtom("e" + std::to_string(i));
    EXPECT_THROW(big.powerSet(), std::length_error);
}

TEST(Copy, IndependentCopies) {
    CantorSet a("{a, {b}}");
    CantorSet b(a);
    b.addAtom("z");
    EXPECT_NE(a, b);
    CantorSet c;
    c = a;
    EXPECT_EQ(c, a);
    c = c;  // self-assignment
    EXPECT_EQ(c, a);
    CantorSet d(std::move(c));
    EXPECT_EQ(d, a);
}

TEST(Stream, WriteAndRead) {
    CantorSet s("{b, a, {c}}");
    std::ostringstream out;
    out << s;
    EXPECT_EQ(out.str(), "{a, b, {c}}");

    std::istringstream in("  {x, {y, z}}  {q}");
    CantorSet r1, r2;
    in >> r1 >> r2;
    EXPECT_EQ(r1, CantorSet("{x, {y, z}}"));
    EXPECT_EQ(r2, CantorSet("{q}"));
}

TEST(Stream, ReadFailures) {
    for (const char* text : {"", "abc", "{a, ", "{a,}"}) {
        std::istringstream in(text);
        CantorSet keep("{keep}"), s = keep;
        in >> s;
        EXPECT_TRUE(in.fail()) << text;
        EXPECT_EQ(s, keep) << text;
    }
}