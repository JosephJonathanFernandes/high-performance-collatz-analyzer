#pragma once

namespace collatz {

/**
 * @brief Represents the result of a Collatz sequence computation.
 */
struct CollatzResult {
    long long starting_number;
    int steps;
    long long peak_value;
};

} // namespace collatz
