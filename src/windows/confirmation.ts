import { app, BrowserWindow } from 'electron';
import { Client } from './client';
import path from 'path';

interface ConfirmationView {
    identifier: number | string,
    message: string,
    view: BrowserWindow,
    confirm_data: [type: string, identifier: number | string]
}

class Confirmation {
	public static total_views: ConfirmationView[] = [];
	public static create = (identifier: number | string, message: string, confirm_data: [type: string, identifier: string] ): void => {
        for (let x = 0; x < this.total_views.length; x++) {
            if (identifier == this.total_views[x].identifier) {
                try {
                    this.total_views[x].view.show();
                } catch (e) {
                    this.total_views.splice(x, 1);
                }
                return;
            }
        }
        this.total_views.push({
            identifier: identifier,
            message: message,
            view: new BrowserWindow({
                show: false,
                width: 350,
                height: 128,
                frame: false,
                resizable: false,
                maximizable: false,
                fullscreenable: false,
                icon: 'assets/icon.ico',
                title: 'Confirmation',
                webPreferences: {
                    devTools: false,
                    preload: path.join(__dirname, 'context.js')
                }
            }),
            confirm_data: confirm_data
        });
        this.total_views[this.total_views.length - 1].view.removeMenu();
        this.total_views[this.total_views.length - 1].view.loadURL(Client.is_production ? `file://${ app.getAppPath() }/.webpack/renderer/confirmation/index.html` : `http://localhost:3000/confirmation/index.html`);
        this.total_views[this.total_views.length - 1].view.on('ready-to-show', () => {
            this.total_views[this.total_views.length - 1].view.webContents.executeJavaScript(`window.Confirmation_Index.create(
                '${ this.total_views[this.total_views.length - 1].identifier }',
                '${ this.total_views[this.total_views.length - 1].message }',
                ['${ this.total_views[this.total_views.length - 1].confirm_data['0'] }', '${ this.total_views[this.total_views.length - 1].confirm_data['1'] }']
            )`);
            this.total_views[this.total_views.length - 1].view.show();
        });
	};
}

export { Confirmation }