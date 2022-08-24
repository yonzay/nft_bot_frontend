import { app, BrowserWindow } from 'electron';
import { Client } from './client';
import path from 'path';

interface EditView {
    wallet_group_name: string,
    view: BrowserWindow
}

class Edit_Wallet_Group {
	public static total_views: EditView[] = [];
	public static create = (wallet_group_name: string, edit_data: string): void => {
        for (let x = 0; x < this.total_views.length; x++) {
            if (wallet_group_name == this.total_views[x].wallet_group_name) {
                try {
                    this.total_views[x].view.show();
                } catch (e) {
                    this.total_views.splice(x, 1);
                }
                return;
            }
        }
        this.total_views.push({
            wallet_group_name: wallet_group_name,
            view: new BrowserWindow({
                show: false,
                width: 600,
                height: 600,
                minWidth: 600,
                minHeight: 600,
                frame: false,
                fullscreenable: false,
                icon: 'assets/icon.ico',
                title: `Edit Wallet Group - ${ wallet_group_name }`,
                webPreferences: {
                    devTools: false,
                    preload: path.join(__dirname, 'context.js')
                }
            })
        });
        this.total_views[this.total_views.length - 1].view.removeMenu();
        this.total_views[this.total_views.length - 1].view.loadURL(Client.is_production ? `file://${ app.getAppPath() }/.webpack/renderer/edit_wallet_group/index.html` : `http://localhost:3000/edit_wallet_group/index.html`);
        this.total_views[this.total_views.length - 1].view.on('ready-to-show', () => {
            this.total_views[this.total_views.length - 1].view.webContents.executeJavaScript(`window.Edit_Wallet_Group_Index.create('${ wallet_group_name }', '${ edit_data }')`);
            this.total_views[this.total_views.length - 1].view.show();
        });
	};
}

export { Edit_Wallet_Group }