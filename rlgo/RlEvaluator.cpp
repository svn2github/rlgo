//----------------------------------------------------------------------------
/** @file RlEvaluator.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlEvaluator.h"

#include "GoBoard.h"
#include "RlBinaryFeatures.h"
#include "RlMoveFilter.h"
#include "RlUtils.h"
#include "RlState.h"
#include "RlWeightSet.h"

using namespace boost;
using namespace std;
using namespace RlShapeUtil;

IMPLEMENT_OBJECT(RlEvaluator);

RlEvaluator::RlEvaluator(GoBoard& board,         
    RlBinaryFeatures* featureset, 
    RlWeightSet* weightset,
    RlMoveFilter* filter)
:   RlAutoObject(board),
    m_featureSet(featureset),
    m_weightSet(weightset),
    m_moveFilter(filter),
    m_differences(false)
{
}

void RlEvaluator::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 6, 6);

    settings >> RlSetting<RlBinaryFeatures*>("FeatureSet", m_featureSet);
    settings >> RlSetting<RlWeightSet*>("WeightSet", m_weightSet);
    settings >> RlSetting<RlMoveFilter*>("MoveFilter", m_moveFilter);
    settings >> RlSetting<bool>("Differences", m_differences);    
}

void RlEvaluator::Initialise()
{
    m_featureSet->EnsureInitialised();
    m_weightSet->EnsureInitialised();
    
    // Tracker map ensures that each feature creates just one tracker
    map<RlBinaryFeatures*, RlTracker*> trackermap;
    m_tracker = m_featureSet->CreateTracker(trackermap);
    m_tracker->Initialise();    
}

void RlEvaluator::Reset()
{
    // Resets agent's internal state, but not the board
    // Called at the start of a new search, before simulation
    m_eval = 0;

    // Reset move filter first (as it may be used by trackers during reset)
    if (m_moveFilter)
        m_moveFilter->Reset();        

    if (m_differences)
        m_dirty.MarkAll(m_board);

    m_tracker->DoReset();
    AddWeights(m_tracker->ChangeList(), m_eval);
}

void RlEvaluator::Execute(SgMove move, SgBlackWhite colour, bool real)
{
    // When a real move is executed, fully update internal representation
    if (real)
    {
        if (m_tracker->MarkSet())
            throw SgException("Can't set mark for real execution");
        Reset();
    }

    // When a simulated move is executed, update incrementally
    else
    {
        m_tracker->DoExecute(move, colour);
        AddWeights(m_tracker->ChangeList(), m_eval);

        if (m_differences)
            m_tracker->UpdateDirty(move, colour, m_dirty);
        if (m_moveFilter)
            m_moveFilter->Execute(move, colour);        
    }
}

void RlEvaluator::PlayExecute(SgMove move, SgBlackWhite colour, bool real)
{
    m_board.Play(move, colour);
    Execute(move, colour, real);
}

void RlEvaluator::Undo(bool real)
{
    // When a real move is undone, fully update internal representation
    if (real)
        Reset();

    // When a simulated move is undone, update incrementally
    else
    {
        if (!m_tracker->SupportUndo())
            throw SgException("Undo not supported for current features");
            
        m_tracker->DoUndo();
        SubWeights(m_tracker->ChangeList(), m_eval);
        
        if (m_differences)
            m_dirty.MarkAll(m_board); // @todo: could do something smarter here
        if (m_moveFilter)
            m_moveFilter->Undo();
    }
}

void RlEvaluator::TakeBackUndo(bool real)
{
    m_board.Undo();
    Undo(real);
}

void RlEvaluator::AddWeights(const RlChangeList& changes, RlFloat& eval)
{
    for (RlChangeList::Iterator i_changes(changes); i_changes; ++i_changes)
    {
        RlWeight& weight = m_weightSet->Get(i_changes->m_featureIndex);
        eval += weight.Weight() * i_changes->m_occurrences;
    }
}

void RlEvaluator::SubWeights(const RlChangeList& changes, RlFloat& eval)
{
    for (RlChangeList::Iterator i_changes(changes); i_changes; ++i_changes)
    {
        RlWeight& weight = m_weightSet->Get(i_changes->m_featureIndex);
        eval -= weight.Weight() * i_changes->m_occurrences;
    }
}

RlFloat RlEvaluator::EvaluateMove(SgMove move, SgBlackWhite colour)
{
    if (m_differences)
        return EvalMoveDiffs(move, colour);
    else
        return EvalMoveSimple(move, colour);
}

RlFloat RlEvaluator::EvalMoveSimple(SgMove move, SgBlackWhite colour)
{
    RlFloat weightchange;

    m_board.Play(move, colour);

    m_tracker->DoEvaluate(move, colour);
    AddWeights(m_tracker->ChangeList(), weightchange);
    m_board.Undo();
    return m_eval + weightchange;
}

RlFloat RlEvaluator::EvalMoveDiffs(SgMove move, SgBlackWhite colour)
{    
    // Add differences to current evaluation (recalculate where necessary)
    RlFloat weightchange;
    if (m_dirty.IsDirty(move, colour))
    {
        m_board.Play(move, colour);
        m_tracker->DoEvaluate(move, colour);
        AddWeights(m_tracker->ChangeList(), weightchange);
        m_diffs[BWIndex(colour)][move] = weightchange;
        m_dirty.Clear(move, colour);
        m_dirty.IncPruned(false);
        m_board.Undo();
    }
    else
    {
        weightchange = m_diffs[BWIndex(colour)][move];
        m_dirty.IncPruned(true);
    }
    
    return m_eval + weightchange;
}

void RlEvaluator::FindBest(RlState& state)
{
    static SgMove ties[SG_MAX_MOVES];
    int numties = 0;
    SgBlackWhite colour = state.m_colour;
    state.m_bestMove = SG_PASS;
    state.m_bestEval = colour == SG_BLACK ? -RlInfinity : RlInfinity;

    for (RlMoveFilter::Iterator i_filter(*m_moveFilter, colour); 
            i_filter; ++i_filter)
    {
        SgMove move = *i_filter;
        RlFloat eval = EvaluateMove(move, colour);
        if ((colour == SG_BLACK && eval >= state.m_bestEval)
            || (colour == SG_WHITE && eval <= state.m_bestEval))
        {
            if (eval != state.m_bestEval)
                numties = 0;
            state.m_bestMove = move;
            state.m_bestEval = eval;
            ties[numties++] = move;
        }
    }
    
    // Random tie-breaking
    if (numties > 1)
    {
        int index = SgRandom::Global().Int(numties);
        state.m_bestMove = ties[index];
    }
}

void RlEvaluator::SetMark()
{
    m_tracker->SetMark();
}

void RlEvaluator::ClearMark()
{
    m_tracker->ClearMark();
}

void RlEvaluator::RefreshValue(RlState& state)
{
    RlFloat eval = 0;
    for (RlActiveSet::Iterator i_active(state.Active()); 
        i_active; ++i_active)
    {
        RlWeight& weight = m_weightSet->Get(i_active->m_featureIndex);
        eval += weight.Weight() * i_active->m_occurrences;
    }

    state.SetEval(eval);
}

int RlEvaluator::GetActiveSize() const
{
    SG_ASSERT(IsInitialised());
    return m_tracker->GetActiveSize();
}

void RlEvaluator::PrintDirty(ostream& ostr)
{
    for (int c = SG_BLACK; c <= SG_WHITE; ++c)
    {
        ostr << "Dirty points for " << SgBW(c) << ":\n";
        for (int j = m_board.Size(); j >= 1; --j)
        {
            for (int i = 1; i <= m_board.Size(); ++i)
                ostr << (IsDirty(SgPointUtil::Pt(i, j), c) ? "*" : " ");
            ostr << "\n";
        }
    }
}

void RlEvaluator::EnsureSimple()
{
    if (m_differences)
        throw SgException("Differences must be disabled for simple operation");
}

//----------------------------------------------------------------------------

void RlMoveSorter::Sort(RlEvaluator* evaluator, SgBlackWhite toplay)
{
    m_evals.clear();
    
    // Evaluate all legal moves
    for (RlMoveFilter::Iterator i_moves(*evaluator->GetMoveFilter(), toplay);
            i_moves; ++i_moves)
    {
        SgMove move = *i_moves;
        RlFloat eval = evaluator->EvaluateMove(move, toplay);
        m_evals.push_back(EvalPair(move, eval));
    }
    
    SortMoves(toplay);
}

void RlMoveSorter::SortMoves(SgBlackWhite toplay)
{
    // Sort moves so that best moves are first
    if (toplay == SG_BLACK)
        sort(m_evals.begin(), m_evals.end(), greater<EvalPair>());
    else
        sort(m_evals.begin(), m_evals.end(), less<EvalPair>());
}

int RlMoveSorter::GetRank(SgMove move) const
{
    for (int i = 0; i < ssize(m_evals); ++i)
        if (m_evals[i].m_move == move)
            return i;
            
    return -1;
}

RlFloat RlMoveSorter::GetProportionalRank(SgMove move) const
{
    if (GetNumMoves() > 0)
        return (RlFloat) GetRank(move) / (RlFloat) GetNumMoves();
    else
        return 0.0;
}

//----------------------------------------------------------------------------
