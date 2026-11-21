#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

// (us): a glob (0) or MIME type (1) rule.
using FilterRule = std::tuple<uint32_t, std::string>;

// (sa(us)): a named filter.
using FileFilter = std::tuple<std::string, std::vector<FilterRule>>;

// (ss): an option of a choice, or a choice and the option picked.
using ChoiceOption = std::tuple<std::string, std::string>;

// (ssa(ss)s): id, label, options (none for a check box) and the initial selection.
using FileChoice = std::tuple<std::string, std::string, std::vector<ChoiceOption>, std::string>;
