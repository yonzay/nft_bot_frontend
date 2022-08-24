import { app, BrowserWindow } from 'electron';
import { Client } from './client';
import { Chronicle } from './chronicle';
import path from 'path';

class Authentication {
	public static ready = false;
	public static set_ready = (value: boolean): void => { this.ready = value };
	public static close_window = (): void => { this.window.close() };
	public static window: BrowserWindow;
	public static create = (): void => {
		this.window = new BrowserWindow({
			show: false,
			width: 600,
			height: 600,
			minWidth: 600,
			minHeight: 600,
			frame: false,
			fullscreenable: false,
			icon: 'assets/icon.ico',
			title: 'Authentication',
			webPreferences: {
				devTools: false,
				preload: path.join(__dirname, 'context.js')
			}
		});
		this.window.removeMenu();
		this.window.loadURL(Client.is_production ? `file://${ app.getAppPath() }/.webpack/renderer/authentication/index.html` : `http://localhost:3000/authentication/index.html`);
		this.window.on('ready-to-show', () => { this.window.show() });
		this.window.on('close', () => { if (!this.ready) { app.quit() } });
		this.window.webContents.on('before-input-event', (event, input) => {
			if (input.key == 'F12') { Chronicle.window.show() }
		});
	};
}

export { Authentication }