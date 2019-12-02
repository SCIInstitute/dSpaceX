#
# Installs conda environment for building dSpaceX
#

if [ -z "$PS1" ]; then
  echo "ERROR: must call this script using \"source ./conda_installs.sh\")"
  exit 1
fi

if ! command -v conda 2>/dev/null 1>&2; then
  echo "installing anaconda..."
  if [ "$(uname)" == "Darwin" ]; then
    curl https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh -o Miniconda3-latest-MacOSX-x86_64.sh
    bash ./Miniconda3-latest-MacOSX-x86_64.sh
    rm ./Miniconda3-latest-MacOSX-x86_64.sh
  elif [ "$(uname)" == "Linux" ]; then
    curl https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -o Miniconda3-latest-Linux-x86_64.sh
    bash ./Miniconda3-latest-Linux-x86_64.sh
    rm ./Miniconda3-latest-Linux-x86_64.sh
  else
    echo "ERROR: unknown OS $(uname)"
    return 1
  fi

  source ~/miniconda3/bin/activate
  conda config --set auto_activate_base false
fi

#update anaconda
conda update --yes -n base -c defaults conda
conda update --yes --all

#create and activate dspacex env
CONDAENV=dspacex-test00
conda create --yes --name $CONDAENV
eval "$(conda shell.bash hook)"
conda activate $CONDAENV

#install dspacex deps
conda install --yes -c conda-forge zlib cmake nodejs eigen
conda install --yes -c anaconda libpng

if [ "$(uname)" == "Linux" ]; then
  conda install --yes -c conda-forge blas liblapack
fi

conda info
