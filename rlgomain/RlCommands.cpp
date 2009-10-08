//----------------------------------------------------------------------------
/** @file RlCommands.cpp
    See RlCommands.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlCommands.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <boost/lexical_cast.hpp>

#include "GoGtpCommandUtil.h"
#include "RlEngine.h"
#include "RlAgentPlayer.h"
#include "RlBinaryFeatures.h"
#include "RlWeightSet.h"
#include "RlEvaluator.h"
#include "RlForceLink.h"
#include "RlLearningRule.h"
#include "RlLocalShapeTracker.h"
#include "RlMoveFilter.h"
#include "RlPolicy.h"
#include "RlSetup.h"
#include "RlSimulator.h"
#include "RlState.h"
#include "RlTracker.h"
#include "RlTrainer.h"
#include "RlFuegoPolicy.h"
#include "RlUtils.h"
#include "SgDebug.h"
#include "SgTimer.h"
#include "SgWrite.h"

using namespace boost;
using namespace std;
using namespace RlMathUtil;

using namespace std;

//----------------------------------------------------------------------------
/** RLGO extension GTP commands */

RlCommands::RlCommands()
:   m_stepSize(false),
    m_sort(false),
    m_numPV(4),
    m_featureIndex(0),
    m_activeIndex(0),
    m_pageIndex(0)
{
}

void RlCommands::Setup(RlSetup* setup)
{
    m_board = &setup->GetBoard();
    m_agent = setup->GetMainAgent();
}

void RlCommands::Register(GtpEngine& e)
{
    Register(e, "rlgo_active_all", &RlCommands::CmdActiveAll);
    Register(e, "rlgo_active_next", &RlCommands::CmdActiveNext);
    Register(e, "rlgo_active_prev", &RlCommands::CmdActivePrev);
    Register(e, "rlgo_active_weights", &RlCommands::CmdActiveWeights);
    Register(e, "rlgo_dirty", &RlCommands::CmdDirty);
    Register(e, "rlgo_eval_params", &RlCommands::CmdEvalParams);
    Register(e, "rlgo_evaluate", &RlCommands::CmdEvaluate);
    Register(e, "rlgo_feature_next", &RlCommands::CmdFeatureNext);
    Register(e, "rlgo_feature_prev", &RlCommands::CmdFeaturePrev);
    Register(e, "rlgo_license", &RlCommands::CmdLicense);
    Register(e, "rlgo_load_weights", &RlCommands::CmdLoadWeights);
    Register(e, "rlgo_save_weights", &RlCommands::CmdSaveWeights);
    Register(e, "rlgo_movefilter", &RlCommands::CmdMoveFilter);
    Register(e, "rlgo_move_freqs", &RlCommands::CmdMoveFreqs);
    Register(e, "rlgo_newgame", &RlCommands::CmdNewGame);
    Register(e, "rlgo_output_params", &RlCommands::CmdOutputParams);
    Register(e, "rlgo_page_next", &RlCommands::CmdPageNext);
    Register(e, "rlgo_page_prev", &RlCommands::CmdPagePrev);
    Register(e, "rlgo_policy", &RlCommands::CmdPolicy);
    Register(e, "rlgo_simulate", &RlCommands::CmdSimulate);
    Register(e, "rlgo_variation", &RlCommands::CmdVariation);
}

void RlCommands::Register(GtpEngine& engine, const std::string& command,
    GtpCallback<RlCommands>::Method method)
{
    engine.Register(command, new GtpCallback<RlCommands>(this, method));
}

void RlCommands::AddGoGuiAnalyzeCommands(GtpCommand& cmd)
{
    cmd <<
        "gfx/RLGO next active/rlgo_active_next\n"
        "gfx/RLGO previous active/rlgo_active_prev\n"
        "gfx/RLGO all active/rlgo_active_all\n"
        "dboard/RLGO active weights/rlgo_active_weights\n"
        "plist/RLGO dirty/rlgo_dirty\n"
        "param/RLGO output params/rlgo_output_params\n"
        "param/RLGO eval params/rlgo_eval_params\n"
        "dboard/RLGO evaluate/rlgo_evaluate\n"
        "gfx/RLGO next feature/rlgo_feature_next\n"
        "gfx/RLGO previous feature/rlgo_feature_prev\n"
        "plist/RLGO move filter/rlgo_movefilter\n"
        "dboard/RLGO move frequences/rlgo_move_freqs\n"
        "none/RLGO load weights/rlgo_load_weights %r\n"
        "gfx/RLGO next page/rlgo_page_next\n"
        "gfx/RLGO prev page/rlgo_page_prev\n"
        "dboard/RLGO policy/rlgo_policy %s\n"
        "dboard/RLGO playout policy/rlgo_playout_policy %s\n"
        "dboard/RLGO predictions/rlgo_predictions\n"
        "dboard/RLGO prediction mass/rlgo_prediction_mass\n"
        "none/RLGO load weights/rlgo_save_weights %w\n"
        "param/RLGO select agent/rlgo_select_agent\n"
        "param/RLGO select tracker/rlgo_select_tracker\n"
        "none/RLGO select value/rlgo_select_value\n"
        "dboard/RLGO shape mass/rlgo_shape_mass\n"
        "none/RLGO simulate/rlgo_simulate\n"
        "var/RLGO variation/rlgo_variation";
}

void RlCommands::CmdDirty(GtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    SgBlackWhite colour = GoGtpCommandUtil::BlackWhiteArg(cmd, 0);

    for (GoBoard::Iterator i_board(Board()); i_board; ++i_board)
    {
        SgPoint move = *i_board;
        if (Agent()->GetEvaluator()->IsDirty(move, colour))
            cmd << SgWritePoint(move) << " ";
    }
}

void RlCommands::CmdEvalParams(GtpCommand& cmd)
{
    string key, value;
    if (cmd.NuArg() > 0)
    {
        cmd.CheckNuArg(2);
        key = cmd.Arg(0);
        value = cmd.Arg(1);

        if (key == "Mode")
            for (int i = 0; i < Scale::RL_NUM_MODES; ++i)
                if (value == m_scale.ModeString(i))
                    m_scale.m_mode = i;
    }

    cmd << "[list";
    for (int i = 0; i < Scale::RL_NUM_MODES; ++i)
        cmd << "/" << m_scale.ModeString(i);
    cmd << "] Mode " << m_scale.ModeString(m_scale.m_mode) << "\n";
}

void RlCommands::CmdOutputParams(GtpCommand& cmd)
{
    string key, value;
    if (cmd.NuArg() > 0)
    {
        cmd.CheckNuArg(2);
        key = cmd.Arg(0);
        value = cmd.Arg(1);
        
        if (key == "StepSize")
            m_stepSize = lexical_cast<int>(value);
        if (key == "Sort")
            m_sort = lexical_cast<int>(value);
        if (key == "NumPV")
            m_numPV = lexical_cast<int>(value);
    }

    cmd << "[bool] StepSize " << m_stepSize << "\n";
    cmd << "[bool] Sort " << m_sort << "\n";
    cmd << "NumPV " << m_numPV << "\n";
}

void RlCommands::CmdEvaluate(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    
    Influence influence;
    RlEvaluator* evaluator = Agent()->GetEvaluator();

    for (RlBoardIterator i_board(Board()); i_board; ++i_board)
    {
        SgMove move = *i_board;
        if (Board().IsLegal(move))
        {
            RlFloat eval = evaluator->EvaluateMove(move, Board().ToPlay());
            influence.Set(move, eval);
        }
    }

    m_scale.ScaleInfluence(influence);
    influence.DBoard(this, cmd);
}

void RlCommands::CmdFeatureNext(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    cmd << "TEXT " << Agent()->GetFeatureSet()->SetName() << ": "
        << m_featureIndex + 1 << " of " 
        << Agent()->GetFeatureSet()->GetNumFeatures();
    DescribeFeature(cmd, m_featureIndex);
    if (m_featureIndex < Agent()->GetFeatureSet()->GetNumFeatures() - 1)
        m_featureIndex++;
}

void RlCommands::CmdFeaturePrev(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    if (m_featureIndex > 0)
        m_featureIndex--;
    cmd << "TEXT " << Agent()->GetFeatureSet()->SetName() << ": "
        << m_featureIndex + 1 << " of " 
        << Agent()->GetFeatureSet()->GetNumFeatures();
    DescribeFeature(cmd, m_featureIndex);
}

void RlCommands::DescribeFeature(GtpCommand& cmd, int featureindex)
{
    Agent()->GetFeatureSet()->DisplayFeature(featureindex, cmd);

    // Use status bar for value (not associated with any point)
    RlWeightSet* wset = m_agent->GetWeightSet();
    RlWeight& weight = wset->Get(featureindex);
    if (m_stepSize)
        cmd << " Step = " << weight.Step() <<"\n";
    else
        cmd << " Value = " << weight.Weight() << "\n";
}

void RlCommands::DescribeActive(
    GtpCommand& cmd, RlChange& entry, int size)
{
    RlBinaryFeatures* fset = Agent()->GetFeatureSet();

    cmd << "TEXT " << entry.m_occurrences << "x " << fset->SetName()
        << ": " << m_activeIndex + 1 << " of " << size
        << " [" << entry.m_featureIndex << "]\n";
    DescribeFeature(cmd, entry.m_featureIndex);
}

void RlCommands::AllActive(vector<RlChange>& entries)
{
    entries.clear();
    RlState& state = Agent()->GetState();
    state.SetActive(Agent()->GetEvaluator()->Active());
    for (RlActiveSet::Iterator i_active(state.Active());
        i_active; ++i_active)
    {
        RlChange entry;
        entry.m_featureIndex = i_active->m_featureIndex;
        entry.m_occurrences = i_active->m_occurrences;
        entry.m_slot = i_active.Slot();
        entries.push_back(entry);
    }
}

void RlCommands::CmdActiveAll(GtpCommand& cmd)
{
    cmd.CheckArgNone();

    vector<RlChange> entries;
    AllActive(entries);
    int size = entries.size();

    for (int i = 0; i < size; ++i)
    {
        RlBinaryFeatures* fset = Agent()->GetFeatureSet();
        if (fset == Agent()->GetFeatureSet())
            fset->DisplayFeature(entries[i].m_featureIndex, cmd);
    }
}

bool RlCommands::EntryCmp::operator()(
    const RlChange& lhs, const RlChange& rhs) const
{
    const RlWeight& weight1 = m_weightSet->Get(lhs.m_featureIndex);
    const RlWeight& weight2 = m_weightSet->Get(rhs.m_featureIndex);
    return fabs(weight1.Weight()) > fabs(weight2.Weight());
}

void RlCommands::CmdActiveNext(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    vector<RlChange> entries;
    AllActive(entries);
    if (m_sort)
        sort(entries.begin(), entries.end(), 
            EntryCmp(m_agent->GetWeightSet()));
    int size = entries.size();
    if (size == 0)
        return;
    if (m_activeIndex >= size)
        m_activeIndex = size - 1;
    DescribeActive(cmd, entries[m_activeIndex], size);
    if (m_activeIndex < size - 1)
        m_activeIndex++;
}

void RlCommands::CmdActivePrev(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    vector<RlChange> entries;
    AllActive(entries);
    if (m_sort)
        sort(entries.begin(), entries.end(), EntryCmp(m_agent->GetWeightSet()));
    int size = entries.size();
    if (size == 0)
        return;
    if (m_activeIndex >= size)
        m_activeIndex = size - 1;
    if (m_activeIndex > 0)
        m_activeIndex--;
    DescribeActive(cmd, entries[m_activeIndex], size);
}

void RlCommands::CmdActiveWeights(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    if (!Agent()->GetFeatureSet())
        return;

    // Display active weight associated with each position
    RlBinaryFeatures* fset = Agent()->GetFeatureSet();
    RlWeightSet* wset = Agent()->GetWeightSet();

    vector<RlChange> entries;
    AllActive(entries);
    Influence influence;
    for (int i = 0; i < ssize(entries); ++i)
    {
        RlChange& entry = entries[i];
        const RlWeight& weight = wset->Get(entry.m_featureIndex);
        SgPoint pos = fset->GetPosition(entry.m_featureIndex);
        if (m_stepSize)
            influence.Add(pos, weight.Step());
        else
            influence.Add(pos, weight.Weight());
    }

    m_scale.ScaleInfluence(influence);
    influence.DBoard(this, cmd);
}

void RlCommands::CmdLicense(GtpCommand& cmd)
{
    cmd << "\n" <<
        "RLGO " << PACKAGE_VERSION 
            << ": Reinforcement Learning in Computer Go\n" <<
        "Copyright (C) 2009 David Silver.\n"
        "\n"
        "This program is free software: you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation, either version 3 of the License, or "
        "(at your option) any later version."
        "\n"
        "This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        "GNU General Public License for more details."
        "\n"
        "You should have received a copy of the GNU General Public License "
        "along with this program.  If not, see <http://www.gnu.org/licenses/>"
        "\n";
}

void RlCommands::CmdLoadWeights(GtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    bfs::path filename(cmd.Arg(0));

    try
    {
        Agent()->Load(filename);
    }
    catch (ios_base::failure& e)
    {
        throw GtpFailure() << "Couldn't load weights from " 
            << filename.string();
    }
}

void RlCommands::CmdMoveFilter(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    const RlMoveFilter* filter = Agent()->GetEvaluator()->GetMoveFilter();
    for (RlMoveFilter::Iterator i_filter(*filter, Board().ToPlay());
        i_filter; ++i_filter)
    {
        SgMove move = *i_filter;
        cmd << SgWritePoint(move) << " ";
    }
}

void RlCommands::CmdMoveFreqs(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    if (!Agent()->GetSimulator())
        throw GtpFailure() << "No simulator";
        
    vector<RlFloat> dboard;
    Agent()->GetSimulator()->GetAllFreqs(dboard);
    WriteFloatBoard(cmd, dboard);
}

void RlCommands::CmdNewGame(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    Agent()->NewGame();
}

void RlCommands::CmdPageNext(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    DescribePage(cmd);
    if (m_pageIndex < Agent()->GetFeatureSet()->GetNumPages() - 1)
        m_pageIndex++;
}

void RlCommands::CmdPagePrev(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    if (m_pageIndex > 0)
        m_pageIndex--;
    DescribePage(cmd);
}

void RlCommands::DescribePage(GtpCommand& cmd)
{
    vector<int> indices;
    ostringstream pagename;
    Agent()->GetFeatureSet()->GetPage(m_pageIndex, indices, pagename);
    RlWeightSet* wset = Agent()->GetWeightSet();
    
    Influence influence;
    int index = 0;
    for (GoBoard::Iterator i_board(Board()); i_board; ++i_board)
    {
        SgPoint pt = *i_board;
        if (indices[index] >= 0)
        {
            const RlWeight& weight = wset->Get(indices[index]);
            RlFloat val = m_stepSize ? weight.Step() : weight.Weight();
            influence.Set(pt, val);
        }
        index++;
    }

    m_scale.ScaleInfluence(influence);
    influence.Gfx(this, cmd);
    
    cmd << "\nTEXT " << pagename.str();
}

void RlCommands::CmdPolicy(GtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    int numsamples = lexical_cast<int>(cmd.Arg(0));
    
    vector<RlFloat> probs(SG_PASS + 1, 0);
    Agent()->GetPolicy()->SampleProbabilities(
        Agent()->GetTimeStep(), numsamples, probs);

    vector<RlFloat> fboard;
    for (GoBoard::Iterator i_board(Board()); i_board; ++i_board)
        fboard.push_back(probs[*i_board]);
    WriteFloatBoard(cmd, fboard);
}

void RlCommands::CmdSaveWeights(GtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    bfs::path filename(cmd.Arg(0));

    try
    {
        Agent()->Save(filename);
    }
    catch (ios_base::failure& e)
    {
        throw GtpFailure() << "Couldn't save weights to " 
            << filename.string();
    }
}

void RlCommands::CmdSimulate(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    Agent()->Think();
}

void RlCommands::CmdVariation(GtpCommand& cmd)
{
    cmd.CheckArgNone();

    RlEvaluator* evaluator = Agent()->GetEvaluator();
    RlGreedy greedy(Board(), evaluator, Agent()->GetHistory());
    
    for (int i = 0; i < m_numPV; ++i)
    {
        SgBlackWhite toplay = Board().ToPlay();
        // use temporary state to make variation
        RlState state(Agent()->GetTimeStep() + i, toplay);
        SgMove move = greedy.SelectMove(state);
        evaluator->PlayExecute(move, toplay, false);
        cmd << SgWritePoint(move) << " ";
    }
    for (int i = 0; i < m_numPV; ++i)
        evaluator->TakeBackUndo(false);
}

void RlCommands::WriteFloatBoard(GtpCommand& cmd, 
    const std::vector<RlFloat>& vec)
{
    SgGrid size = Board().Size();
    if (ssize(vec) < size * size || ssize(vec) > size * size + 1)
        throw GtpFailure() << "WriteFloatBoard passed vector of wrong size";

    // Rows need to be output in reverse order
    for (SgGrid row = size - 1; row >= 0; --row)
    {
        for (SgGrid col = 0; col < size; ++col)
        {
            cmd.ResponseStream().precision(3);
            cmd << vec[row * size + col] << " ";
        }
        cmd << "\n";
    }    
    
    if (ssize(vec) == size * size + 1)
        cerr << "Off-board value: " << vec[size * size] << "\n";
}

void RlCommands::WriteStringBoard(GtpCommand& cmd, const vector<string>& vec)
{
    SgGrid size = Board().Size();
    if (ssize(vec) != size * size)
        throw GtpFailure() << "WriteStringBoard passed vector of wrong size";

    // Rows need to be output in reverse order
    for (SgGrid row = size - 1; row >= 0; --row)
    {
        for (SgGrid col = 0; col < size; ++col)
        {
            string s = vec[row * size + col];
            cmd << (s.empty() ? "\"\"" : s) << " ";
        }
        cmd << "\n";
    }    
}

RlCommands::Influence::Influence()
{
    for (int i = 0; i < SG_MAXPOINT; ++i)        
    {
        m_values[i] = 0;
        m_set[i] = false;
    }
}

void RlCommands::Influence::Set(SgPoint pt, RlFloat value)
{
    m_values[pt] = value;
    m_set[pt] = true;
}

void RlCommands::Influence::Add(SgPoint pt, RlFloat value)
{
    m_values[pt] += value;
    m_set[pt] = true;
}

void RlCommands::Influence::DBoard(RlCommands* commands, GtpCommand& cmd)
{
    vector<RlFloat> dboard;
    for (GoBoard::Iterator i_board(
        commands->Board()); i_board; ++i_board)
    {
        SgPoint pt = *i_board;
        if (m_set[pt])
            dboard.push_back(m_values[pt]);
        else
            dboard.push_back(0);
    }

    // Off-board value stored at point 0.
    if (m_set[0])
        dboard.push_back(m_values[0]);
    else
        dboard.push_back(0);
        
    commands->WriteFloatBoard(cmd, dboard);
}

void RlCommands::Influence::Gfx(RlCommands* commands, GtpCommand& cmd)
{
    cmd << "INFLUENCE";
    vector<RlFloat> dboard;
    for (GoBoard::Iterator i_board(
        commands->Board()); i_board; ++i_board)
    {
        SgPoint pt = *i_board;
        if (m_set[pt])
            cmd << " " << SgWritePoint(pt) << " " << m_values[pt];
    }
}

string RlCommands::Scale::ModeString(int mode) const
{
    switch (mode)
    {
        case RL_NONE:
            return "None";
        case RL_CENTRED:
            return "Centred";
        case RL_LOGISTIC:
            return "Logistic";
        case RL_LINEAR:
            return "Linear";
        case RL_STDDEV:
            return "StdDev";
        case RL_MAXDEV:
            return "MaxDev";
    }
    return "Unknown";
}

RlFloat RlCommands::Scale::ScaleFloat(RlFloat eval) const
{
    //@todo: replace with ScaleInfluence
    RlFloat uniteval;
    switch (m_mode)
    {
        case RL_LOGISTIC:
            uniteval = Logistic(eval);
            break;
        default:
            uniteval = (eval + 1) / 2;
            break;
    }
    return uniteval * 2 - 1;
}

void RlCommands::Scale::ScaleInfluence(
    RlCommands::Influence& influence) const
{
    RlFloat sum = 0, sumsq = 0, mean = 0, stddev = 0, maxdev = 0;
    int count = 0;
    for (int i = 0; i < SG_MAXPOINT; ++i)
    {
        if (influence.IsSet(i))
        {
            sum += influence.Value(i);
            sumsq += influence.Value(i) * influence.Value(i);
            count++;
        }
    }
    if (count > 0)
    {
        mean = sum / count;
        stddev = sqrt(sumsq / count - mean * mean);
    }
    RlFloat min = +RlInfinity, max = -RlInfinity;
    for (int i = 0; i < SG_MAXPOINT; ++i)
    {
        RlFloat absdev = fabs(influence.Value(i) - mean);
        if (influence.IsSet(i))
        { 
            if (absdev > maxdev)
                maxdev = absdev;
            if (influence.Value(i) > max)
                max = influence.Value(i);
            if (influence.Value(i) < min)
                min = influence.Value(i);
        }
    }

    cerr << "Mean = " << mean << " StdDev = " << stddev 
        << " MaxDev = " << maxdev << " Min = " << min
        << " Max = " << max << "\n";

    for (int i = 0; i < SG_MAXPOINT; ++i)
    {
        if (!influence.IsSet(i))
            continue;
        switch (m_mode)
        {
            case RL_NONE:
                break;
            case RL_CENTRED:
                influence.Set(i, influence.Value(i) - mean);
                break;
            case RL_LOGISTIC:
                influence.Set(i, Logistic(influence.Value(i)));
                influence.Set(i, influence.Value(i) * 2 - 1);
                break;
            case RL_LINEAR:
                if (max - min != 0)
                {
                    influence.Set(i, 
                        (influence.Value(i) - min) / (max - min));
                    influence.Set(i, influence.Value(i) * 2 - 1);
                }
                break;
            case RL_STDDEV:
                if (stddev != 0)
                    influence.Set(i, (influence.Value(i) - mean) / stddev);
                break;
            case RL_MAXDEV:
                if (maxdev != 0)
                    influence.Set(i, (influence.Value(i) - mean) / maxdev);
                break;
        }        
    }
}

//----------------------------------------------------------------------------
