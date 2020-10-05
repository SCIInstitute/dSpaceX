# dSpaceX - Design Space Exploration Tool

Exploration of multidimensional data using dSpaceX, the Design Space Explorer

<img src="documentation/images/all_samples.png" width="1000px" align="center" hspace="20">

Table of Contents
====================
- [Overview](#overview)  
- [Installation](#installation)  
- [Using dSpaceX](#using-dspacex)  
- [Development](#development)  

## Overview
The Design Space Explorer facilitates decomposition, modeling, and deep exploration of multivariate datasets consisting of design parameters, quantities of interest, and shapes that represent these combinations. 

## Installation
Please see [Installing dSpaceX](documentation/INSTALL.md) to get things running.

## Using dSpaceX
To use dSpaceX for data exploration, the data must first be prepared, then the server started, and finally the client loaded from your web browser. 

1. Data preparation  
The first step in using dSpaceX for exploration is to preprocess datasets for loading.  
Please see [dSpaceX Configuration](./documentation/configuration.md) to learn how to do this.  

2. [Start the server](documentation/server.md#running-the-server)
3. [Open the web client](./client/README.md#running).  

Instructions for [Data Exploration using dSpaceX](documentation/using.md) can be found here.  

_Happy exploring!_

## Development
dSpaceX is under active development. It consists of a standalone server written in C++ and a portable web client based on the JavaScript React Framework.  
**See [BUILD.md](./documentation/BUILD.md) to get started**
