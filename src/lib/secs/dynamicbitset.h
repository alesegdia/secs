#pragma once

#include <vector>
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace secs
{

/**
 * @class DynamicBitset
 * @brief A dynamically-sized bitset stored as 64-bit words.
 *
 * The capacity grows on demand: calling Set() on an index that does not yet exist
 * resizes the underlying storage. Bits beyond the current size are implicitly false,
 * so bitsets of different sizes compare and combine as if padded with zeros.
 *
 * Queries (Test, IsSubsetOf, ==) never allocate; they work a word at a time.
 */
class DynamicBitset
{
public:

    using word_t = std::uint64_t;
    static constexpr size_t BitsPerWord = 64;

    DynamicBitset() = default;

    /**
     * @brief Sets the bit at the given position to the given value. Resizes if necessary.
     * @param pos   The bit position.
     * @param value The value to assign (default: true).
     */
    void Set(size_t pos, bool value = true)
    {
        const size_t word = pos / BitsPerWord;
        if (word >= m_words.size())
        {
            if (!value)
                return; // already implicitly false — no resize needed
            m_words.resize(word + 1, 0);
        }
        const word_t mask = word_t(1) << (pos % BitsPerWord);
        if (value)
            m_words[word] |= mask;
        else
            m_words[word] &= ~mask;
    }

    /**
     * @brief Clears the bit at the given position. No-op if pos is out of range.
     * @param pos The bit position.
     */
    void Clear(size_t pos)
    {
        Set(pos, false);
    }

    /**
     * @brief Tests the bit at the given position.
     * @param pos The bit position.
     * @return The value of the bit, or false if pos is out of range.
     */
    bool Test(size_t pos) const
    {
        const size_t word = pos / BitsPerWord;
        if (word >= m_words.size())
            return false;
        return (m_words[word] >> (pos % BitsPerWord)) & word_t(1);
    }

    /// Read-only indexed access. Returns false if pos is out of range.
    bool operator[](size_t pos) const
    {
        return Test(pos);
    }

    /**
     * @brief Resets all bits to false without releasing storage.
     */
    void Reset()
    {
        std::fill(m_words.begin(), m_words.end(), word_t(0));
    }

    /**
     * @brief Returns true if every bit set in this bitset is also set in other.
     *        Allocation-free; used on the hot path to match entities against systems.
     */
    bool IsSubsetOf(const DynamicBitset& other) const
    {
        const size_t common = std::min(m_words.size(), other.m_words.size());
        for (size_t i = 0; i < common; ++i)
        {
            if ((m_words[i] & ~other.m_words[i]) != 0)
                return false;
        }
        for (size_t i = common; i < m_words.size(); ++i)
        {
            if (m_words[i] != 0)
                return false;
        }
        return true;
    }

    DynamicBitset operator|(const DynamicBitset& other) const
    {
        DynamicBitset result = *this;
        result |= other;
        return result;
    }

    DynamicBitset& operator|=(const DynamicBitset& other)
    {
        if (other.m_words.size() > m_words.size())
            m_words.resize(other.m_words.size(), 0);
        for (size_t i = 0; i < other.m_words.size(); ++i)
            m_words[i] |= other.m_words[i];
        return *this;
    }

    DynamicBitset operator&(const DynamicBitset& other) const
    {
        DynamicBitset result;
        const size_t common = std::min(m_words.size(), other.m_words.size());
        result.m_words.resize(common, 0);
        for (size_t i = 0; i < common; ++i)
            result.m_words[i] = m_words[i] & other.m_words[i];
        return result;
    }

    bool operator==(const DynamicBitset& other) const
    {
        const size_t common = std::min(m_words.size(), other.m_words.size());
        for (size_t i = 0; i < common; ++i)
        {
            if (m_words[i] != other.m_words[i])
                return false;
        }
        const auto& longer = m_words.size() > other.m_words.size() ? m_words : other.m_words;
        for (size_t i = common; i < longer.size(); ++i)
        {
            if (longer[i] != 0)
                return false;
        }
        return true;
    }

    bool operator!=(const DynamicBitset& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Number of addressable bits currently stored (a multiple of BitsPerWord).
     */
    size_t Size() const
    {
        return m_words.size() * BitsPerWord;
    }

private:
    std::vector<word_t> m_words;
};

} // namespace secs
