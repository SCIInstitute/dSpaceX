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
    wget https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh
    bash ./Miniconda3-latest-MacOSX-x86_64.sh
    rm ./Miniconda3-latest-MacOSX-x86_64.sh
  elif [ "$(uname)" == "Linux" ]; then
    wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
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
conda create --yes --name dspacex
eval "$(conda shell.bash hook)"
conda activate dspacex

#install dspacex deps
conda install --yes -c conda-forge zlib cmake igl nodejs
conda install --yes -c anaconda libpng

# maybe also...
# bzip2                     1.0.8                h01d97ff_0    conda-forge
# ca-certificates           2019.10.16                    0  
# ctags                     5.8               h1de35cc_1000    conda-forge
# curl                      7.65.3               ha441bb4_0  
# expat                     2.2.6                h0a44026_0  
# freetype                  2.9.1                hb4e5f40_0  
# ftgl                      2.1.3                         1    cryoem
# krb5                      1.16.3            hcfa6398_1001    conda-forge
# libcurl                   7.65.3               h051b688_0  
# libcxx                    8.0.0                         4    conda-forge
# libcxxabi                 8.0.0                         4    conda-forge
# libedit                   3.1.20181209         hb402a30_0  
# libssh2                   1.8.2                hcdc9a53_2    conda-forge
# libuv                     1.30.1               h01d97ff_0    conda-forge
# ncurses                   6.1               h0a44026_1002    conda-forge
# nodejs                    10.13.0              h0a44026_0  
# openssl                   1.1.1d               h1de35cc_3  
# rhash                     1.3.8                ha12b0ac_0  
# tk                        8.6.9             h2573ce8_1002    conda-forge
# xz                        5.2.4             h1de35cc_1001    conda-forge


conda info
