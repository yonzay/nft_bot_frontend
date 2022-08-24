/* eslint-disable @typescript-eslint/explicit-module-boundary-types */
/* eslint-disable @typescript-eslint/no-explicit-any */
import { app, BrowserWindow, ipcMain } from 'electron';
import { Client } from './client';
import { Authentication } from './authentication';
import { Key_Derivation } from './key_derivation';
import { Decrypt } from './decrypt';
import { Main } from './main';
import { Settings } from './settings';
import { Create_Contract_Task } from './create_contract_task';
import { Create_Sniper_Task } from './create_sniper_task';
import { View_Sniper_Task } from './view_sniper_task';
import { Generate_Wallet } from './generate_wallet';
import { Create_Wallet_Group } from './create_wallet_group';
import { peform_argument } from './context';
import { View_Contract_Task } from './view_contract_task';
import { Edit_Wallet_Group } from './edit_wallet_group';
import { Confirmation } from './confirmation';
import module from '../../build/Release/Module.node';
import path from 'path';

if (require('electron-squirrel-startup')) app.quit();

type chronicle_library = 'chronicle_insert_log' | 'title_bar_update_content' | 'main_update_instance_summary' | 'main_update_subscription_info' | 'main_update_dashboard_card'
| 'main_update_analytics' | 'main_update_total_value_spent' | 'main_update_todays_transactions' | 'main_update_contract_tasks' | 'main_update_sniper_tasks'
| 'main_update_wallet_groups' | 'main_update_transaction_history' | 'main_insert_transaction_history';

interface TransactionEntry {
    blockchain: string;
    transaction_hash: string;
    date_initiated: number;
}

interface ContractTaskEntry {
    task_id: number;
    blockchain: string;
    contract_address: string;
    wallet_group_name: string;
    time_started: number;
}

interface SniperTaskEntry {
    task_id: number;
    blockchain: string;
    marketplace: string;
    collection_slug: string;
	minimum_value: number;
    maximum_value: number;
}

interface WalletGroupEntry {
    name: string;
    blockchain: string;
    private_keys: number;
    value: number;
    acronym: string;
    market_value: number;
}

class Chronicle {
	public static window: BrowserWindow | null = null;
	public static create = (): void => {
		app.whenReady().then(() => {
			this.window = new BrowserWindow({
				show: false,
				width: 980,
				height: 512,
				minWidth: 980,
				minHeight: 512,
				frame: false,
				fullscreenable: false,
				icon: 'assets/icon.ico',
				title: 'Chronicle',
				webPreferences: {
					devTools: false,
					preload: path.join(__dirname, 'context.js')
				}
			});
			this.window.removeMenu();
			this.window.loadURL(Client.is_production ? `file://${ app.getAppPath() }/.webpack/renderer/chronicle/index.html` : `http://localhost:3000/chronicle/index.html`);
			this.window.on('ready-to-show', () => { this.window.show() });
			this.window.on('close', () => { app.quit() });
			module.main_function();
		});
	};
	public static action = (book: chronicle_library, data: any): void => {
		switch (book) {
			case 'chronicle_insert_log':
				Chronicle.window.webContents.executeJavaScript(`window.Chronicle_Index.insert_log('${ data.user }', '${ data.type }', '${ data.input.replace('\'', '\\\'') }');`);
				break;
			case 'title_bar_update_content':
				Main.window.webContents.executeJavaScript(`window.TitleBar.update_content({
					user_info: '${ data.user_info }',
					user_info_email: '${ data.user_info_email }',
					status_indicator_color: '${ data.status_indicator_color }',
					status_info: '${ data.status_info }',
					version: '${ data.version }'
				});`);
				break;
			case 'main_update_instance_summary':
				Main.window.webContents.executeJavaScript(`window.SideBar.update_instance_summary({
					ethereum_spent: ${ data.ethereum_spent },
					solana_spent: ${ data.solana_spent },
					current_gas: ${ data.current_gas },
					txns_attempted: ${ data.txns_attempted },
					total_contract_tasks: ${ data.total_contract_tasks },
					contract_tasks_active: ${ data.contract_tasks_active },
					total_sniper_tasks: ${ data.total_sniper_tasks },
					sniper_tasks_active: ${ data.sniper_tasks_active }
				});`);
				break;
			case 'main_update_subscription_info':
				Main.window.webContents.executeJavaScript(`window.SideBar.update_subscription_info({
					renewal_date: ${ data.renewal_date },
					last_login: ${ data.last_login },
					wallet_address: '${ data.wallet_address }'
				});`);
				break;
			case 'main_update_dashboard_card':
				Main.window.webContents.executeJavaScript(`window.Dashboard.update_dashboard_card({
					nft_image_url: '${ data.nft_image_url }',
					nft_iteration: ${ data.nft_iteration },
					welcome_message: '${ data.welcome_message }',
					username: '${ data.username }',
					quote: '${ data.quote }'
				});`);
				break;
			case 'main_update_analytics':
				Main.window.webContents.executeJavaScript(`window.Dashboard.update_analytics({
					total_transactions: ${ data.total_transactions },
					total_spent: ${ data.total_spent },
					spent_today: ${ data.spent_today },
				});`);
				break;
			case 'main_update_total_value_spent':
				Main.window.webContents.executeJavaScript(`window.Dashboard.update_total_value_spent(${ data });`);
				break;
			case 'main_update_todays_transactions':
				{
					const wrap_data = (data: TransactionEntry[]): string => {
						let data_string = '[';
						data.forEach((transaction) => {
							data_string += `{
								blockchain: '${ transaction.blockchain }',
								transaction_hash: '${ transaction.transaction_hash }',
								date_initiated: ${ transaction.date_initiated }
							},`;
						});
						return data_string += ']';
					};
					Main.window.webContents.executeJavaScript(`window.Dashboard.tsx_wrap_todays_transactions(${ wrap_data(data) });`);
				}
				break;
			case 'main_update_contract_tasks':
				{
					const wrap_data = (data: ContractTaskEntry[]): string => {
						let data_string = '[';
						data.forEach((contract_task) => {
							data_string += `{
								task_id: ${ contract_task.task_id },
								blockchain: '${ contract_task.blockchain }',
								contract_address: '${ contract_task.contract_address }',
								wallet_group_name: '${ contract_task.wallet_group_name }',
								time_started: ${ contract_task.time_started }
							},`;
						});
						return data_string += ']';
					};
					Main.window.webContents.executeJavaScript(`window.ContractTasks.tsx_wrap_contract_tasks(${ wrap_data(data) });`);
				}
				break;
			case 'main_update_sniper_tasks':
				{
					const wrap_data = (data: SniperTaskEntry[]): string => {
						let data_string = '[';
						data.forEach((sniper_task) => {
							data_string += `{
								task_id: ${ sniper_task.task_id },
								blockchain: '${ sniper_task.blockchain }',
								marketplace: '${ sniper_task.marketplace }',
								collection_slug: '${ sniper_task.collection_slug }',
								maximum_value: ${ sniper_task.maximum_value },
								minimum_value: ${ sniper_task.minimum_value }
							},`;
						});
						return data_string += ']';
					};
					Main.window.webContents.executeJavaScript(`window.SniperTasks.tsx_wrap_sniper_tasks(${ wrap_data(data) });`);
				}
				break;
			case 'main_update_wallet_groups':
				{
					const wrap_data = (data: WalletGroupEntry[]): string => {
						let data_string = '[';
						data.forEach((wallet_group) => {
							data_string += `{
								name: '${ wallet_group.name }',
								blockchain: '${ wallet_group.blockchain }',
								private_keys: ${ wallet_group.private_keys },
								value: ${ wallet_group.value },
								acronym: '${ wallet_group.acronym }',
								market_value: ${ wallet_group.market_value }
							},`;
						});
						return data_string += ']';
					};
					Main.window.webContents.executeJavaScript(`window.WalletGroups.tsx_wrap_wallet_groups(${ wrap_data(data) });`);
				}
				break;
			case 'main_update_transaction_history':
				{
					const wrap_data = (data: TransactionEntry[]): string => {
						let data_string = '[';
						data.forEach((transaction) => {
							data_string += `{
								blockchain: '${ transaction.blockchain }',
								transaction_hash: '${ transaction.transaction_hash }',
								date_initiated: ${ transaction.date_initiated }
							},`;
						});
						return data_string += ']';
					};
					Main.window.webContents.executeJavaScript(`window.TransactionHistory.tsx_wrap_transaction_history(${ wrap_data(data) });`);
				}
				break;
			case 'main_insert_transaction_history':
				Main.window.webContents.executeJavaScript(`window.TransactionHistory.tsx_insert_transaction_history({
					blockchain: '${ data.blockchain }',
					transaction_hash: '${ data.transaction_hash }',
					date_initiated: ${ data.date_initiated }
				});`);
				break;
		}
	};
}

module.bridge_functions(Chronicle.create, Main.create, Authentication.create, Key_Derivation.create, Decrypt.create,
Authentication.set_ready, Key_Derivation.set_ready, Decrypt.set_ready, Chronicle.action, app.quit, Authentication.close_window,
Key_Derivation.close_window, Decrypt.close_window, Main.show_window, Client.create_public_variable_poll_head, Client.create_pending_transaction_poll_head,
Client.poll_ethereum_transaction, Client.candy_machine_poll);

export { Chronicle }

ipcMain.on('perform', (events, args: peform_argument) => {
	switch (args[0]) {
		case 'open':
			switch (args[1]) {
				case 'settings':
					Settings.create(module.view_settings());
					break;
				case 'create_contract_task':
					Create_Contract_Task.create();
					break;
				case 'view_contract_task':
					View_Contract_Task.create(args[2].task_id, module.view_contract_task(args[2].task_id));
					break;
				case 'create_sniper_task':
					Create_Sniper_Task.create();
					break;
				case 'view_sniper_task':
					View_Sniper_Task.create(args[2].task_id, module.view_sniper_task(args[2].task_id));
					break;
				case 'generate_wallet':
					Generate_Wallet.create();
					break;
				case 'create_wallet_group':
					Create_Wallet_Group.create();
					break;
				case 'edit_wallet_group':
					Edit_Wallet_Group.create(args[2].wallet_group_name, module.view_wallet_group(args[2].wallet_group_name));
					break;
				case 'confirmation':
					switch (args[2].confirm_type) {
						case 'contract_task':
							Confirmation.create(args[2].task_id, `Permanently delete contract task [${ args[2].task_id }]?`, ['contract_task', args[2].task_id]);
							break;
						case 'sniper_task':
							Confirmation.create(args[2].task_id, `Permanently delete sniper task [${ args[2].task_id }]?`, ['sniper_task', args[2].task_id]);
							break;
						case 'wallet_group':
							Confirmation.create(args[2].wallet_group_name, `Permanently delete wallet group \\'${ args[2].wallet_group_name }\\' with ${ args[2].wallet_group_private_keys } private key(s) containing ${ Number((args[2].wallet_group_value as number).toFixed(3)) } ${ args[2].value_acronym } currently worth $${ args[2].market_value } USD?`, ['wallet_group', args[2].wallet_group_name]);
							break;
					}
					break;
			}
			break;
		case 'minimize':
			switch (args[1]) {
				case 'chronicle': 
					Chronicle.window.minimize();
					break;
				case 'authentication':
					Authentication.window.minimize();
					break;
				case 'key_derivation': 
					Key_Derivation.window.minimize();
					break;
				case 'decrypt':
					Decrypt.window.minimize();
					break;
				case 'settings':
					Settings.window.minimize();
					break;
				case 'main':
					Main.window.minimize();
					break;
				case 'create_contract_task':
					Create_Contract_Task.window.minimize();
					break;
				case 'create_sniper_task':
					Create_Sniper_Task.window.minimize();
					break;
				case 'generate_wallet':
					Generate_Wallet.window.minimize();
					break;
				case 'create_wallet_group':
					Create_Wallet_Group.window.minimize();
					break;
			}
			if (args[1].includes('view_contract_task')) {
				const contract_task_id: number = parseInt(args[1].substring(19));
				for (let x = 0; x < View_Contract_Task.total_views.length; x++) {
					if (contract_task_id == View_Contract_Task.total_views[x].task_id) {
						View_Contract_Task.total_views[x].view.minimize();
					}
				}
			} else if (args[1].includes('view_sniper_task')) {
				const sniper_task_id: number = parseInt(args[1].substring(17));
				for (let x = 0; x < View_Sniper_Task.total_views.length; x++) {
					if (sniper_task_id == View_Sniper_Task.total_views[x].task_id) {
						View_Sniper_Task.total_views[x].view.minimize();
					}
				}
			} else if (args[1].includes('edit_wallet_group')) {
				const wallet_group_name: string = args[1].substring(18);
				for (let x = 0; x < Edit_Wallet_Group.total_views.length; x++) {
					if (wallet_group_name == Edit_Wallet_Group.total_views[x].wallet_group_name) {
						Edit_Wallet_Group.total_views[x].view.minimize();
					}
				}
			} else if (args[1].includes('confirmation')) {
				const identifier: string = args[1].substring(13);
				for (let x = 0; x < Confirmation.total_views.length; x++) {
					if (identifier == Confirmation.total_views[x].identifier) {
						Confirmation.total_views[x].view.minimize();
					}
				}
			}
			break;
		case 'maximize':
			switch (args[1]) {
				case 'chronicle':
					if (Chronicle.window.isMaximized()) {
						Chronicle.window.unmaximize();
					} else {
						Chronicle.window.maximize();
					}
					break;
				case 'authentication':
					if (Authentication.window.isMaximized()) {
						Authentication.window.unmaximize();
					} else {
						Authentication.window.maximize();
					}
					break;
				case 'key_derivation':
					if (Key_Derivation.window.isMaximized()) {
						Key_Derivation.window.unmaximize();
					} else {
						Key_Derivation.window.maximize();
					}
					break;
				case 'decrypt':
					if (Decrypt.window.isMaximized()) {
						Decrypt.window.unmaximize();
					} else {
						Decrypt.window.maximize();
					}
					break;
				case 'settings':
					if (Settings.window.isMaximized()) {
						Settings.window.unmaximize();
					} else {
						Settings.window.maximize();
					}
					break;
				case 'create_contract_task':
					if (Create_Contract_Task.window.isMaximized()) {
						Create_Contract_Task.window.unmaximize();
					} else {
						Create_Contract_Task.window.maximize();
					}
					break;
				case 'create_sniper_task':
					if (Create_Sniper_Task.window.isMaximized()) {
						Create_Sniper_Task.window.unmaximize();
					} else {
						Create_Sniper_Task.window.maximize();
					}
					break;
				case 'generate_wallet':
					if (Generate_Wallet.window.isMaximized()) {
						Generate_Wallet.window.unmaximize();
					} else {
						Generate_Wallet.window.maximize();
					}
					break;
				case 'create_wallet_group':
					if (Create_Wallet_Group.window.isMaximized()) {
						Create_Wallet_Group.window.unmaximize();
					} else {
						Create_Wallet_Group.window.maximize();
					}
					break;
			}
			if (args[1].includes('view_contract_task')) {
				const contract_task_id: number = parseInt(args[1].substring(19));
				for (let x = 0; x < View_Contract_Task.total_views.length; x++) {
					if (contract_task_id == View_Contract_Task.total_views[x].task_id) {
						if (View_Contract_Task.total_views[x].view.isMaximized()) {
							View_Contract_Task.total_views[x].view.unmaximize();
						} else {
							View_Contract_Task.total_views[x].view.maximize();
						}
					}
				}
			} else if (args[1].includes('view_sniper_task')) {
				const sniper_task_id: number = parseInt(args[1].substring(17));
				for (let x = 0; x < View_Sniper_Task.total_views.length; x++) {
					if (sniper_task_id == View_Sniper_Task.total_views[x].task_id) {
						if (View_Sniper_Task.total_views[x].view.isMaximized()) {
							View_Sniper_Task.total_views[x].view.unmaximize();
						} else {
							View_Sniper_Task.total_views[x].view.maximize();
						}
					}
				}
			} else if (args[1].includes('edit_wallet_group')) {
				const wallet_group_name: string = args[1].substring(18);
				for (let x = 0; x < Edit_Wallet_Group.total_views.length; x++) {
					if (wallet_group_name == Edit_Wallet_Group.total_views[x].wallet_group_name) {
						if (Edit_Wallet_Group.total_views[x].view.isMaximized()) {
							Edit_Wallet_Group.total_views[x].view.unmaximize();
						} else {
							Edit_Wallet_Group.total_views[x].view.maximize();
						}
					}
				}
			}
			break;
		case 'close':
			switch (args[1]) {
				case 'chronicle':
					Chronicle.window.hide();
					break;
				case 'authentication':
					Authentication.window.close();
					break;
				case 'key_derivation':
					Key_Derivation.window.close();
					break;
				case 'decrypt':
					Decrypt.window.close();
					break;
				case 'settings':
					Settings.window.close();
					break;
				case 'main':
					Main.window.close();
					break;
				case 'create_contract_task':
					Create_Contract_Task.window.close();
					break;
				case 'create_sniper_task':
					Create_Sniper_Task.window.close();
					break;
				case 'generate_wallet':
					Generate_Wallet.window.close();
					break;
				case 'create_wallet_group':
					Create_Wallet_Group.window.close();
					break;
			}
			if (args[1].includes('view_contract_task')) {
				const contract_task_id: number = parseInt(args[1].substring(19));
				for (let x = 0; x < View_Contract_Task.total_views.length; x++) {
					if (contract_task_id == View_Contract_Task.total_views[x].task_id) {
						View_Contract_Task.total_views[x].view.close();
						View_Contract_Task.total_views.splice(x, 1);
					}
				}
			} else if (args[1].includes('view_sniper_task')) {
				const sniper_task_id: number = parseInt(args[1].substring(17));
				for (let x = 0; x < View_Sniper_Task.total_views.length; x++) {
					if (sniper_task_id == View_Sniper_Task.total_views[x].task_id) {
						View_Sniper_Task.total_views[x].view.close();
						View_Sniper_Task.total_views.splice(x, 1);
					}
				}
			} else if (args[1].includes('edit_wallet_group')) {
				const wallet_group_name: string = args[1].substring(18);
				for (let x = 0; x < Edit_Wallet_Group.total_views.length; x++) {
					if (wallet_group_name == Edit_Wallet_Group.total_views[x].wallet_group_name) {
						Edit_Wallet_Group.total_views[x].view.close();
						Edit_Wallet_Group.total_views.splice(x, 1);
					}
				}
			} else if (args[1].includes('confirmation')) {
				const identifier: string = args[1].substring(13);
				for (let x = 0; x < Confirmation.total_views.length; x++) {
					if (identifier == Confirmation.total_views[x].identifier) {
						Confirmation.total_views[x].view.close();
						Confirmation.total_views.splice(x, 1);
					}
				}
			}
			break;
		case 'set_value_spent':
			switch (args[1]) {
				case 'main':
					module.total_value_spent(args[2].acronym);
					break;
			}
			break;
		case 'href':
			switch(args[1]) {
				case 'main':
					module.open_href(JSON.stringify(args[2]));
					break;
			}
			break;
		case 'submit_action':
			switch (args[1]) {
				case 'authentication':
					module.login(args[2]);
					break;
				case 'key_derivation':
					module.key_derivation(args[2]);
					break;
				case 'decrypt':
					module.decrypt(args[2]);
					break;
				case 'settings':
					module.set_settings(args[2]);
					break;
				case 'create_contract_task':
					module.create_contract_task(args[2]);
					break;
				case 'create_sniper_task':
					module.create_sniper_task(args[2]);
					break;
				case 'generate_wallet':
					module.generate_wallet(args[2]);
					break;
				case 'create_wallet_group':
					module.create_wallet_group(args[2]);
					break;
				case 'edit_wallet_group':
					module.edit_wallet_group(args[2][0], args[2][1]);
					break;
			}
			break;
		case 'play':
			switch (args[1]) {
				case 'main':
					switch (args[2].task_type) {
						case 'contract_task':
							module.start_contract_task(args[2].task_id);
							break;
						case 'sniper_task':
							module.start_sniper_task(args[2].task_id);
							break;
					}
					break;
			}
			break;
		case 'pause':
			switch (args[1]) {
				case 'main':
					switch (args[2].task_type) {
						case 'contract_task':
							module.stop_contract_task(args[2].task_id);
							break;
						case 'sniper_task':
							module.stop_sniper_task(args[2].task_id);
							break;
					}
					break;
			}
			break;
		case 'refresh_balances':
			switch (args[1]) {
				case 'main':
					module.refresh_balances();
					break;
			}
			break;
		case 'confirm':
			switch (args[1]) {
				case 'confirmation':
					switch (args[2].type) {
						case 'contract_task':
							module.delete_contract_task(args[2].identifier);
							break;
						case 'sniper_task':
							module.delete_sniper_task(args[2].identifier);
							break;
						case 'wallet_group':
							module.delete_wallet_group(args[2].identifier);
							break;
					}
					break;
			}
			break;
	}
});