const webpack = require('webpack')
const path = require('path');

let config = {
  target: 'web',
  mode: 'development',
  entry: ['./src/main.js'],
  output: {
    path: path.resolve(__dirname, "build"),
    filename: "client.bundle.js"
  },
  module: {
    rules: [
      {
        test: /\.jsx?$/,
        exclude: /node_modules/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: ['babel-preset-env']
          }
        }
      }
    ]
  }
};

module.exports = config;