source("linear.gaussian.R")
source("linear.gaussianNd.R")

linear.gaussian.squeezek.plots.k("~/research/papers/msregression/figures/msnoise_squeezek_gaussian.ps", n=10000)

linear.gaussian.squeezek.plots("~/research/papers/msregression/figures/msnoise_squeeze50obs_gaussian.ps", n=10000, k=50)

linear.gaussianNd.squeezek.plots.samplesize("~/research/papers/msregression/figures/msnoise_squeezek_gaussianNd_samplesize.ps", n=100000)

linear.gaussianNd.squeezek.plots.knn("~/research/papers/msregression/figures/msnoise_squeezek_gaussianNd_knn.ps", n=100000)

linear.gaussianNd.lorder.plots.samplesize("~/research/papers/msregression/figures/msnoise_lorder_gaussianNd_samplesize.ps", n=100000)

