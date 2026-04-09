#!/bin/bash -e

BENCHMARKS="bubble-sort distinctness edit-distance eulers-number-approx fft-int filtered-query flood-fill flood-fill-On2 gcd-list gradient-descent grad-x image-filters kadane kepler-calc knapsack lda linear-reg mersenne minspan mnist-cnn nonlinear-nn nr-solver pagerank parrando primal-test rad-to-degree recommender set-intersect shortest-path tea-cipher triangle-count"
MODES="na do enc"

curDIR=$PWD

for dir in $BENCHMARKS; do
  for mode in $MODES; do

    cd $dir
    echo "--------------------------------"
    echo "Running "$dir" in MODE="$mode
    echo "--------------------------------"

    make MODE=$mode clean build test

    cd ..

  done
done
exit

