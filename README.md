[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# Connection-Based Transit Assignment

This is a C++ framework for efficient simulation-based transit assignment. It was developed at [KIT](https://www.kit.edu) in the [group of Prof. Dorothea Wagner](https://i11www.iti.kit.edu/). This repository contains code for the following publications:

* *Efficient Traffic Assignment for Public Transit Networks*
  Lars Briem, Sebastian Buck, Holger Ebhart, Nicolai Mallig, Ben Strasser, Peter Vortisch, Dorothea Wagner, Tobias Zündorf
  In: Proceedings of the 16th International Symposium on Experimental Algorithms (SEA'17), Leibniz International Proceedings in Informatics, pages 20:1–20:14, 2017
  [pdf](https://drops.dagstuhl.de/opus/volltexte/2017/7610/pdf/LIPIcs-SEA-2017-20.pdf)

* *Efficient Computation of Multi-Modal Public Transit Traffic Assignments using ULTRA*
  Jonas Sauer, Dorothea Wagner, Tobias Zündorf
  In: Proceedings of the 27th ACM SIGSPATIAL International Conference on Advances in Geographic Information Systems (SIGSPATIAL'19), ACM Press, pages 524–527, 2019
  [pdf](https://i11www.iti.kit.edu/_media/members/tobias_zuendorf/sigspatial.pdf) [arXiv](https://arxiv.org/abs/1909.08519)

## Data format
The assignment algorithm takes two inputs: a public transit network and a demand file. Both are supplied in CSV format. An example input is provided in the ``Data`` folder.

The following CSV files are required. All times are given in seconds and all coordinates as decimal degrees.
| File                   | Values                                                                          | Description |
| ---------------------- | ------------------------------------------------------------------------------- | ----------- |
| ``stops.csv``          | ``stop_id``,``change_time``,``name``,``lon``,``lat``                            | Stops of the network. ``change_time`` is the time required to transfer between two trips at this stop. |
| ``transfers.csv``      | ``dep_stop``,``arr_stop``,``duration``                                          | Footpaths between neighboring stops. |
| ``zones.csv``          | ``zone_id``,``lon``,``lat``                                                     | Zones for passenger origins/destinations. |
| ``zone_transfers.csv`` | ``zone_id``,``stop_id``,``duration``                                            | Footpaths between zones and stops. |
| ``trips.csv``          | ``trip_id``,``vehicle,name``,``line_id``                                        | Public transit trips. The schedule of a trip is given as a sequence of connections between consecutive stops. |
| ``connections.csv``    | ``dep_stop``,``arr_stop``,``dep_time``,``arr_time``,``trip_id``                 | Connections for the trips. |
| ``demand.csv``         | ``dep_zone``,``arr_zone``,``min_dep_time``,``max_dep_time``,``passenger_count`` | Zone-based demand. |

## Usage
The algorithms are provided in the console application ``Assignment``. You can compile it with the ``Makefile`` in the ``Runnables`` folder. Type ``make AssignmentRelease -B`` to compile in release mode. The following commands are available:

* ``parseCSAFromCSV``: Converts the public transit network into the binary format used by the algorithm.
* ``groupAssignment``: Computes an assignment. Parameters:
    - Settings file: Path to the settings file for the assignment algorithm. If no file exists at this location, one will be created with default settings. See ``DataStructures/Assignment/Settings.h`` for explanations of the individual settings.
    - CSA binary: Binary network data created with ``parseCSAFromCSV``.
    - Demand file: Demand file in CSV format.
    - Output file: Path for the output files.
    - Demand multiplier: The passenger count for each demand group is multiplied with this value (default: 1).
    - Num threads: Number of threads for parallel execution (default: 0, sequential).
    - Thread offset: Offset for the processor IDs to be used. For example, a value of 2 means that cores 0, 2, 4, ... will be used (default: 1).
    - Use transfer buffer times?: If set to true, the minimum transfer times supplied in ``stops.csv`` are considered even when transferring to a stop with a footpath (default: false).
    - Demand output file: Output file for the filtered demand data, excluding unassigned passengers (default: -, no file is written).
    - Demand output size: Maximum number of entries in the filtered demand (default: -1, no limit)
