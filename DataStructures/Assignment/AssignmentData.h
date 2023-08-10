#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "GroupData.h"
#include "../Demand/AccumulatedVertexDemand.h"
#include "../../Helpers/Types.h"
#include "../../Helpers/IO/File.h"
#include "../../Helpers/Vector/Vector.h"

namespace Assignment {

struct AssignmentData {
    AssignmentData(const size_t numberOfConnections) :
        groupsPerConnection(numberOfConnections) {
    }

    inline GroupId createNewGroup(const AccumulatedVertexDemand::Entry& demandEntry, const int passengerMultiplier) noexcept {
        const GroupId id = groups.size();
        groups.emplace_back(id, demandEntry.demandIndex, demandEntry.numberOfPassengers * passengerMultiplier);
        connectionsPerGroup.emplace_back();
        return id;
    }

    inline GroupId splitGroup(const GroupId parentGroup, const double numberOfPassengers) noexcept {
        AssertMsg(numberOfPassengers > 0, "Cannot create an empty group!");
        AssertMsg(groups[parentGroup].groupSize > numberOfPassengers, "Cannot split off " << numberOfPassengers << " passengers from group of size " << groups[parentGroup].groupSize << ")!");
        const GroupId id = groups.size();
        groups.emplace_back(id, groups[parentGroup].demandIndex, numberOfPassengers);
        connectionsPerGroup.emplace_back(connectionsPerGroup[parentGroup]);
        groups[parentGroup].groupSize -= numberOfPassengers;
        return id;
    }

    inline void addGroupsToConnections() noexcept {
        for (GroupId group = 0; group < connectionsPerGroup.size(); group++) {
            for (const ConnectionId connection : connectionsPerGroup[group]) {
                groupsPerConnection[connection].emplace_back(group);
            }
        }
    }

    inline void clear() noexcept {
        std::vector<GroupData>().swap(groups);
        std::vector<std::vector<ConnectionId>>().swap(connectionsPerGroup);
        std::vector<GroupList>(groupsPerConnection.size()).swap(groupsPerConnection);
        GroupList().swap(unassignedGroups);
        GroupList().swap(directWalkingGroups);
    }

    inline long long byteSize() const noexcept {
        long long result = Vector::byteSize(groups);
        result += Vector::byteSize(connectionsPerGroup);
        result += Vector::byteSize(groupsPerConnection);
        result += Vector::byteSize(unassignedGroups);
        result += Vector::byteSize(directWalkingGroups);
        return result;
    }

    inline double getConnectionLoad(const ConnectionId connectionId) const noexcept {
        return Vector::sum(groupsPerConnection[connectionId], [&](const GroupId group) {
            return groups[group].groupSize;
        });
    }

    inline void writeAssignment(const std::string& fileName) const noexcept {
        IO::OFStream file(fileName);
        file << "connectionId,groupId\n";
        for (size_t i = 0; i < groupsPerConnection.size(); i++) {
            for (const GroupId group : groupsPerConnection[i]) {
                file << i << "," << group << "\n";
            }
        }
    }

    inline void writeGroups(const std::string& fileName) const noexcept {
        IO::OFStream file(fileName);
        file << "groupId,demandId,groupSize\n";
        for (const GroupData& group : groups) {
            file << group.groupId << "," << group.demandIndex << "," << group.groupSize << "\n";
        }
    }

    inline void filterDemand(AccumulatedVertexDemand& demand, const size_t maxSize = -1) const noexcept {
        Set<size_t> unassignableDemandIndices;
        for (const GroupId group: unassignedGroups) {
            unassignableDemandIndices.insert(groups[group].demandIndex);
        }
        for (size_t i = 0; i < demand.entries.size(); i++) {
            if (!unassignableDemandIndices.contains(demand.entries[i].demandIndex)) continue;
            demand.entries[i] = demand.entries.back();
            demand.entries.pop_back();
            i--;
        }
        if (demand.entries.size() > maxSize) {
            demand.entries.resize(maxSize);
        }
    }

    inline AssignmentData& operator+=(const AssignmentData& other) noexcept {
        const size_t groupOffset = groups.size();
        for (const GroupData& group : other.groups) {
            AssertMsg(group.groupId + groupOffset == groups.size(), "Current group id is " << (group.groupId + groupOffset) << ", but should be " << groups.size() << "!");
            groups.emplace_back(groups.size(), group.demandIndex, group.groupSize);
            connectionsPerGroup.emplace_back(other.connectionsPerGroup[group.groupId]);
        }
        for (size_t i = 0; i < other.groupsPerConnection.size(); i++) {
            for (const GroupId group : other.groupsPerConnection[i]) {
                groupsPerConnection[i].emplace_back(group + groupOffset);
            }
        }
        for (const GroupId group : other.unassignedGroups) {
            unassignedGroups.emplace_back(group + groupOffset);
        }
        for (const GroupId group : other.directWalkingGroups) {
            directWalkingGroups.emplace_back(group + groupOffset);
        }
        return *this;
    }

    std::vector<GroupData> groups;
    std::vector<std::vector<ConnectionId>> connectionsPerGroup;
    std::vector<GroupList> groupsPerConnection;
    GroupList unassignedGroups;
    GroupList directWalkingGroups;
};
}
