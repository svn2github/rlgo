//----------------------------------------------------------------------------
/** @file RlAgent.h
    An agent represents the top-level API for RLGO
*/
//----------------------------------------------------------------------------

#ifndef RLAGENT_H
#define RLAGENT_H

#include "RlActiveSet.h"
#include "RlFactory.h"
#include "RlHistory.h"
#include "RlSetup.h"
#include "RlTrace.h"
#include "SgBoardColor.h"
#include "SgDebug.h"
#include "SgRect.h"
#include <boost/filesystem/fstream.hpp>
#include <list>
#include <vector>

namespace bfs = boost::filesystem;

class RlAgentLog;
class RlBinaryFeatures;
class RlWeightSet;
class RlEvaluator;
class RlHistory;
class RlLog;
class RlPolicy;
class RlSimulator;
class RlTrainer;

//----------------------------------------------------------------------------
/** Top-level API for RLGO
*/
class RlAgent : public RlAutoObject
{
public:

    RlAgent(GoBoard& board,
        RlPolicy* policy = 0,
        RlEvaluator* evaluator = 0,
        RlBinaryFeatures* featureset = 0,
        RlWeightSet* weightset = 0,
        RlHistory* history = 0,
        RlTrainer* trainer = 0);
                
    /** Load in the settings for this agent */
    virtual void LoadSettings(std::istream& settings);
    
    /** Initialise agent */
    virtual void Initialise();

    /** Begin a new game (only ignore for startup) */
    virtual void NewGame();
    
    /** End game by resignation or by normal termination. 
        Updates history and calls trainer. Returns result. */
    virtual RlFloat EndGame(bool resign, bool train = true);

    /** Choose epsilon-greedy move (before move is made) */
    virtual SgMove SelectMove();

    /** Update features after executing a move by either player 
        This does not touch the weights 
        @param move
        @param colour
        @param updateboard Whether move should be played on board */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool updateboard = true);

    /** Update features after undoing a move by either player 
        This does not touch the weights
        @param updateboard Whether move should be undone on board */
    virtual void Undo(bool updateboard = true);

    /** Think during agent's time */
    virtual void Think();
    
    /** Ponder during opponent's time */
    virtual void Ponder();

    void Load(const bfs::path& filename);
    void Save(const bfs::path& filename);
    
    /** Accessor functions */
    //@todo: improve encapsulation
    GoBoard& Board() { return m_board; }
    RlBinaryFeatures* GetFeatureSet() const { return m_featureSet;}
    RlWeightSet* GetWeightSet() const { return m_weightSet;}
    RlPolicy* GetPolicy() { return m_policy; } //@todo: const
    RlEvaluator* GetEvaluator() { return m_evaluator; }
    RlHistory* GetHistory() { return m_history; }
    RlTrainer* GetTrainer() { return m_trainer; }
    int GetTimeStep() const { return m_timestep; }
    RlState& GetState() { return m_history->GetState(m_timestep); }
    RlAgentLog* GetLog() { return m_log; }

    void SetPolicy(RlPolicy* policy) { m_policy = policy; }

protected:

    /** Set played move in state */
    virtual void SetMove(SgMove move);

    /** Set active features in state */
    void SetActive();
    
    /** Get the probability of winning the game after selecting move */
    RlFloat GetProbability(SgMove move, SgBlackWhite colour) const;
    
    /** Check value after move is below the resignation threshold */
    bool CheckResign(RlFloat pwin) const;

    /** Score final position */
    RlFloat Score(bool resign) const;

protected:

    RlPolicy* m_policy;
    RlEvaluator* m_evaluator;
    RlBinaryFeatures* m_featureSet;
    RlWeightSet* m_weightSet;
    RlHistory* m_history;
    RlTrainer* m_trainer;
    RlAgentLog* m_log;

    /** Threshold at which to resign game */
    RlFloat m_resignThreshold;

    /** Whether to prune discovered features each game */
    bool m_prune;

    /** Current update timestep */
    int m_timestep;

friend class RlAgentLog;
};

//----------------------------------------------------------------------------
/** Agent class for dealing with real experience */
class RlRealAgent : public RlAgent
{
public:

    DECLARE_OBJECT(RlRealAgent);

    RlRealAgent(GoBoard& board,
        RlPolicy* policy = 0,
        RlEvaluator* evaluator = 0,
        RlBinaryFeatures* featureset = 0,
        RlWeightSet* weightset = 0,
        RlHistory* history = 0,
        RlTrainer* trainer = 0,
        RlSimulator* simulator = 0);
        
    /** Load in the settings for this agent */
    virtual void LoadSettings(std::istream& settings);
    
    /** Initialise agent */
    virtual void Initialise();

    /** Begin a new game */
    virtual void NewGame();
    
    /** End game by resignation or by normal termination. 
        Updates history and calls trainer. Returns result. */
    virtual RlFloat EndGame(bool resign, bool train = true);

    /** Choose move according to current policy. */
    virtual SgMove SelectMove();

    /** Update features after executing a move by either player 
        This does not touch the weights 
        @param move
        @param colour
        @param updateboard Whether move should be played on board */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool updateboard = true);

    /** Update features after undoing a move by either player 
        This does not touch the weights
        @param updateboard Whether move should be undone on board */
    virtual void Undo(bool updateboard = true);

    /** Think during agent's time */
    virtual void Think();

    /** Ponder during opponent's time */
    virtual void Ponder();

    /** Accessor functions */
    //@todo: improve encapsulation
    RlSimulator* GetSimulator() { return m_simulator; }

protected:

    /** Load and/or zero weights */
    void InitWeights();
    
protected:

    RlSimulator* m_simulator;

    /** Weights to load on new game */
    std::string m_weightFile;
    
    enum
    {
        RL_NEVER_RESET = 0,
        RL_RESET_ON_INIT = 1,
        RL_RESET_ON_NEWGAME = 2
    };
    
    /** Whether to reset weights (to zero or weightfile) on init or new game */
    int m_resetWeights;
    
    /** Min and max values to reset weights */
    RlFloat m_minWeight, m_maxWeight;

friend class RlAgentLog;
};

//----------------------------------------------------------------------------
/** Agent class for dealing with simulated experience */
class RlSimAgent : public RlAgent
{
public:

    DECLARE_OBJECT(RlSimAgent);

    RlSimAgent(GoBoard& board,
        RlPolicy* policy = 0,
        RlEvaluator* evaluator = 0,
        RlBinaryFeatures* featureset = 0,
        RlWeightSet* weightset = 0,
        RlHistory* history = 0,
        RlTrainer* trainer = 0);
        
    /** End game by resignation or by normal termination. 
        Updates history and calls trainer. Returns result. */
    virtual RlFloat EndGame(bool resign, bool train = true);

    /** Update features after executing a move by either player 
        This does not touch the weights 
        @param move
        @param colour
        @param updateboard Whether move should be played on board */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool updateboard = true);

    /** Update features after undoing a move by either player 
        This does not touch the weights
        @param updateboard Whether move should be undone on board */
    virtual void Undo(bool updateboard = true);
};

//----------------------------------------------------------------------------

#endif // RLAGENT_H

