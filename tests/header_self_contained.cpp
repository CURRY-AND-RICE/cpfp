#include "cpfp/arithmetic.hpp"
#include "cpfp/combinator.hpp"
#include "cpfp/comparison.hpp"
#include "cpfp/fold.hpp"
#include "cpfp/logical.hpp"
#include "cpfp/experimental.hpp"

int main() { return cpfp::add(1, 2) == 3 ? 0 : 1; }
