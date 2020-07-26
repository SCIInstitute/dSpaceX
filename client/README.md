# The dSpaceX Web Client
The web client for dSpaceX relies on HTML5 which is supported by all modern browsers. It uses Node.js and the Webpack bundler to build the javascript application. Webpack runs on the Node.js runtime. This project uses the Node npm package manager (as opposed to yarn). Steps for installing and building follow:

- [Installation](#installation)  
- [Running](#running)  
- [Development](#development)  

## Installation
1. Install the client depedencies locally for the project - be sure you are running in the dspacex conda environment first 
    _NOTE:_ Requires [nodejs](https://nodejs.org/en/) (version 8.9.4 LTS or greater), which is already installed by [conda_installs.sh](../conda_installs.sh) (see main **[README.md](README.md)**).
```bash
<.../dSpaceX> conda activate dSpaceX
<.../dSpaceX>$ cd client
<.../dSpaceX/client>$ npm install
```
This will create a local directory under `<.../dSpaceX/client>` called `node_modules` containing all the source code for dependencies pulled in through the package manager.

## Running
1. Activate the dspacex conda environment, run the build script, and start the server
```bash
<.../dSpaceX/client> conda activate dSpaceX
<.../dSpaceX/client>$ npm run build
<.../dSpaceX/client>$ npm run server
```
This creates a file called 'client.bundle.js' in the `client/build` folder.  

2. Open dSpaceX in your browser by either opening dSpaceX.html or simply using the localhost url
```http
file:///.../dSpaceX/client/dSpaceX.html
```
```http
http://localhost:8080/
```
_\**NOTE:* currently only the *Google Chrome* browser is actively supported, but other browsers will likely still work._

## Development
For dSpaceX development, there is a debuggable server with automatic updates when source files are changed.  
Note: The release bundle is significantly smaller and notably faster than the development server.
1. Activate dspacex conda environment and run the start script
```bash
<.../dSpaceX/client> conda activate dSpaceX
<.../dSpaceX/client>$ npm start
```
By defaut, port 8080 is used, but you can select a different port by adding `-- --port <num>` (ex: `npm start -- --port 3001`)
_(note the extra dashes between `npm start` and `--port`)_

2. Connect to localhost:<port>/dSpaceX.html in browser
```http
http://localhost:8080/dSpaceX.html
```

### Running a "watch" server
This is similar to a development server but it will not auto-refresh when a file changes, only when the browser is refreshed.
1. Activate dspacex conda environment and run watch script in one terminal
```bash
<.../dSpaceX/client> conda activate dSpaceX
<.../dSpaceX/client>$ npm run watch
```
By defaut, development mode is used, but you can build the optimized version by passing `-- --mode production` to the command.
_(note the extra dashes between `npm run watch` and `--mode`)_

2. In a second terminal, activate dspacex conda environment and run a local http server
```bash
<.../dSpaceX/client> conda activate dSpaceX
<.../dSpaceX/client>$ npm run server
```
As before, 8080 is the default port, but a different port can be used by adding `-- --port <num>` (ex: `npm run server -- --port 3000`)
_(note the extra dashes between `npm run server` and `--port`)_

You may need to force the web page to reload using ctrl + shift + r or command + shift + r in order to ignore the browser cache.

