The v2 web client for dSpaceX uses node and the webpack bundler to build the javascript application. Webpack runs on the Node.js runtime. There are two pcakage managers for node, npm and yarn. This project uses npm. Steps for installing and building follow:

# Install node
https://nodejs.org/en/
Recommend version: 8.9.4 LTS

# Install the client depedencies locally for the project
    ./dSpaceX/client $ npm install

This will create a local directory called ./node_modules containing all the source code for dependencies pulled in through the package manager.


# Build the javascript bundle
    ./dSpaceX/client $ npm run build

This will create a file called 'client.bundle.js' in the client/build folder.

# Open dSpaceX.html in a browser.
file:///.../dSpaceX/client/dSpaceX.html

