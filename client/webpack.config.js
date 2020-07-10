const path = require('path');
const fs = require('fs');
const HtmlWebpackPlugin = require('html-webpack-plugin');

// Client directory
const clientDirectory = fs.realpathSync(process.cwd());

// Get absolute path of file within client directory
const resolveAppPath = (relativePath) => path.resolve(clientDirectory, relativePath);

// Host
const host = process.env.HOST || 'localhost';


let config = {
  target: 'web',
  mode: 'development', // dev default for watch and start scripts; build script overrides to 'production'
  devtool: 'source-map',
  entry: ['@babel/polyfill', resolveAppPath('src') + '/main.js'],
  output: {
    path: resolveAppPath('build'),
    filename: 'client.bundle.js',
  },
  devServer: {
    contentBase: resolveAppPath('.'),
    compress: true,
    hot: true,
    host,
    port: 3000, // select a different port by adding `-- --port <num>` (ex: `npm start -- --port 3001`)
    publicPath: '/build/',
  },
  module: {
    rules: [
      {
        test: /\.jsx?$/,
        exclude: /node_modules/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: ['@babel/preset-env'],
          },
        },
      },
      {
        test: /\.(frag|vert)$/,
        use: 'raw-loader',
      },
      {
        test: /\.css$/,
        loader: 'style-loader!css-loader',
      },
    ],
  },
  plugins: [
    new HtmlWebpackPlugin({
      inject: false,
      template: resolveAppPath('dSpaceX.html'),
    }),
  ],
};

module.exports = config;
