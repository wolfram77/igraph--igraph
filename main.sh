#!/usr/bin/env bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=64
#SBATCH --exclusive
#SBATCH --job-name slurm
#SBATCH --output=slurm.out
# source scl_source enable gcc-toolset-11
# module load hpcx-2.7.0/hpcx-ompi
# module load openmpi/4.1.5
src="igraph--igraph"
out="$HOME/Logs/$src$1.log"
ulimit -s unlimited
printf "" > "$out"

# Download igraph
if [[ "$DOWNLOAD" != "0" ]]; then
  rm -rf $src
  git clone https://github.com/wolfram77/$src
  cd $src
  git checkout for-rak-communities-openmp
fi

# Build and run igraph
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
cmake --build . -j32
cmake --install .
cd ..
cd examples/simple
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=$HOME/.local ..
cmake --build .
cd ../../..

# Convert graph to binary format, run igraph, and clean up
runIgraph() {
  stdbuf --output=L printf "Converting $1 to $1.edg ...\n"                      | tee -a "$out"
  lines="$(node process.js header-lines "$1")"
  tail -n +$((lines+1)) "$1" > "$1.edg"
  stdbuf --output=L examples/simple/build/igraph_test "$1.edg" "$1.comids" 2>&1 | tee -a "$out"
  stdbuf --output=L printf "\n\n"                                               | tee -a "$out"
  rm -rf "$1.edg"
}

# Run igraph on all graphs
runAll() {
# runIgraph "$HOME/Data/web-Stanford.mtx"
runIgraph "$HOME/Data/indochina-2004.mtx"
runIgraph "$HOME/Data/uk-2002.mtx"
runIgraph "$HOME/Data/arabic-2005.mtx"
runIgraph "$HOME/Data/uk-2005.mtx"
runIgraph "$HOME/Data/webbase-2001.mtx"
runIgraph "$HOME/Data/it-2004.mtx"
runIgraph "$HOME/Data/sk-2005.mtx"
runIgraph "$HOME/Data/com-LiveJournal.mtx"
runIgraph "$HOME/Data/com-Orkut.mtx"
runIgraph "$HOME/Data/asia_osm.mtx"
runIgraph "$HOME/Data/europe_osm.mtx"
runIgraph "$HOME/Data/kmer_A2a.mtx"
runIgraph "$HOME/Data/kmer_V1r.mtx"
}

# Run 5 times
for i in {1..5}; do
  runAll
done

# Signal completion
curl -X POST "https://maker.ifttt.com/trigger/puzzlef/with/key/${IFTTT_KEY}?value1=$src$1"
