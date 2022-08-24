module.exports = [
	{
		test: /\.node$/,
		use: [{ loader: 'node-loader' }],
	},
	{
		test: /\.tsx?$/,
		exclude: /(node_modules|\.webpack)/,
		use: {
			loader: 'ts-loader',
			options: {
				transpileOnly: true
			}
		}
	}
];