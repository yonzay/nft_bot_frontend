import { app, BrowserWindow } from 'electron';
import { Client } from './client';
import { Chronicle } from './chronicle';
import path from 'path';

class Main {
	public static window: BrowserWindow;
	public static show_window = (): void => { this.window.show() };
	public static create = (): void => {
		app.whenReady().then(() => {
			this.window = new BrowserWindow({
				show: false,
				width: 1280,
				height: 722,
				frame: false,
				resizable: false,
				maximizable: false,
				fullscreenable: false,
				icon: 'assets/icon.ico',
				title: 'MoonDweller',
				webPreferences: {
					devTools: false,
					preload: path.join(__dirname, 'context.js')
				}
			});
			this.window.removeMenu();
			this.window.loadURL(Client.is_production ? `file://${ app.getAppPath() }/.webpack/renderer/main/index.html` : `http://localhost:3000/main/index.html`);
			this.window.on('close', () => { app.quit() });
			this.window.webContents.on('before-input-event', (event, input) => {
				if (input.key == 'F12') { Chronicle.window.show() }
			});
		});
	};
}

export { Main }