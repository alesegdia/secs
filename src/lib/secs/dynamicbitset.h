#pragma once

#include <vector>
#include <algorithm>
#include <cstddef>

namespace secs
{

/**
 * @class DynamicBitset
 * @brief A dynamically-sized bitset backed by std::vector<bool>.
 *
 * NOTE: std::vector<bool> is a space-optimized specialization mandated by the C++
 * standard that stores exactly 1 bit per element — the same storage density as
 * std::bitset. Unlike std::bitset, the capacity grows on demand: calling Set() on
 * an index that does not yet exist will automatically resize the underlying storage.
 * Bits beyond the current size are implicitly false.
 */
class DynamicBitset
{
public:

    DynamicBitset() = default;

    /**
     * @brief Sets the bit at the given position to the given value. Resizes if necessary.
     * @param pos   The bit position.
     * @param value The value to assign (default: true).
     */
    void Set(size_t pos, bool value = true)
    {
        if (pos >= m_bits.size())
        {
            if (!value)
                return; // already implicitly false — no resize needed
            m_bits.resize(pos + 1, false);
        }
        m_bits[pos] = value;
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
        if (pos >= m_bits.size())
            return false;
        return m_bits[pos];
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
        std::fill(m_bits.begin(), m_bits.end(), false);
    }

    DynamicBitset operator|(const DynamicBitset& other) const
    {
        DynamicBitset result;
        size_t max_size = std::max(m_bits.size(), other.m_bits.size());
        result.m_bits.resize(max_size, false);
        for (size_t i = 0; i < m_bits.size(); ++i)
            result.m_bits[i] = m_bits[i];
        for (size_t i = 0; i < other.m_bits.size(); ++i)
            result.m_bits[i] = bool(result.m_bits[i]) || bool(other.m_bits[i]);
        return result;
    }

    DynamicBitset& operator|=(const DynamicBitset& other)
    {
        if (other.m_bits.size() > m_bits.size())
            m_bits.resize(other.m_bits.size(), false);
        for (size_t i = 0; i < other.m_bits.size(); ++i)
            m_bits[i] = bool(m_bits[i]) || bool(other.m_bits[i]);
        return *this;
    }

    DynamicBitset operator&(const DynamicBitset& other) const
    {
        DynamicBitset result;
        size_t min_size = std::min(m_bits.size(), other.m_bits.size());
        result.m_bits.resize(min_size, false);
        for (size_t i = 0; i < min_size; ++i)
            result.m_bits[i] = bool(m_bits[i]) && bool(other.m_bits[i]);
        return result;
    }

    bool operator==(const DynamicBitset& other) const
    {
        size_t max_size = std::max(m_bits.size(), other.m_bits.size());
        for (size_t i = 0; i < max_size; ++i)
        {
            bool a = (i < m_bits.size())       ? bool(m_bits[i])       : false;
            bool b = (i < other.m_bits.size()) ? bool(other.m_bits[i]) : false;
            if (a != b)
                return false;
        }
        return true;
    }

    bool operator!=(const DynamicBitset& other) const
    {
        return !(*this == other);
    }

    size_t Size() const
    {
        return m_bits.size();
    }

private:
    std::vector<bool> m_bits;
};

} // namespace secs
