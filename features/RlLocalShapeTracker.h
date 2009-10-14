//----------------------------------------------------------------------------
/** @file RlLocalShapeTracker.h
    Tracker for local shapes
*/
//----------------------------------------------------------------------------

#ifndef RLLOCALSHAPETRACKER_H
#define RLLOCALSHAPETRACKER_H

#include "RlTracker.h"

class RlLocalShapeFeatures;

//----------------------------------------------------------------------------
/** Tracker for local shape features */
class RlLocalShapeTracker : public RlTracker
{
public:

    RlLocalShapeTracker(GoBoard& board, RlLocalShapeFeatures* shapes,
        bool successorFile = true);
    ~RlLocalShapeTracker(); 

    /** Reset to current board position */
    virtual void Reset();
    
    /** Incremental execute */
    virtual void Execute(SgMove move, SgBlackWhite colour, bool execute);

    /** Incremental undo */
    virtual void Undo();

    /** Update dirty moves */
    virtual void UpdateDirty(SgMove move, SgBlackWhite colour, 
        RlDirtySet& dirty);

    /** Remember current position for fast resets */
    virtual void SetMark();

    /** Size of active set */
    virtual int GetActiveSize() const;

    /** Lookup successor from table */
    int GetSuccessor(int index, int x, int y, int c) const;

    /** Verify that all indices correctly correspond to board */
    void Verify() const;

    /** Local shapes support undo */
    virtual bool SupportUndo() const { return true; }

    /** Delete successor file */
    void DeleteSuccessorFile();

protected:

    /** Update all changes for specified move */
    void MakeMove(SgMove move, SgBlackWhite colour, bool execute);

    /** Main function to update representation for one changed stone */
    void UpdateStone(SgPoint stone, SgBlackWhite colour, bool execute);
    
    /** Lookup local move index */
    int GetLocalMove(int x, int y, int c) const;

    /** Lookup successor from table using local move index */
    int GetSuccessor(int index, int localmove) const;
        
    void UpdateDirty(SgPoint stone, RlDirtySet& dirty);
    int GetOffset(SgPoint anchor) const;
    void Store(SgPoint point);
    void MakeSuccessors();
    void MakeLocalMoves();
    bfs::path GetFileName();
    bool LoadSuccessors();
    bool SaveSuccessors();

protected:

    /** Shape features to track */
    RlLocalShapeFeatures* m_shapes;
    
    /** Whether to load/save successors to a file */
    bool m_successorFile;

    SgArray<int, SG_MAXPOINT> m_index;
    SgArray<int, SG_MAXPOINT> m_markIndex;
    int* m_successor;
    int m_numLocal;
    int m_numEntries;

    struct Change
    {
        Change(int step, SgPoint anchor, int index)
        :   m_step(step), m_anchor(anchor), m_index(index) { }
        
        int m_step;
        SgPoint m_anchor;
        int m_index;
    };
    
    std::vector<Change> m_changes;
    int m_step;
    
    struct LocalMove
    {
        SgPoint m_anchor;
        int m_localMove;
    };

    std::vector<LocalMove> m_localMoves[3][SG_MAXPOINT];
};

//----------------------------------------------------------------------------

#endif // RLLOCALSHAPETRACKER_H
