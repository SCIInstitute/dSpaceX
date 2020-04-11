# dSpaceX Web Client
The web client for dSpaceX uses Node.js and the Webpack bundler to build the javascript application. Webpack runs on the Node.js runtime. There are two package managers for Node: npm and yarn. This project uses npm. Steps for installing and building follow:

1. Install the client depedencies locally for the project - be sure you are running in the dspacex conda environment first 
    _NOTE:_ Requires [nodejs](https://nodejs.org/en/) (version 8.9.4 LTS or greater), which is already installed by [conda_installs.sh](../conda_installs.sh) (see main **[README.md](README.md)**).
```bash
<.../dSpaceX> conda activate dSpaceX
<.../dSpaceX>$ cd client
<.../dSpaceX/client>$ npm install
```
    This will create a local directory under `<.../dSpaceX/client>` called `node_modules` containing all the source code for dependencies pulled in through the package manager.

2. Build the javascript bundle
```bash
<.../dSpaceX/client>$ npm run build
```
    This creates a file called 'client.bundle.js' in the `client/build` folder.
    _*NOTE:* you must re-run `npm run build` each time the client code is modified_

3. Open dSpaceX build in browser in a browser.*
```http
file:///.../dSpaceX/client/build/index.html
```
    _\**NOTE:* currently only the *Chrome* brower is supported. However, it is very likely other browsers will work given our tec stack_

# Running a developement server
1. Activate conda environment and run start script
```bash
<.../dSpaceX/client> conda activate dSpaceX
<.../dSpaceX/client>$ npm start
```

2. Connect to local host in browser
```http
http://localhost:3000/
```

# Building release bundle (significantly smaller) and serving it locally
1. Activate conda environment and run start script
```bash
<.../dSpaceX/client> conda activate dSpaceX
<.../dSpaceX/client>$ npm build
<.../dSpaceX/client>$ http-server . -p 8080
```

2. Connect to local host in browser (_note port 8080_)
```http
http://localhost:8080/
```
