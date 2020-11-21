# Installing dSpaceX

**dSpaceX** consists of two components, a standalone server and a portable web client.  
The server must have access to the raw datasets, while the client is lightweight
and only requires a web browser.

- [Install prerequisites](#prerequisites)  
- [User installation](#user-installation)  
- [Configuration](#configuration)  

## Prerequisites

### Install Anaconda (requried)

We use conda to create a _sandbox_ environment, facilitating multiple
applications with different dependencies without global conflicts. It is not a
virtual environment and therefore incurs no performance penalty. Importantly,
_Anaconda does not modify the host system's base environment. (i.e., it is
completely safe)_ Install Anaconda and the **dSpaceX** dependencies using:

```bash
source ./conda_installs.sh
```

Accept the cryptography license terms and default installation path.  

### Install *Google Chrome*

Chrome is the only officially supported browser, though other browsers should
also work.

## User installation

Currently **dSpaceX** is provided in source form only, so users must build both
the server and the client.

Please see [The Build Instructions](BUILD.md) for details.

## Configuration

Finally, follow the [Instructions for Processing and Loading
Datasets](configuration.md) to prepare data for interactive exploration.

_Happy exploring!_

