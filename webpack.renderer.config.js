const rules = require('./webpack.rules');
const plugins = require('./webpack.plugins');

rules.push({
	test: /\.css$/,
	use: [{ loader: 'style-loader' }, { loader: 'css-loader' }]
});

rules.push({
	test: /\.(png|jpe?g|gif|jp2|webp)$/,
	loader: 'file-loader',
	options: {
		name: '[name].[ext]',
	}
});

module.exports = {
	module: {
		rules,
	},
	plugins: plugins,
	resolve: {
		extensions: ['.js', '.ts', '.jsx', '.tsx', '.css']
	}
};