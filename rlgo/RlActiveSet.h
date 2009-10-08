//----------------------------------------------------------------------------
/** @file RlActiveSet.h
    A set of feature indices that is active at any time */
//----------------------------------------------------------------------------

#ifndef RLACTIVESET_H
#define RLACTIVESET_H

#include <vector>
#include "RlBinaryFeatures.h"
#include "RlMiscUtil.h"

#ifdef RL_FLOAT_OCCURRENCES
typedef RlFloat RlOccur;
inline void EnsureFloatOccurrences() { }
#else
typedef int RlOccur;
inline void EnsureFloatOccurrences()
{
    throw SgException("Floating point occurrences required");
}
#endif


struct RlActiveEntry
{
    RlActiveEntry(int featureindex = -1, RlOccur occurrences = 0)
    :   m_featureIndex(featureindex),
        m_occurrences(occurrences)
    { }
    
    void Clear()
    {
        m_featureIndex = -1;
        m_occurrences = 0;
    }
    
    int m_featureIndex;
    RlOccur m_occurrences;
};

struct RlChange
{
    RlChange()
    {
    }
    
    RlChange(int slot, int featureindex, RlOccur occurrences)
    :   m_slot(slot),
        m_featureIndex(featureindex),
        m_occurrences(occurrences)
    {
    }

    int m_slot;
    int m_featureIndex;
    RlOccur m_occurrences;
};

//----------------------------------------------------------------------------
/** A list of changes to be applied to an active set */
class RlChangeList
{
public:

    RlChangeList()
    :   m_capacity(0)
    {
    }

    int Size() const
    {
        return m_numChanges;
    }
    
    void Clear()
    {
        m_numChanges = 0;
    }

    void Change(const RlChange& change)
    {
        SG_ASSERT(m_numChanges <= m_capacity);
        if (m_numChanges == m_capacity)
        {
            m_changes.push_back(change);
            m_capacity++;
        }
        else
        {
            m_changes[m_numChanges] = change;
        }
        m_numChanges++;
    }
    
    void CopyList(const RlChangeList& changelist)
    {
        Clear();
        for (Iterator i_changelist(changelist); i_changelist; ++i_changelist)
            Change(*i_changelist);
    }
    
    void Display(RlBinaryFeatures* features, std::ostream& ostr) const
    {
        for (Iterator i_changes(*this); i_changes; ++i_changes)
        {
            features->DescribeFeature(i_changes->m_featureIndex, ostr);
            ostr << ": Slot = " << i_changes->m_slot << 
                ", Index = " << i_changes->m_featureIndex <<
                ", Occurrences = " << i_changes->m_occurrences << "\n";
        }
    }

    class Iterator
    {
    public:

        Iterator(const RlChangeList& changelist)
        :   m_changeList(&changelist),
            m_cursor(0)
        { 
        }
        
        const RlChange& operator*() const
        {
            return m_changeList->m_changes[m_cursor];
        }

        const RlChange* operator->() const
        {
            return &m_changeList->m_changes[m_cursor];
        }

        void operator++()
        {
            ++m_cursor;
        }

        operator bool() const
        {
            return m_cursor < m_changeList->m_numChanges;
        }
        
        static Iterator InvalidIterator()
        {
            return Iterator();
        }
        
    private:
    
        Iterator()
        :   m_changeList(0),
            m_cursor(0)
        {
        }

        const RlChangeList* m_changeList;
        int m_cursor;
    };

    class ReverseIterator
    {
    public:

        ReverseIterator(const RlChangeList& changelist)
        :   m_changeList(changelist),
            m_cursor(m_changeList.m_numChanges - 1)
        { 
        }
        
        const RlChange& operator*()
        {
            return m_changeList.m_changes[m_cursor];
        }

        const RlChange* operator->()
        {
            return &m_changeList.m_changes[m_cursor];
        }

        void operator++()
        {
            --m_cursor;
        }

        operator bool() const
        {
            return m_cursor >= 0;
        }
        
    private:

        const RlChangeList& m_changeList;
        int m_cursor;
    };
    
private:

    int m_capacity;
    int m_numChanges;
    std::vector<RlChange> m_changes;
    
friend class Iterator;
};

//----------------------------------------------------------------------------
/** A set of feature indices that is active at any time */
class RlActiveSet
{
public:

    RlActiveSet(int size = 0)
    :   m_totalActive(0)
    {
        m_entries.resize(size);
    }

    int Size() const 
    {
        return m_entries.size(); 
    }

    void Resize(int size)
    {
        m_entries.resize(size);
        Clear();
    }

    void Clear()
    {
        for (int i = 0; i < Size(); ++i)
            m_entries[i].Clear();
        m_totalActive = 0;
    }
    
    void Change(const RlChange& change)
    {
        SG_ASSERT(change.m_featureIndex >= 0);
        SG_ASSERT(change.m_slot >= 0 && change.m_slot < Size());
        RlActiveEntry& entry = m_entries[change.m_slot];
        SG_ASSERT(entry.m_featureIndex == change.m_featureIndex
            || entry.m_featureIndex == -1);
        SG_ASSERT((entry.m_featureIndex == -1 && entry.m_occurrences == 0)
                || (entry.m_featureIndex != -1 && entry.m_occurrences != 0));

        m_totalActive += change.m_occurrences;
        entry.m_occurrences += change.m_occurrences;
        if (entry.m_occurrences == 0)
            entry.m_featureIndex = -1;
        else
            entry.m_featureIndex = change.m_featureIndex;
    }
    
    bool IsActive(int slot) const
    {
        return m_entries[slot].m_featureIndex >= 0;
    }
        
    int GetFeatureIndex(int slot) const
    {
        return m_entries[slot].m_featureIndex;
    }
    
    RlOccur GetOccurrences(int slot) const
    {
        return m_entries[slot].m_occurrences;
    }
    
    RlOccur GetTotalActive() const
    {
        return m_totalActive;
    }
    
    void Display(RlBinaryFeatures* features, std::ostream& ostr) const
    {
        for (Iterator i_active(*this); i_active; ++i_active)
        {
            features->DescribeFeature(i_active->m_featureIndex, ostr);
            ostr << ": Slot = " << i_active.Slot() << 
                " Index = " << i_active->m_featureIndex <<
                ", Occurrences = " << i_active->m_occurrences << "\n";
        }
    }

    class Iterator
    {
    public:

        Iterator(const RlActiveSet& active, int startslot = 0)
        :   m_active(active),
            m_cursor(startslot)
        { 
            SkipEmpty();
        }
        
        const RlActiveEntry& operator*()
        {
            return m_active.m_entries[m_cursor];
        }

        const RlActiveEntry* operator->()
        {
            return &m_active.m_entries[m_cursor];
        }

        void operator++()
        {
            ++m_cursor;
            SkipEmpty();
        }

        operator bool() const
        {
            return m_cursor < m_active.Size();
        }
        
        int Slot() const
        {
            return m_cursor;
        }
        
    private:

        void SkipEmpty()
        {
            while (m_cursor < m_active.Size()
                && m_active.m_entries[m_cursor].m_featureIndex == -1)
            {
                ++m_cursor;
            }
        }
        

        const RlActiveSet& m_active;
        int m_cursor;
    };

protected:
    
    std::vector<RlActiveEntry> m_entries;
    RlOccur m_totalActive;

friend class Iterator;
};

//----------------------------------------------------------------------------

#endif // RLACTIVESET_H

