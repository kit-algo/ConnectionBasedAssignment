#pragma once

#include <vector>
#include <string>

#include "../../Helpers/ConfigFile.h"

namespace Assignment {

enum CycleMode {
    KeepCycles,
    RemoveStopCycles,
    RemoveStationCycles
};

enum DepartureTimeChoice {
    DecisionModelWithoutAdaption,
    DecisionModelWithAdaption,
    Uniform,
    Rooftop,
    DecisionModelWithBoxCox,
};

class Settings {

public:
    Settings() {}
    Settings(ConfigFile& config) {
        cycleMode = config.get("cycleMode", cycleMode);
        profilerType = config.get("profilerType", profilerType);
        randomSeed = config.get("randomSeed", randomSeed);
        passengerMultiplier = config.get("passengerMultiplier", passengerMultiplier);
        allowDepartureStops = config.get("allowDepartureStops", allowDepartureStops);
        transferCosts = config.get("transferCosts", transferCosts);
        walkingCosts = config.get("walkingCosts", walkingCosts);
        waitingCosts = config.get("waitingCosts", waitingCosts);
        decisionModel = config.get("decisionModel", decisionModel);
        beta = config.get("beta", beta);
        delayTolerance = config.get("delayTolerance", delayTolerance);
        delayValue = config.get("delayValue", delayValue);
        maxDelay = config.get("maxDelay", maxDelay);
        demandIntervalSplitTime = config.get("demandIntervalSplitTime", demandIntervalSplitTime);
        keepDemandIntervals = config.get("keepDemandIntervals", keepDemandIntervals);
        includeIntervalBorder = config.get("includeIntervalBorder", includeIntervalBorder);
        departureTimeChoice = config.get("departureTimeChoice", departureTimeChoice);
        maxAdaptationTime = config.get("maxAdaptationTime", maxAdaptationTime);
        adaptationCost = config.get("adaptationCost", adaptationCost);
        adaptationOffset = config.get("adaptationOffset", adaptationOffset);
        adaptationBeta = config.get("adaptationBeta", adaptationBeta);
        adaptationLambda = config.get("adaptationLambda", adaptationLambda);
    }

    ConfigFile toConfigFile(const std::string& fileName) const noexcept {
        ConfigFile config(fileName);
        config.set("cycleMode", cycleMode);
        config.set("profilerType", profilerType);
        config.set("randomSeed", randomSeed);
        config.set("passengerMultiplier", passengerMultiplier);
        config.set("allowDepartureStops", allowDepartureStops);
        config.set("transferCosts", transferCosts);
        config.set("walkingCosts", walkingCosts);
        config.set("waitingCosts", waitingCosts);
        config.set("decisionModel", decisionModel);
        config.set("beta", beta);
        config.set("delayTolerance", delayTolerance);
        config.set("delayValue", delayValue);
        config.set("maxDelay", maxDelay);
        config.set("demandIntervalSplitTime", demandIntervalSplitTime);
        config.set("keepDemandIntervals", keepDemandIntervals);
        config.set("includeIntervalBorder", includeIntervalBorder);
        config.set("departureTimeChoice", departureTimeChoice);
        config.set("maxAdaptationTime", maxAdaptationTime);
        config.set("adaptationCost", adaptationCost);
        config.set("adaptationOffset", adaptationOffset);
        config.set("adaptationBeta", adaptationBeta);
        config.set("adaptationLambda", adaptationLambda);
        return config;
    }

    int cycleMode{RemoveStationCycles}; // Cycle removal (CycleMode)
    int profilerType{0}; // 0 = NoProfiler, 1 = TimeProfiler, 2 = DecisionProfiler

    int randomSeed{42}; // random seed of the Monte Carlo simulation
    int passengerMultiplier{100}; // multiplier for the demand
    bool allowDepartureStops{true}; // Can demand use stops as origins?

    int transferCosts{5 * 60}; // PAT overhead for changing vehicles
    double walkingCosts{2.0}; // cost factor for the walking time in the PAT (must be >= 0, walking is counted 1 + walkingCosts times)
    double waitingCosts{0.0}; // cost factor for the waiting time in the PAT (must be >= 0, waiting is counted 1 + waitingCosts times)

    int decisionModel{0}; // 0 = Linear, 1 = Logit, 2 = Kirchhoff, 3 = RelativeLogit, 4 = Optimal
    double beta{1.0}; // Adjustment parameter for Logit & Kirchhoff
    int delayTolerance{5 * 60}; // maximum difference a journey PAT can have to the optimal PAT in order to be considered for assignment of passengers
    int delayValue{5 * 60}; // Linear: PAT overhead for non-optimal journeys

    int maxDelay{0}; // max delay of vehicles in the MEAT model

    int demandIntervalSplitTime{86400}; // Time interval size for discretization of the demand input time intervals (negative value indicates no discretization)
    bool keepDemandIntervals{true}; // false = collapse demand departure time intervals to their minimal value, true = keep full intervals
    bool includeIntervalBorder{false}; // true = intervals before discretization are interpreted as (min <= x <= max), false intervals before discretization are interpreted as (min <= x < max)

    int departureTimeChoice{DecisionModelWithoutAdaption}; // Handling of departure times in demand (DepartureTimeChoice)
    int maxAdaptationTime{0}; // Maximum amount that passengers are willing to adjust their departure time
    double adaptationCost{2.0}; // DecisionModelWithAdaptation/Rooftop: Cost factor for adjusting departure time
    int adaptationOffset{0}; // DecisionModelWithAdaptation: Maximum amount of adaptation that is allowed without incurring costs in
    double adaptationBeta{0.1}; // DecisionModelWithBoxCox: Beta value for Box-Cox transformation
    double adaptationLambda{2.0}; // DecisionModelWithBoxCox: Lambda value for Box-Cox transformation
};

}
