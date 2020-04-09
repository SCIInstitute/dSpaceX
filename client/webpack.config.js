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
  mode: 'development',
  devtool: 'source-map',
  entry: ['@babel/polyfill', resolveAppPath('src') + '/main.js'],
  output: {
    path: resolveAppPath('build'),
    filename: 'client.bundle.js',
  },
  devServer: {
    contentBase: resolveAppPath('public'),
    compress: true,
    hot: true,
    host,
    port: 3000,
    publicPath: '/',
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
      inject: true,
      template: resolveAppPath('public/dSpaceX.html'),
    }),
  ],
};

module.exports = config;
