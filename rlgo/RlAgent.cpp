//----------------------------------------------------------------------------
/** @file RlAgent.cpp
    @see RlAgent.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlAgent.h"

#include "RlAgentLog.h"
#include "RlBinaryFeatures.h"
#include "RlWeightSet.h"
#include "RlEvaluator.h"
#include "RlLearningRule.h"
#include "RlMoveFilter.h"
#include "RlPolicy.h"
#include "RlSetup.h"
#include "RlSimulator.h"
#include "RlTrace.h"
#include "RlTrainer.h"
#include "GoGame.h"

#include <boost/filesystem/convenience.hpp>

using namespace boost;
using namespace std;
using namespace RlMathUtil;
using namespace RlMoveUtil;
using namespace RlPathUtil; 
using namespace SgPointUtil;

//----------------------------------------------------------------------------

RlAgent::RlAgent(
    GoBoard& board, 
    RlPolicy* policy,
    RlEvaluator* evaluator,
    RlBinaryFeatures* featureset,
    RlWeightSet* weightset,
    RlHistory* history,
    RlTrainer* trainer)
:   RlAutoObject(board),
    m_policy(policy),
    m_evaluator(evaluator),
    m_featureSet(featureset),
    m_weightSet(weightset),
    m_history(history),
    m_trainer(trainer),
    m_log(0),
    m_resignThreshold(0),
    m_prune(false)
{
}

void RlAgent::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 22, 22);
    settings >> RlSetting<RlPolicy*>("Policy", m_policy);
    settings >> RlSetting<RlEvaluator*>("Evaluator", m_evaluator);
    settings >> RlSetting<RlBinaryFeatures*>("FeatureSet", m_featureSet);
    settings >> RlSetting<RlWeightSet*>("WeightSet", m_weightSet);
    settings >> RlSetting<RlHistory*>("History", m_history);
    settings >> RlSetting<RlTrainer*>("Trainer", m_trainer);
    settings >> RlSetting<RlAgentLog*>("Log", m_log);
    settings >> RlSetting<RlFloat>("ResignThreshold", m_resignThreshold);
    settings >> RlSetting<bool>("Prune", m_prune);
}

void RlAgent::Initialise()
{
    if (m_policy)
        m_policy->EnsureInitialised();
    if (m_evaluator)
        m_evaluator->EnsureInitialised();
    if (m_featureSet)
        m_featureSet->EnsureInitialised();
    if (m_history)
        m_history->EnsureInitialised();
    if (m_trainer)
        m_trainer->EnsureInitialised();

    m_history->Resize(m_evaluator->GetActiveSize());
}

void RlAgent::NewGame()
{
    m_timestep = 0;
    m_history->NewEpisode();
    m_history->AddState(0, m_board.ToPlay());

    m_featureSet->Start();
    if (m_prune)
        m_featureSet->Prune();
    // @todo: reset pruned features to zero weight

    // Reset incremental tracking to current position
    m_evaluator->Reset();
    
    // Set value and active features for initial position
    GetState().SetEval(m_evaluator->Eval());
    SetActive();

    if (m_log)
    {
        m_log->StartGame();
        if (m_log->LogIsActive())
        {
            m_log->LogWeights(); // once per game
            m_log->Debug(RlSetup::VOCAL) << "Starting new game\n";
            m_log->PrintBoard();
            m_log->PrintValue();
        }
    }
}

RlFloat RlAgent::EndGame(bool resign, bool train)
{
    // Score game
    RlFloat score = Score(resign);
    m_history->TerminateEpisode(score);

    // Learn from this episode
    if (m_trainer && train)
        m_trainer->Train();

    m_featureSet->End();

    // Log the following once per game, at the end:
    if (m_log && m_log->LogIsActive())
    {
        m_log->LogStep();
        m_log->EndGame();
        m_log->LogGame();
        m_log->SaveRecord();
        m_log->SaveWeights();
        m_log->TopTex();
    }
    
    return score;
}

SgMove RlAgent::SelectMove()
{
    // Select move to play
    SgMove move = m_policy->SelectMove(GetState());
    SG_ASSERT(m_board.IsLegal(move));

    if (m_resignThreshold > 0)
    {
        RlFloat pwin = GetProbability(move, m_board.ToPlay());
        if (CheckResign(pwin))
            return SG_RESIGN;
    }

    return move;
}

void RlAgent::Execute(SgMove move, SgBlackWhite colour, bool updateboard)
{
    // Move is executed in evaluator by real or simulated agent

    SG_UNUSED(move);
    SG_UNUSED(colour);
    SG_UNUSED(updateboard);

    if (m_log && m_log->StepLogIsActive())
    {
        m_log->LogStep();
        m_log->LogEval();
    }

    m_timestep++;
    if (m_timestep > RL_MAX_TIME)
        throw SgException("Timestep has exceeded max moves");

    // Store active features in new position
    m_history->AddState(m_timestep, m_board.ToPlay());
    GetState().SetEval(m_evaluator->Eval());
    SetActive();

    if (m_log && m_log->StepLogIsActive())
    {
        m_log->PrintBoard();
        m_log->PrintValue();
    }
}

void RlAgent::Undo(bool updateboard)
{
    SG_UNUSED(updateboard);
    if (m_timestep <= 0)
        throw SgException("Attempted Undo when timestep <= 0");
    m_history->Truncate(m_timestep);
    m_timestep--;
}

void RlAgent::Think()
{
}

void RlAgent::Ponder()
{
}

void RlAgent::SetActive()
{
    RlState& state = GetState();
    state.SetActive(m_evaluator->Active());
}

void RlAgent::SetMove(SgMove move)
{
    // Move is associated with state in which it is executed
    GetState().SetMove(move);
    if (m_policy)
        m_policy->SetCurrentMove(GetState());
}

RlFloat RlAgent::GetProbability(SgMove move, SgBlackWhite colour) const
{
    // Estimate probability of Black winning the game, after selecting move
    RlFloat value;
    if (!m_policy->SearchValue(value))
        value = m_evaluator->EvaluateMove(move, colour);

    if (m_board.ToPlay() == SG_WHITE)
        value = -value;
    return Logistic(value);
}

bool RlAgent::CheckResign(RlFloat pwin) const
{
    // If we would have less than threshold % chance of winning,
    // after making our selected move, then resign.
    if (pwin < m_resignThreshold)
    {
        if (m_log)
            m_log->Debug(RlSetup::QUIET) 
                << "Probability of winning (" << pwin * 100
                << "%) is below resignation threshold\n";
        return true;
    }
    return false;
}

RlFloat RlAgent::Score(bool resign) const
{
    static RlFloat epsilon = std::numeric_limits<RlFloat>::epsilon();

    if (m_log && m_log->LogIsActive())
        m_log->Debug(RlSetup::VOCAL) << m_board << "Game over: ";

    RlFloat blackwin;
    if (resign)
    {
        if (m_board.ToPlay() == SG_BLACK)
        {
            blackwin = 0;
            if (m_log && m_log->LogIsActive())
                m_log->Debug(RlSetup::VOCAL) << "Black resigns\n";
        }
        else
        {
            blackwin = 1;
            if (m_log && m_log->LogIsActive())
                m_log->Debug(RlSetup::VOCAL) << "White resigns\n";
        }
    }
    else
    {
        // The implementation of ScoreSimpleEndPosition in Fuego 0.4 is buggy
        // the following code corrects the score computation
        RlFloat komi = m_board.Rules().Komi().ToFloat();
        RlFloat whitescore = GoBoardUtil::ScoreSimpleEndPosition(
            m_board, 0, true);
        RlFloat blackscore = RlPoint()->NumPoints() - whitescore;
        if (m_log && m_log->LogIsActive())
            m_log->Debug(RlSetup::VOCAL) << "B " << blackscore 
                << ", W " << whitescore << ", komi " << komi << "\n";
        if (blackscore > whitescore + komi + epsilon)
        {
            blackwin = 1;
            if (m_log && m_log->LogIsActive())
                m_log->Debug(RlSetup::VOCAL) << "Black wins\n";
        }
        else if (blackscore < whitescore + komi - epsilon)
        {
            blackwin = 0;
            if (m_log && m_log->LogIsActive())
                m_log->Debug(RlSetup::VOCAL) << "White wins\n";
        }
        else
        {
            if (m_log && m_log->LogIsActive())
                m_log->Debug(RlSetup::VOCAL) << "Draw\n";
            blackwin = 0.5;
        }
    }
            
    return blackwin;
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlRealAgent);

RlRealAgent::RlRealAgent(
    GoBoard& board,
    RlPolicy* policy,
    RlEvaluator* evaluator,
    RlBinaryFeatures* featureset,
    RlWeightSet* weightset,
    RlHistory* history,
    RlTrainer* trainer,
    RlSimulator* simulator)
  :   RlAgent(board, policy, evaluator, featureset, weightset, 
        history, trainer),
    m_simulator(simulator),
    m_resetWeights(RL_RESET_ON_NEWGAME)
{
}

void RlRealAgent::LoadSettings(istream& settings)
{
    RlAgent::LoadSettings(settings);
    settings >> RlSetting<RlSimulator*>("Simulator", m_simulator);
    settings >> RlSetting<string>("WeightFile", m_weightFile);
    settings >> RlSetting<int>("ResetWeights", m_resetWeights);
    settings >> RlSetting<RlFloat>("MinWeight", m_minWeight);
    settings >> RlSetting<RlFloat>("MaxWeight", m_maxWeight);
}

void RlRealAgent::Initialise()
{
    RlAgent::Initialise();

    if (m_simulator)
        m_simulator->EnsureInitialised();

    if (m_resetWeights == RL_RESET_ON_INIT)
    {
        m_featureSet->Clear();
        InitWeights();
    }
}

void RlRealAgent::NewGame()
{
    if (m_resetWeights == RL_RESET_ON_NEWGAME)
    {
        m_featureSet->Clear();        
        InitWeights();
    }

    RlAgent::NewGame();

    if (m_simulator)
        m_simulator->SetReady(true);
}

RlFloat RlRealAgent::EndGame(bool resign, bool train)
{
    if (m_simulator)
        m_simulator->SetReady(false);

    return RlAgent::EndGame(resign, train);
}

SgMove RlRealAgent::SelectMove()
{
    Think();
    return RlAgent::SelectMove();
}

void RlRealAgent::Execute(SgMove move, SgBlackWhite colour, bool updateboard)
{
    SetMove(move);
    if (updateboard)
        m_evaluator->PlayExecute(move, colour, true);
    else
        m_evaluator->Execute(move, colour, true);

    RlAgent::Execute(move, colour, updateboard);
}

void RlRealAgent::Undo(bool updateboard)
{
    if (updateboard)
        m_evaluator->TakeBackUndo(true);
    else
        m_evaluator->Undo(true);
    RlAgent::Undo(updateboard);
}

void RlRealAgent::Think()
{
    if (!m_simulator)
        return;

    // Clear simulation history
    m_simulator->GetAgent()->GetHistory()->Clear();
    
    // Simulate games of self-play
    m_simulator->Simulate();

    // New features may have been discovered; weights may have changed
    // Reset the evaluator to use new features and weights
    m_evaluator->Reset();
}

void RlRealAgent::Ponder()
{
    if (m_simulator)
        m_simulator->Ponder();
}

void RlRealAgent::InitWeights()
{
    // @todo: currently weights MUST be initialised to zero
    // unless a fully connected architecture is used.
    // Otherwise unused weights will be assumed to be zero during updates.

    RlDebug(RlSetup::VOCAL) << "Resetting weights...";
    m_weightSet->RandomiseWeights(m_minWeight, m_maxWeight);
    RlDebug(RlSetup::VOCAL) << " done\n";

    if (!m_weightFile.empty() && m_weightFile != "NULL")
    {
        bfs::path wpath = bfs::complete(m_weightFile, GetInputPath());
        Load(wpath);
    }
}

void RlAgent::Load(const bfs::path& filename)
{
    bfs::ifstream weightfile(filename);
    if (!weightfile)
        throw SgException("Failed to load weight file " 
            + filename.native_file_string());
    RlDebug(RlSetup::VOCAL) << "Loading weights...";
    m_featureSet->LoadData(weightfile);
    m_weightSet->Load(weightfile);
    RlDebug(RlSetup::VOCAL) << " done\n";
}

void RlAgent::Save(const bfs::path& filename)
{
    bfs::ofstream weightfile(filename);
    if (!weightfile)
        throw SgException("Failed to save weight file" 
            + filename.native_file_string());
    RlDebug(RlSetup::VOCAL) << "Saving weights...";
    m_featureSet->SaveData(weightfile);
    m_weightSet->Save(weightfile);
    RlDebug(RlSetup::VOCAL) << " done\n";
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSimAgent);

RlSimAgent::RlSimAgent(
    GoBoard& board, 
    RlPolicy* policy,
    RlEvaluator* evaluator,
    RlBinaryFeatures* featureset,
    RlWeightSet* weightset,
    RlHistory* history,
    RlTrainer* trainer)
:   RlAgent(board, policy, evaluator, featureset, weightset, history, trainer)
{
}

RlFloat RlSimAgent::EndGame(bool resign, bool train)
{
    RlFloat result = RlAgent::EndGame(resign, train);    
    return result;
}

void RlSimAgent::Execute(SgMove move, SgBlackWhite colour, bool updateboard)
{
    SetMove(move);
    if (updateboard)
        m_evaluator->PlayExecute(move, colour, false);
    else
        m_evaluator->Execute(move, colour, false);
    RlAgent::Execute(move, colour, updateboard);
}

void RlSimAgent::Undo(bool updateboard)
{
    if (updateboard)
        m_evaluator->TakeBackUndo(false);
    else
        m_evaluator->Undo(false);
    RlAgent::Undo(updateboard);
}

//----------------------------------------------------------------------------
