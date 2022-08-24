const path = require('path');
const glob = require('glob');
const CopyPlugin = require('copy-webpack-plugin');

module.exports = {
	/**
	 * This is the main entry point for your application, it's the first file
	 * that runs in the main process.
	 */
	entry: toObject(glob.sync('./src/windows/*.ts')),
	output: {
		filename: '[name]'
	},
	plugins: [
        new CopyPlugin({
            patterns: [
                {
					from: './build/Release/*.dll',
					to: '[name][ext]'
				},
				{
					from: './app-update.yml',
					to: '[name][ext]'
				}
            ],
        })
    ],
	// Put your normal webpack config below here
	module: {
		rules: require('./webpack.rules'),
	},
	resolve: {
		extensions: ['.js', '.ts', '.jsx', '.tsx', '.css', '.json', '.node']
	}
};

function toObject(paths) {
	const entry = {};
	paths.forEach(function (p) {
		const name = path.basename(p, '.js');
		entry[name.split('.')[0] + '.js'] = p;
	});
	return entry;
}