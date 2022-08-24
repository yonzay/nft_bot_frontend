import { app, BrowserWindow } from 'electron';
import { Client } from './client';
import path from 'path';

class Create_Contract_Task {
	public static is_created = false;
	public static window: BrowserWindow;
	public static create = (): void => {
		if (this.is_created) {
			this.window.show();
		} else {
			this.window = new BrowserWindow({
				show: false,
				width: 600,
				height: 600,
				minWidth: 600,
				minHeight: 600,
				frame: false,
				fullscreenable: false,
				icon: 'assets/icon.ico',
				title: 'Create Contract Task',
				webPreferences: {
					devTools: false,
					preload: path.join(__dirname, 'context.js')
				}
			});
			this.is_created = true;
			this.window.removeMenu();
			this.window.loadURL(Client.is_production ? `file://${ app.getAppPath() }/.webpack/renderer/create_contract_task/index.html` : `http://localhost:3000/create_contract_task/index.html`);
			this.window.on('ready-to-show', () => { this.window.show() });
			this.window.on('close', () => { this.is_created = false });
		}
	};
}

export { Create_Contract_Task }