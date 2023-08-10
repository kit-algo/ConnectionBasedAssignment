#pragma once

#include "../../Shell/Shell.h"

#include "../../Algorithms/DecisionModels/Kirchhoff.h"
#include "../../Algorithms/DecisionModels/Linear.h"
#include "../../Algorithms/DecisionModels/Logit.h"
#include "../../Algorithms/DecisionModels/Optimal.h"
#include "../../Algorithms/DecisionModels/RelativeLogit.h"
#include "../../Algorithms/Assignment/GroupAssignment.h"
#include "../../Algorithms/Assignment/Profiler.h"
#include "../../DataStructures/Assignment/Settings.h"

using namespace Shell;

class ParseCSAFromCSV : public ParameterizedCommand {

public:
    ParseCSAFromCSV(BasicShell& shell) :
        ParameterizedCommand(shell, "parseCSAFromCSV", "Parses raw .csv files containing a CSA network and converts it to a binary representation.") {
        addParameter("Input directory");
        addParameter("Output file");
        addParameter("Parse zones?", "false");
        addParameter("Make bidirectional?", "true");
        addParameter("Repair files?", "false");
    }

    virtual void execute() noexcept {
        const std::string csvDirectory = getParameter("Input directory");
        const bool parseZones = getParameter<bool>("Parse zones?");
        const bool makeBidirectional = getParameter<bool>("Make bidirectional?");
        const bool repairFiles = getParameter<bool>("Repair files?");

        if (repairFiles) {
            CSA::Data::RepairFiles(csvDirectory);
        }
        if (parseZones) {
            if (makeBidirectional) {
                parseData(CSA::Data::FromCSVwithZones<false>(csvDirectory));
            } else {
                parseData(CSA::Data::FromCSVwithZones<true>(csvDirectory));
            }
        } else {
            parseData(CSA::Data::FromCSV(csvDirectory));
        }
    }

private:
    inline void parseData(const CSA::Data& data) const noexcept {
        const std::string outputFile = getParameter("Output file");
        data.printInfo();
        data.serialize(outputFile);
    }

};

class GroupAssignment : public ParameterizedCommand {

public:
    GroupAssignment(BasicShell& shell) :
        ParameterizedCommand(shell, "groupAssignment", "Computes a public transit traffic assignment for zone based demand.",  "Num threads:", "    positive number  - parallel execution with <Num threads> threads", "    otherwise        - sequential execution") {
        addParameter("Settings file");
        addParameter("CSA binary");
        addParameter("Demand file");
        addParameter("Output file");
        addParameter("Demand multiplier", "1");
        addParameter("Num threads", "0");
        addParameter("Thread offset", "1");
        addParameter("Aggregate file", "-");
        addParameter("Aggregate prefix", "-");
        addParameter("Use transfer buffer times", "false");
        addParameter("Demand output file", "-");
        addParameter("Demand output size", "-1");
    }

    virtual void execute() noexcept {
        const bool useTransferBufferTimes = getParameter<bool>("Use transfer buffer times");
        if (useTransferBufferTimes) {
            chooseProfiler<true>();
        } else {
            chooseProfiler<false>();
        }
    }

private:
    template<bool USE_TRANSFER_BUFFER_TIMES>
    inline void chooseProfiler() {
        const std::string settingsFileName = getParameter("Settings file");

        ConfigFile configFile(settingsFileName, true);
        Assignment::Settings settings(configFile);
        configFile.writeIfModified(false);
        switch (settings.profilerType) {
            case 0: {
                chooseDecisionModel<Assignment::NoProfiler, USE_TRANSFER_BUFFER_TIMES>(settings);
                break;
            }
            case 1: {
                chooseDecisionModel<Assignment::TimeProfiler, USE_TRANSFER_BUFFER_TIMES>(settings);
                break;
            }
            case 2: {
                chooseDecisionModel<Assignment::DecisionProfiler, USE_TRANSFER_BUFFER_TIMES>(settings);
                break;
            }
        }
    }

    template<typename PROFILER, bool USE_TRANSFER_BUFFER_TIMES>
    inline void chooseDecisionModel(const Assignment::Settings& settings) {
        switch (settings.decisionModel) {
            case 0: {
                computeApportionment<Assignment::GroupAssignment<DecisionModels::Linear, PROFILER, USE_TRANSFER_BUFFER_TIMES>>(settings);
                break;
            }
            case 1: {
                computeApportionment<Assignment::GroupAssignment<DecisionModels::Logit, PROFILER, USE_TRANSFER_BUFFER_TIMES>>(settings);
                break;
            }
            case 2: {
                computeApportionment<Assignment::GroupAssignment<DecisionModels::Kirchhoff, PROFILER, USE_TRANSFER_BUFFER_TIMES>>(settings);
                break;
            }
            case 3: {
                computeApportionment<Assignment::GroupAssignment<DecisionModels::RelativeLogit, PROFILER, USE_TRANSFER_BUFFER_TIMES>>(settings);
                break;
            }
            case 4: {
                computeApportionment<Assignment::GroupAssignment<DecisionModels::Optimal, PROFILER, USE_TRANSFER_BUFFER_TIMES>>(settings);
                break;
            }
        }
    }

    template<typename APPORTIONMENT_TYPE>
    inline void computeApportionment(const Assignment::Settings& settings) {
        const std::string csaFileName = getParameter("CSA binary");
        const std::string demandFileName = getParameter("Demand file");
        const std::string outputFileName = getParameter("Output file");
        const size_t demandMultiplier = getParameter<size_t>("Demand multiplier");
        const int numThreads = getParameter<int>("Num threads");
        const int pinMultiplier = getParameter<int>("Thread offset");
        const std::string aggregateFileName = getParameter("Aggregate file");
        const std::string aggregatePrefix = getParameter("Aggregate prefix");
        const std::string demandOutputFileName = getParameter("Demand output file");
        const size_t demandOutputSize = getParameter<size_t>("Demand output size");

        CSA::Data csaData = CSA::Data::FromBinary(csaFileName);
        csaData.sortConnectionsAscendingByDepartureTime();
        csaData.printInfo();
        csaData.transferGraph.printAnalysis();
        std::cout << std::endl;
        CSA::TransferGraph reverseGraph = csaData.transferGraph;
        reverseGraph.revert();
        AccumulatedVertexDemand originalDemand = AccumulatedVertexDemand::FromZoneCSV(demandFileName, csaData, reverseGraph, demandMultiplier);
        AccumulatedVertexDemand demand = originalDemand;

        if (settings.demandIntervalSplitTime >= 0) {
            demand.discretize(settings.demandIntervalSplitTime, settings.keepDemandIntervals, settings.includeIntervalBorder);
        }
        APPORTIONMENT_TYPE ma(csaData, reverseGraph, settings);
        Timer timer;
        if (numThreads > 0) {
            const int numCores(numberOfCores());
            std::cout << "Using " << numThreads << " threads on " << numCores << " cores!" << std::endl;
            ma.run(demand, numThreads, pinMultiplier);
        } else {
            ma.run(demand);
        }

        std::cout << "done in " << String::msToString(timer.elapsedMilliseconds()) << "." << std::endl;
        std::cout << "   removed cycle connections: " << String::prettyInt(ma.getRemovedCycleConnections()) << std::endl;
        std::cout << "   removed cycles: " << String::prettyInt(ma.getRemovedCycles()) << std::endl;
        ma.getProfiler().printStatistics();
        if (demandOutputFileName != "-") {
            AccumulatedVertexDemand outputDemand = originalDemand;
            ma.filterDemand(outputDemand, demandOutputSize);
            outputDemand.toZoneIDs(csaData);
            outputDemand.sanitize();
            outputDemand.toCSV(demandOutputFileName);
        }
        // PassengerData result = ma.getPassengerData(originalDemand);
        // std::cout << result << std::endl;
        // result.serialize(FileSystem::ensureExtension(outputFileName, ".binary"));
        // IdVertexDemand passengers = IdVertexDemand::FromAccumulatedVertexDemand(originalDemand, settings.passengerMultiplier, 100000000);
        // result.writePassengerConnectionPairs(csaData, passengers, FileSystem::ensureExtension(outputFileName, ".csv"));
        ma.printStatistics(originalDemand, outputFileName);
        ma.writeConnectionsWithLoad(FileSystem::ensureExtension(outputFileName, "_connections.csv"));
        ma.writeAssignment(FileSystem::ensureExtension(outputFileName, "_assignment.csv"));
        ma.writeGroups(FileSystem::ensureExtension(outputFileName, "_groups.csv"));
        ma.writeAssignedJourneys(FileSystem::ensureExtension(outputFileName, "_journeys.csv"), demand);
        if (aggregateFileName != "-") {
            ma.writeConnectionStatistics(aggregateFileName, aggregatePrefix);
        }
    }
};
