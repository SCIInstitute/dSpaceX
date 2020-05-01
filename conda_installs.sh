#
# Installs conda environment for building dSpaceX
#

(return 0 2>/dev/null) && sourced=1 || sourced=0

if [[ "$sourced" == "0" ]]; then
  echo "ERROR: must call this script using \"source ./conda_installs.sh\""
  exit 1
fi

function install_conda() {
  if ! command -v conda 2>/dev/null 1>&2; then
    echo "installing anaconda..."
    if [[ "$(uname)" == "Darwin" ]]; then
      curl -o ./Miniconda3-latest-MacOSX-x86_64.sh https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh
      bash ./Miniconda3-latest-MacOSX-x86_64.sh
      rm ./Miniconda3-latest-MacOSX-x86_64.sh
    elif [[ "$(uname)" == "Linux" ]]; then
      curl -o ./Miniconda3-latest-Linux-x86_64.sh https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
      bash ./Miniconda3-latest-Linux-x86_64.sh
      rm ./Miniconda3-latest-Linux-x86_64.sh
    else
      echo "ERROR: unknown OS $(uname)"
      return 1
    fi

    source ~/miniconda3/bin/activate
    conda config --set auto_activate_base false
  fi

  # add default channels
  conda config --add channels anaconda
  conda config --add channels conda-forge
  
  # update anaconda
  if ! conda update --yes -n base conda; then return 1; fi
  if ! conda update --yes --all; then return 1; fi

  # create and activate dspacex env
  CONDAENV=dspacex-new
  if ! conda create --yes --name $CONDAENV python=3.7; then return 1; fi
  eval "$(conda shell.bash hook)"
  if ! conda activate $CONDAENV; then return 1; fi

  # pip is needed in sub-environments or the base env's pip will silently install to base
  if ! conda install --yes pip=20.0.2; then return 1; fi
  if ! pip install --upgrade pip; then return 1; fi

  # install dspacex deps
  if ! conda install --yes \
       zlib=1.2.11 \
       ncurses=6.1 \
       cmake=3.15.5 \
       nodejs=13.9.0 \
       eigen=3.3.7 \
       libpng=1.6.37 \
       gtest=1.10.0 \
       yaml-cpp=0.6.3
  then return 1; fi

  # linux-only deps
  if [[ "$(uname)" = "Linux" ]]; then
    conda install --yes \
          blas=2.14 \
          liblapack=3.8.0
  fi
  
  # pip installs (none currently needed, but here are two examples:)
  #if ! pip install matplotlib==3.1.2; then return 1; fi
  #if ! pip install -e Python/DatasetUtilsPackage; then return 1; fi   # install the local GirderConnector code as a package

  conda info
  return 0
}

if install_conda; then
  echo "$CONDAENV environment successfully created/updated!"
  conda activate $CONDAENV
else
  echo "Problem encountered creating/updating $CONDAENV conda environment."
  return 1;
fi
