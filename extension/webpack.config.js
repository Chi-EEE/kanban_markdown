//@ts-check

'use strict';

const path = require('path');
const webpack = require('webpack');
const WebpackObfuscator = require('webpack-obfuscator');

/**@type {import('webpack').Configuration}*/
const config = {
    target: 'node', // https://webpack.js.org/configuration/target/#target

    entry: './src/extension.js', // the entry point of this extension, 📖 -> https://webpack.js.org/configuration/entry-context/
    output: {
        // the bundle is stored in the 'dist' folder (check package.json), 📖 -> https://webpack.js.org/configuration/output/
        path: path.resolve(__dirname, 'dist'),
        filename: 'extension.js',
        libraryTarget: 'commonjs2',
        devtoolModuleFilenameTemplate: '../[resource-path]'
    },
    devtool: 'source-map',
    externals: {
        vscode: 'commonjs vscode' // the vscode-module is created on-the-fly and must be excluded. Add other modules that cannot be webpack'ed, 📖 -> https://webpack.js.org/configuration/externals/
    },
    resolve: {
        // support reading TypeScript and JavaScript files, 📖 -> https://github.com/TypeStrong/ts-loader
        extensions: ['.ts', '.js'],
    },
    module: {
        rules: [
            {
                test: /\.ts$/,
                exclude: /node_modules/,
                use: [
                    {
                        loader: 'ts-loader',
                        options: {
                            compilerOptions: {
                                "module": "es6" // override `tsconfig.json` so that TypeScript emits native JavaScript modules.
                            }
                        }
                    }
                ]
            }
        ]
    },
    plugins: [
        new WebpackObfuscator({
            rotateStringArray: true,
        }, ['node_modules/**/*'])
    ]
};
module.exports = config;